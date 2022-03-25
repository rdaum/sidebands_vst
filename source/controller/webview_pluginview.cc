#include "controller/webview_pluginview.h"

#include <deque>

#include <glog/logging.h>
#include <nlohmann/json.hpp>
#include <public.sdk/source/vst/utility/stringconvert.h>

#include "controller/webview/webview.h"

namespace sidebands {
namespace ui {

WebviewPluginView::WebviewPluginView(Steinberg::Vst::EditController *controller,
                                     Steinberg::ViewRect *size)
    : Steinberg::Vst::EditorView(controller, size),
      thread_checker_(Steinberg::Vst::ThreadChecker::create()) {}

Steinberg::tresult
WebviewPluginView::isPlatformTypeSupported(Steinberg::FIDString type) {
  return Steinberg::kResultTrue;
}

Steinberg::tresult WebviewPluginView::onSize(Steinberg::ViewRect *newSize) {
  {
    std::lock_guard<std::mutex> webview_lock(webview_mutex_);
    if (webview_handle_) {
      webview_handle_->SetViewSize(
          newSize->getWidth(), newSize->getHeight(),
          webview::Webview::SizeHint::kNone);
    }
  }
  return Steinberg::Vst::EditorView::onSize(newSize);
}

void WebviewPluginView::attachedToParent() {
  LOG(INFO) << "Attach on correct thread: "
            << thread_checker_->test("Not attached on UI thread");
  if (!webview_handle_) {
    webview_handle_ = webview::MakeWebview(
        true, systemWindow, [this](webview::Webview *webview) {
          LOG(INFO) << "Loaded webview :" << webview << " doing stuff.";
          webview->SetTitle("Sidebands");
          webview->SetViewSize(rect.getWidth(), rect.getHeight(),
                               webview::Webview::SizeHint::kNone);
          webview->BindFunction(
              "getParameterObject",
              [this](int seq, const std::string &fname,
                     const nlohmann::json &in) -> nlohmann::json {
                LOG(INFO) << "thread? "
                          << thread_checker_->test(
                                 "Not called back on UI thread");
                int id = in[0];
                auto *param = controller->getParameterObject(id);
                if (!param)
                  return "{}";
                auto &info = param->getInfo();
                nlohmann::json out = {
                    {"normalized", param->getNormalized()},
                    {"precision", param->getUnitID()},
                    {"unitID", param->getUnitID()},
                    {"info",
                     {
                         {"id", info.id},
                         {"title", VST3::StringConvert::convert(info.title)},
                         {"stepCount", info.stepCount},
                         {"flags", info.flags},
                         {"defaultNormalizedValue",
                          info.defaultNormalizedValue},
                         {"units", info.units},
                         {"shortTitle",
                          VST3::StringConvert::convert(info.shortTitle)},
                     }},
                };
                return out;
              });
          webview->BindFunction(
              "setParamNormalized",
              [this](int seq, const std::string &fname,
                     const nlohmann::json &in) -> nlohmann::json {
                LOG(INFO) << "thread? "
                          << thread_checker_->test(
                                 "Not called back on UI thread");
                int tag = in[0];
                double value = in[1];
                nlohmann::json out = controller->setParamNormalized(tag, value);
                return out;
              });
          webview->Navigate("https://appassets.daumaudioworks/start.html");
          LOG(INFO) << "Done load sequence";
        });
  }

  LOG(INFO) << "Created webview: " << webview_handle_.get();

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

void WebviewPluginView::update(Steinberg::FUnknown *unknown,
                               Steinberg::int32 message) {
  LOG(INFO) << "Called in UI thread? "
            << thread_checker_->test("not in ui thread");

  if (message != IDependent::kChanged)
    return;
  CBObject *changed_cb;
  if (unknown->queryInterface(CBObject::iid, (void **)&changed_cb) !=
      Steinberg::kResultOk)
    return;

  // Execute the callback on our UI thread.
  std::string result = changed_cb->callback_fn_(changed_cb->Message());

  // Set its reply.
  changed_cb->SetReply(result);
}

Steinberg::tresult WebviewPluginView::canResize() {
  return Steinberg::kResultFalse;
}

void CBObject::update(Steinberg::FUnknown *unknown, Steinberg::int32 message) {}

CBObject::CBObject(std::function<std::string(std::string)> fn)
    : Steinberg::FObject(), callback_fn_(fn) {
  thread_checker_ = Steinberg::Vst::ThreadChecker::create();
  thread_checker_->test("not created ui thread");
  LOG(ERROR) << "UpdateHandler: " << getUpdateHandler();
}

void CBObject::Call(const std::string &message) {
  // Can be called on any thread.

  // Change the outbound message.
  {
    std::lock_guard<std::mutex> ml(message_mutex_);
    message_ = message;
  }
  // Let dependents know we've changed, this is supposed to transfer threads?!
  LOG(ERROR) << "UpdateHandler: " << getUpdateHandler();
  changed();
}

void CBObject::SetReply(std::string &reply) {
  {
    std::lock_guard<std::mutex> reply_lock(reply_mutex_);
    reply_ = reply;
  }
  reply_condition_.notify_all();
}

std::string CBObject::Message() {
  std::lock_guard<std::mutex> ml(message_mutex_);
  return message_;
}

bool CBObject::WaitReply(std::string &reply) {
  std::unique_lock<std::mutex> reply_wait_lock_(reply_mutex_);
  if (reply_condition_.wait_for(reply_wait_lock_,
                                std::chrono::milliseconds(500)) ==
      std::cv_status::timeout) {
    return false;
  }
  reply = reply_;
  return true;
}

} // namespace ui
} // namespace sidebands
