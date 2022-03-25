#pragma once

#include <thread>
#include <mutex>
#include <deque>
#include <functional>

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <public.sdk/source/common/threadchecker.h>

namespace webview {
class Webview;
}  // namespace webview;

namespace sidebands {
namespace ui {

struct CallbackQueue {
  using ExecFunc = std::function<std::string(std::string)>;
  std::mutex q_mutex;
  std::deque<ExecFunc> cb_queue;
  ExecFunc Pop();
  void Schedule(ExecFunc ef);
};

// Passes string values back and forth.
class CBObject : public Steinberg::FObject {
public:
  explicit CBObject( std::function<std::string(std::string)> fn);

  void Call(const std::string &message);
  std::string Message();

  bool WaitReply(std::string &reply);
  void SetReply(std::string &reply);

  // IDependent overrides
  void update(FUnknown *unknown, Steinberg::int32 int_32) override;

  const std::function<std::string(std::string)> callback_fn_;

private:
  std::unique_ptr<Steinberg::Vst::ThreadChecker> thread_checker_;

  std::mutex message_mutex_;
  std::string message_;

  std::mutex reply_mutex_;
  std::condition_variable reply_condition_;
  std::string reply_;
};

class WebviewPluginView : public Steinberg::Vst::EditorView {
public:
  WebviewPluginView(Steinberg::Vst::EditController *controller,
                    Steinberg::ViewRect *size = nullptr);


  // EditorView overrides
  Steinberg::tresult
  isPlatformTypeSupported(Steinberg::FIDString type) override;

  void attachedToParent() override;
  void removedFromParent() override;
  Steinberg::tresult setFrame(Steinberg::IPlugFrame *frame) override;
  Steinberg::tresult onFocus(Steinberg::TBool a_bool) override;

  Steinberg::tresult canResize() override;
  Steinberg::tresult onSize(Steinberg::ViewRect *newSize) override;

  void update(FUnknown *unknown, Steinberg::int32 message) override;

private:
  std::unique_ptr<Steinberg::Vst::ThreadChecker> thread_checker_;

  std::unique_ptr<CBObject> queue_changed_cb_;

  // Work to perform on the webview's thread, if we can.
  CallbackQueue to_webview_q_;

  // Work to perform back on the controller thread.
  CallbackQueue from_webview_q_;

  //
  std::thread webview_thread_;
  std::mutex webview_mutex_;
  std::unique_ptr<webview::Webview> webview_handle_;
};

} // namespace ui
} // namespace sidebands