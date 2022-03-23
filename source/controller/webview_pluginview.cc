#include "controller/webview_pluginview.h"

#include <glog/logging.h>

#include "winrt/base.h"
namespace winrt::impl {
template <typename Async>
auto wait_for(Async const &async, Windows::Foundation::TimeSpan const &timeout);
}
#include "webview.h"

namespace sidebands {
namespace ui {

WebviewPluginView::WebviewPluginView(Steinberg::Vst::EditController *controller,
                                     Steinberg::ViewRect *size)
    : Steinberg::Vst::EditorView(controller, size) {}

Steinberg::tresult
WebviewPluginView::isPlatformTypeSupported(Steinberg::FIDString type) {
  return Steinberg::kResultTrue;
}

Steinberg::tresult WebviewPluginView::onSize(Steinberg::ViewRect *newSize) {
  if (webview_handle_) {
    auto *webview = static_cast<webview::webview *>(webview_handle_);
    webview->set_size(newSize->getWidth(), newSize->getHeight(),
                      WEBVIEW_HINT_NONE);
  }
  return Steinberg::Vst::EditorView::onSize(newSize);
}

void WebviewPluginView::attachedToParent() {
  if (!webview_handle_) {
    LOG(INFO) << "Created webview";
    webview_thread_ = std::thread([this]() {
      webview_handle_ = new webview::webview(true, &systemWindow);
      auto *webview = static_cast<webview::webview *>(webview_handle_);
      LOG(INFO) << "Navigating";
      webview->set_title("Sidebands");
      webview->set_size(rect.getWidth(), rect.getHeight(), WEBVIEW_HINT_FIXED);
      webview->set_html("<body>hi there i'm a webdoc</body>");
      webview->run();
    });
    webview_thread_.detach();
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
  webview_handle_ = new webview::webview(true, &systemWindow);
  auto *webview = static_cast<webview::webview *>(webview_handle_);
  webview->terminate();

  EditorView::removedFromParent();
}

Steinberg::tresult WebviewPluginView::canResize() {
  return Steinberg::kResultFalse
}

} // namespace ui
} // namespace sidebands
