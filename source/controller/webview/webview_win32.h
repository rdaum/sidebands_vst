#pragma once

#include "controller/webview/webview.h"

//
// ====================================================================
//
// This implementation uses Win32 API to create a native window. It can
// use either EdgeHTML or Edge/Chromium backend as a browser engine.
//
// ====================================================================
//

#define WIN32_LEAN_AND_MEAN
#include <codecvt>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "winrt/base.h"
namespace winrt::impl {
template <typename Async>
auto wait_for(Async const &async, Windows::Foundation::TimeSpan const &timeout);
}

// EdgeHTML headers and libs
#include <objbase.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.UI.Interop.h>
#pragma comment(lib, "windowsapp")

// Edge/Chromium headers and libs
#include "WebView2.h"
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace webview {

using msg_cb_t = std::function<void(const std::string)>;

// Common interface for EdgeHTML and Edge/Chromium
class browser {
public:
  virtual ~browser() = default;
  virtual bool embed(HWND, bool, msg_cb_t) = 0;
  virtual void navigate(const std::string url) = 0;
  virtual void set_html(const std::string html) = 0;
  virtual void eval(const std::string js) = 0;
  virtual void init(const std::string js) = 0;
  virtual void resize(HWND) = 0;
};

//
// EdgeHTML browser engine
//
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::UI;
using namespace Windows::Web::UI::Interop;

class edge_html : public browser {
public:
  bool embed(HWND wnd, bool debug, msg_cb_t cb) override;
  void navigate(const std::string url) override;
  void init(const std::string js) override;
  void set_html(const std::string html) override;
  void eval(const std::string js) override;
  void resize(HWND wnd) override;

private:
  WebViewControl webview_ = nullptr;
  std::string init_js_ = "";
};

//
// Edge/Chromium browser engine
//
class edge_chromium : public browser {
public:
  bool embed(HWND wnd, bool debug, msg_cb_t cb) override;
  void resize(HWND wnd) override;
  void navigate(const std::string url) override;
  void set_html(const std::string html) override;
  void init(const std::string js) override;
  void eval(const std::string js) override;

private:
  ICoreWebView2 *m_webview = nullptr;
  ICoreWebView2Controller *m_controller = nullptr;

  class webview2_com_handler
      : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
        public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
        public ICoreWebView2WebMessageReceivedEventHandler,
        public ICoreWebView2PermissionRequestedEventHandler {
    using webview2_com_handler_cb_t =
        std::function<void(ICoreWebView2Controller *)>;

  public:
    webview2_com_handler(HWND hwnd, msg_cb_t msgCb,
                         webview2_com_handler_cb_t cb)
        : window_(hwnd), msg_cb_(msgCb), com_handler_cb_(cb) {}

    ULONG STDMETHODCALLTYPE AddRef() { return 1; }
    ULONG STDMETHODCALLTYPE Release() { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppv);
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT res,
                                     ICoreWebView2Environment *env) override;
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT res,
                                     ICoreWebView2Controller *controller);
    HRESULT STDMETHODCALLTYPE Invoke(
        ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args);
    HRESULT STDMETHODCALLTYPE
    Invoke(ICoreWebView2 *sender,
           ICoreWebView2PermissionRequestedEventArgs *args);
  private:
    HWND window_;
    msg_cb_t msg_cb_;
    webview2_com_handler_cb_t com_handler_cb_;
  };
};

class win32_edge_engine : public browser_engine {
public:
  win32_edge_engine(bool debug, void *window, webview *webview);

  void run() override;
  void *window() const override { return (void *)window_; }
  void terminate() override;
  void dispatch(dispatch_fn_t f) override;
  void set_title(const std::string title) override;
  void set_size(int width, int height, int hints) override;
  void navigate(const std::string url) override{
    browser_impl_->navigate(url); }
  void set_html(const std::string html)  override {
    browser_impl_->set_html(html); }
  void eval(const std::string js) override { browser_impl_->eval(js); }
  void init(const std::string js) override { browser_impl_->init(js); }

private:
  HWND window_;
  POINT minsz_ = POINT{0, 0};
  POINT maxsz_ = POINT{0, 0};
  DWORD main_thread_;
  std::unique_ptr<browser> browser_impl_ =
      std::make_unique<edge_chromium>();
};

} // namespace webview
