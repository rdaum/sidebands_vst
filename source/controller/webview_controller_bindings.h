#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <nlohmann/json.hpp>
#include <public.sdk/source/common/threadchecker.h>

#include "controller/webview/webview.h"

using nlohmann::json;

namespace sidebands {

class WebviewControllerBindings {
public:
  explicit WebviewControllerBindings(Steinberg::Vst::EditControllerEx1 *controller);

  void Bind(webview::Webview *webview);

private:
  void DeclareJSBinding(const std::string &name,
                        webview::Webview::FunctionBinding binding);

  using CallbackFn = json (WebviewControllerBindings::*)(webview::Webview *webview, const json &);
  std::function<const json(webview::Webview *webview, int seq, const std::string &, const json &)>
  BindCallback(CallbackFn fn);

  json GetParameterObject(webview::Webview *webview, const json &in);
  json GetParameterObjects(webview::Webview *webview, const json &in);
  json SetParameterNormalized(webview::Webview *webview, const json &in);
  json NormalizedParamToPlain(webview::Webview *webview, const json &in);
  json GetParamNormalized(webview::Webview *webview, const json &in);
  json BeginEdit(webview::Webview *webview, const json &in);
  json PerformEdit(webview::Webview *webview, const json &in);
  json EndEdit(webview::Webview *webview, const json &in);
  json GetParameterCount(webview::Webview *webview, const json &in);
  json GetSelectedUnit(webview::Webview *webview, const json &in);
  json SelectUnit(webview::Webview *webview, const json &in);
  json SubscribeParameter(webview::Webview *webview, const json &in);

  std::unique_ptr<Steinberg::Vst::ThreadChecker> thread_checker_;

  std::vector<std::pair<std::string, webview::Webview::FunctionBinding>>
      bindings_;

  std::unique_ptr<Steinberg::IDependent> param_dep_proxy_;
  Steinberg::Vst::EditControllerEx1 *controller_;
};

}  // namespace sidebands