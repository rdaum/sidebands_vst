#pragma once

#include <public.sdk/source/common/threadchecker.h>
#include <public.sdk/source/vst/vsteditcontroller.h>

#include <nlohmann/json.hpp>

#include "controller/webview/webview.h"

using nlohmann::json;

namespace sidebands {

class WebviewMessageListener;
class WebviewControllerBindings {
 public:
  explicit WebviewControllerBindings(
      Steinberg::Vst::EditControllerEx1 *controller);

  void Bind(webview::Webview *webview);

  WebviewMessageListener *message_listener() const {
    return message_listener_.get();
  }

 private:
  void DeclareJSBinding(const std::string &name,
                        webview::Webview::FunctionBinding binding);

  using CallbackFn = json (WebviewControllerBindings::*)(
      webview::Webview *webview, const json &);
  std::function<const json(webview::Webview *webview, int seq,
                           const std::string &, const json &)>
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
  json DoSendMessage(webview::Webview *webview, const json &in);

  std::unique_ptr<Steinberg::Vst::ThreadChecker> thread_checker_;
  std::vector<std::pair<std::string, webview::Webview::FunctionBinding>>
      bindings_;
  std::unique_ptr<Steinberg::IDependent> param_dep_proxy_;
  Steinberg::Vst::EditControllerEx1 *controller_;
  std::unique_ptr<WebviewMessageListener> message_listener_;
};

class WebviewMessageListener {
 public:
  struct MessageAttribute {
    std::string name;
    enum class Type { INT, FLOAT, STRING, BINARY };
    Type type;
  };

  explicit WebviewMessageListener(webview::Webview *webview)
      : webview_(webview) {}

  void Subscribe(const std::string &receiver, const std::string &message_id,
                 const std::vector<MessageAttribute> &attributes);

  Steinberg::tresult Notify(Steinberg::Vst::IMessage *message);

 private:
  struct MessageDescriptor {
    std::string message_id;
    std::vector<MessageAttribute> attributes;
  };

  struct MessageSubscription {
    MessageDescriptor descriptor;
    std::string notify_function;
  };

  json SerializeMessage(Steinberg::Vst::IMessage *message,
                        const MessageDescriptor &descriptor);

  std::unordered_map<std::string, MessageSubscription> subscriptions_;
  webview::Webview *webview_;
};

}  // namespace sidebands