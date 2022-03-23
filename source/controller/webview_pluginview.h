#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <thread>
#include <mutex>

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
  //
  std::thread webview_thread_;

  // Void ptr to avoid the include hell with webview.h, for now.
  void *webview_handle_ = nullptr;
};

} // namespace ui
} // namespace sidebands