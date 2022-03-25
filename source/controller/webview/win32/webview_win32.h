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


namespace webview {

using MessageReceivedCallback = std::function<void(const std::string)>;

class Win32BrowserEngine : public Webview {
public:
  Win32BrowserEngine(HWND parent_window, bool debug,
                     WebviewCreatedCallback created_cb);
  ~Win32BrowserEngine() override = default;

  void *PlatformWindow() const override { return (void *)window_; }
  void Terminate() override;
  void SetTitle(const std::string &title) override;
  void SetViewSize(int width, int height, SizeHint hints) override;

  virtual bool embed() = 0;
  virtual void resize(){};

protected:
  HWND window_;
  bool debug_;
  WebviewCreatedCallback created_cb_;

private:
  POINT minsz_ = POINT{0, 0};
  POINT maxsz_ = POINT{0, 0};
  DWORD main_thread_;
};

} // namespace webview
