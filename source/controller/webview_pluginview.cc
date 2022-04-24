#include "controller/webview_pluginview.h"

#include "controller/webview_controller_bindings.h"
#include "tags.h"
#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <libgen.h>
#include <dlfcn.h>


namespace sidebands {
namespace ui {

WebviewPluginView::WebviewPluginView(
    Steinberg::Vst::EditController *controller,
    WebviewControllerBindings *controller_bindings, Steinberg::ViewRect *size)
    : Steinberg::Vst::EditorView(controller, size),
      controller_bindings_(controller_bindings) {}

Steinberg::tresult
WebviewPluginView::isPlatformTypeSupported(Steinberg::FIDString type) {
  return Steinberg::kResultTrue;
}

Steinberg::tresult WebviewPluginView::onSize(Steinberg::ViewRect *newSize) {
  {
    std::lock_guard<std::mutex> webview_lock(webview_mutex_);
    if (webview_handle_) {
      webview_handle_->SetViewSize(newSize->getWidth(), newSize->getHeight(),
                                   webview::Webview::SizeHint::kNone);
    }
  }
  return Steinberg::Vst::EditorView::onSize(newSize);
}

void WebviewPluginView::attachedToParent() {
  if (!webview_handle_) {
    auto init_function = [this](webview::Webview *webview) {
      controller_bindings_->Bind(webview);
      webview->SetTitle("Sidebands VST Webview");
      webview->SetViewSize(rect.getWidth(), rect.getHeight(),
                           webview::Webview::SizeHint::kFixed);

      webview->Navigate(webview->ContentRootURI() + "/index.html");
    };

    webview_handle_ = webview::MakeWebview(true,  plugFrame, systemWindow, init_function);
  }

  EditorView::attachedToParent();
}

Steinberg::tresult WebviewPluginView::setFrame(Steinberg::IPlugFrame *frame) {
  return CPluginView::setFrame(frame);
}

Steinberg::tresult WebviewPluginView::onFocus(Steinberg::TBool a_bool) {
  return Steinberg::kResultOk;
}

void WebviewPluginView::removedFromParent() {
  if (webview_handle_) {
    webview_handle_->Terminate();
  }

  EditorView::removedFromParent();
}

Steinberg::tresult WebviewPluginView::canResize() {
  return Steinberg::kResultFalse;
}

} // namespace ui
} // namespace sidebands
