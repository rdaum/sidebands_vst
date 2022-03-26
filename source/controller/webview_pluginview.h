#pragma once

#include <deque>
#include <functional>
#include <mutex>
#include <thread>

#include <nlohmann/json.hpp>
#include <public.sdk/source/common/threadchecker.h>
#include <public.sdk/source/vst/vsteditcontroller.h>

namespace webview {
class Webview;
} // namespace webview

using nlohmann::json;

namespace sidebands {
namespace ui {

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

private:
  using CallbackFn = json (WebviewPluginView::*)(const json &);
  std::function<const json(int seq, const std::string &, const json &)>
  BindCallback(CallbackFn fn);

  json WVGetParameterObject(const json &in);
  json WVSetParameterNormalized(const json &in);
  json WVnormalizedParamToPlain(const json &in);
  json WVgetParamNormalized(const json &in);
  json WVbeginEdit(const json &in);
  json WVperformEdit(const json &in);
  json WVendEdit(const json &in);
  json WVgetParameterCount(const json &in);
  json WVsubscribeParameter(const json &in);

  std::unique_ptr<Steinberg::Vst::ThreadChecker> thread_checker_;

  class ParameterDependenciesProxy;
  std::unique_ptr<Steinberg::IDependent> param_dep_proxy_;

  std::mutex webview_mutex_;
  std::unique_ptr<webview::Webview> webview_handle_;
};

} // namespace ui
} // namespace sidebands