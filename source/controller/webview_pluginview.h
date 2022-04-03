#pragma once

#include <deque>
#include <functional>
#include <mutex>
#include <thread>

#include <public.sdk/source/vst/vsteditcontroller.h>

#include "controller/webview/webview.h"


namespace sidebands {

class WebviewControllerBindings;

namespace ui {

class WebviewPluginView : public Steinberg::Vst::EditorView {
public:
  WebviewPluginView(Steinberg::Vst::EditController *controller,
                    WebviewControllerBindings *controller_bindings,
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
  std::mutex webview_mutex_;
  std::unique_ptr<webview::Webview> webview_handle_;
  WebviewControllerBindings *controller_bindings_;
};

} // namespace ui
} // namespace sidebands