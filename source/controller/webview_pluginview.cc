#include "controller/webview_pluginview.h"

#include <functional>

#include <glog/logging.h>
#include <public.sdk/source/vst/utility/stringconvert.h>

#include "controller/webview/webview.h"
#include "tags.h"

namespace sidebands {
namespace ui {

namespace {

json SerializeParameter(Steinberg::Vst::Parameter *param) {
  auto &info = param->getInfo();
  return {
      {"normalized", param->getNormalized()},
      {"precision", param->getUnitID()},
      {"unitID", param->getUnitID()},
      {"info",
       {
           {"id", info.id},
           {"title", VST3::StringConvert::convert(info.title)},
           {"stepCount", info.stepCount},
           {"flags", info.flags},
           {"defaultNormalizedValue", info.defaultNormalizedValue},
           {"units", info.units},
           {"shortTitle", VST3::StringConvert::convert(info.shortTitle)},
       }},
  };
}

// Proxy IDependent through the webview for parameter object changes.
class ParameterDependenciesProxy : public Steinberg::FObject {
public:
  explicit ParameterDependenciesProxy(webview::Webview *webview)
      : webview_(webview) {}

  void update(FUnknown *changedUnknown, Steinberg::int32 message) override {
    if (!webview_ || message != IDependent::kChanged)
      return;

    Steinberg::Vst::Parameter *changed_param;
    auto query_result = changedUnknown->queryInterface(
        Steinberg::Vst::RangeParameter::iid, (void **)&changed_param);

    if (query_result != Steinberg::kResultOk)
      return;

    webview_->EvalJS("notifyParameterChange(" +
                         SerializeParameter(changed_param).dump() + ");",
                     [](const json &r) {});
  }

private:
  webview::Webview *webview_;
};

} // namespace

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
      webview_handle_->SetViewSize(newSize->getWidth(), newSize->getHeight(),
                                   webview::Webview::SizeHint::kNone);
    }
  }
  return Steinberg::Vst::EditorView::onSize(newSize);
}

webview::Webview::FunctionBinding
WebviewPluginView::BindCallback(CallbackFn fn) {
  return std::bind(fn, this, std::placeholders::_3);
}

void WebviewPluginView::attachedToParent() {
  auto test_t = TagFor(1, TAG_OSC, TARGET_C);
  LOG(INFO) << "Attach on correct thread: "
            << thread_checker_->test("Not attached on UI thread");
  if (!webview_handle_) {
    auto init_function = [this](webview::Webview *webview) {
      webview->SetTitle("Sidebands VST");
      webview->SetViewSize(rect.getWidth(), rect.getHeight(),
                           webview::Webview::SizeHint::kFixed);

      webview->BindFunction(
          "getParameterObject",
          BindCallback(&WebviewPluginView::WVGetParameterObject));
      webview->BindFunction(
          "getParameterObjects",
          BindCallback(&WebviewPluginView::WVGetParameterObjects));
      webview->BindFunction(
          "subscribeParameter",
          BindCallback(&WebviewPluginView::WVsubscribeParameter));
      webview->BindFunction(
          "setParamNormalized",
          BindCallback(&WebviewPluginView::WVSetParameterNormalized));
      webview->BindFunction(
          "normalizedParamToPlain",
          BindCallback(&WebviewPluginView::WVnormalizedParamToPlain));
      webview->BindFunction(
          "getParamNormalized",
          BindCallback(&WebviewPluginView::WVgetParamNormalized));
      webview->BindFunction("beginEdit",
                            BindCallback(&WebviewPluginView::WVbeginEdit));
      webview->BindFunction("performEdit",
                            BindCallback(&WebviewPluginView::WVperformEdit));
      webview->BindFunction("endEdit",
                            BindCallback(&WebviewPluginView::WVendEdit));
      webview->BindFunction(
          "getParameterCount",
          BindCallback(&WebviewPluginView::WVgetParameterCount));

      webview->Navigate("https://appassets.daumaudioworks/index.html");
      LOG(INFO) << "Done load sequence";
    };

    webview_handle_ = webview::MakeWebview(true, systemWindow, init_function);
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

json WebviewPluginView::WVGetParameterObject(const json &in) {
  thread_checker_->test();
  int id = in[0];
  auto *param = controller->getParameterObject(id);
  LOG(INFO) << "Retrieve for " << id << " (" << TagStr(id) << ")  == " << param;
  if (!param)
    return json();
  return SerializeParameter(param);
}


json WebviewPluginView::WVGetParameterObjects(const json &in) {
  thread_checker_->test();
  json out;
  for (int id : in[0]) {
    auto *param = controller->getParameterObject(id);
    if (param) {
      out[id] = SerializeParameter(param);
    }
  }
  return out;
}

json WebviewPluginView::WVSetParameterNormalized(const json &in) {
  thread_checker_->test();
  int tag = in[0];
  double value = in[1];
  LOG(INFO) << "Setting " << TagStr(tag) << " to n: " << value;
  json out = controller->setParamNormalized(tag, value) == Steinberg::kResultOk;
  return out;
}

json WebviewPluginView::WVnormalizedParamToPlain(const json &in) {
  thread_checker_->test();
  int tag = in[0];
  double value = in[1];
  json out = controller->normalizedParamToPlain(tag, value);
  return out;
}

json WebviewPluginView::WVgetParamNormalized(const json &in) {
  thread_checker_->test();
  int tag = in[0];
  json out = controller->getParamNormalized(tag);
  return out;
}

json WebviewPluginView::WVbeginEdit(const json &in) {
  thread_checker_->test();
  int tag = in[0];
  json out = controller->beginEdit(tag) == Steinberg::kResultOk;
  return out;
}

json WebviewPluginView::WVperformEdit(const json &in) {
  thread_checker_->test();
  int tag = in[0];
  double value = in[1];
  json out = controller->performEdit(tag, value) == Steinberg::kResultOk;
  return out;
}

json WebviewPluginView::WVendEdit(const json &in) {
  thread_checker_->test();
  int tag = in[0];
  json out = controller->endEdit(tag) == Steinberg::kResultOk;
  return out;
}

json WebviewPluginView::WVgetParameterCount(const json &in) {
  thread_checker_->test();
  json out = controller->getParameterCount();
  return out;
}

json WebviewPluginView::WVsubscribeParameter(const json &in) {
  thread_checker_->test();
  json out = controller->getParameterCount();
  int tag = in[0];
  auto *param = controller->getParameterObject(tag);
  if (!param)
    return json();
  if (!param_dep_proxy_) {
    param_dep_proxy_ =
        std::make_unique<sidebands::ui::ParameterDependenciesProxy>(
            webview_handle_.get());
  }
  param->addDependent(param_dep_proxy_.get());
  return true;
}
} // namespace ui
} // namespace sidebands
