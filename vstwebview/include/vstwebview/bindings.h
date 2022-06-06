#pragma once

namespace vstwebview {

class Webview;

class Bindings {
 public:
  virtual void Bind(vstwebview::Webview *webview) = 0;
};
}  // namespace vstwebview
