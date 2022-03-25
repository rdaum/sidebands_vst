#pragma once

#include "controller/webview/win32/webview_win32.h"

#pragma comment(lib, "windowsapp")

#include "WebView2.h"
#include <wrl.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace webview {

//
// Edge/Chromium browser engine
//
class EdgeChromiumBrowser : public Win32BrowserEngine {
public:
  EdgeChromiumBrowser(HWND parent_window, bool debug,
                      WebviewCreatedCallback created_cb);
  ~EdgeChromiumBrowser() override;

  bool embed() override;
  void resize() override;
  void Navigate(const std::string &url) override;
  void OnDocumentCreate(const std::string &js) override;
  void EvalJS(const std::string &js) override;
  void DispatchIn(DispatchFunction f) override;

private:
  HRESULT OnControllerCreated(HRESULT result, ICoreWebView2Controller *controller);
  HRESULT OnEnvironmentCreated(HRESULT result,
                            ICoreWebView2Environment *environment);
  HRESULT OnWebMessageReceived(
      ICoreWebView2 *sender,
      ICoreWebView2WebMessageReceivedEventArgs *args);
  HRESULT OnPermissionRequested(
      ICoreWebView2 *sender,
      ICoreWebView2PermissionRequestedEventArgs  *args);
  bool SetFilePaths();

  wchar_t virtual_server_path_[MAX_PATH];
  wchar_t user_data_path_[MAX_PATH];

  ICoreWebView2 *webview2_ = nullptr;
  ICoreWebView2Controller *wv2_controller_ = nullptr;

  Microsoft::WRL::ComPtr<
      ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>
      controller_completed_handler_;
  Microsoft::WRL::ComPtr<
      ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>
      environment_completed_handler_;
  Microsoft::WRL::ComPtr<
      ICoreWebView2WebMessageReceivedEventHandler>
      message_received_handler_;
  Microsoft::WRL::ComPtr<
      ICoreWebView2PermissionRequestedEventHandler>
      permission_requested_handler_;
  ICoreWebView2Settings *settings_;
};

}  // namespace webview