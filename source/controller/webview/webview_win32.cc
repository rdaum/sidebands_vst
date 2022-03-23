#include "controller/webview/webview_win32.h"

#include "controller/webview/webview.h"

namespace webview {

bool edge_html::embed(HWND wnd, bool debug, msg_cb_t cb) {
  init_apartment(winrt::apartment_type::single_threaded);
  auto process = WebViewControlProcess();
  auto op =
      process.CreateWebViewControlAsync(reinterpret_cast<int64_t>(wnd), Rect());
  if (op.Status() != AsyncStatus::Completed) {
    handle h(CreateEvent(nullptr, false, false, nullptr));
    op.Completed([h = h.get()](auto, auto) { SetEvent(h); });
    HANDLE hs[] = {h.get()};
    DWORD i;
    CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES |
                                 COWAIT_DISPATCH_CALLS | COWAIT_INPUTAVAILABLE,
                             INFINITE, 1, hs, &i);
  }
  webview_ = op.GetResults();
  webview_.Settings().IsScriptNotifyAllowed(true);
  webview_.IsVisible(true);
  webview_.ScriptNotify([=](auto const &sender, auto const &args) {
    std::string s = winrt::to_string(args.Value());
    cb(s.c_str());
  });
  webview_.NavigationStarting([=](auto const &sender, auto const &args) {
    webview_.AddInitializeScript(winrt::to_hstring(init_js_));
  });
  init("window.external.invoke = s => window.external.notify(s)");
  return true;
}

void edge_html::navigate(const std::string url) {
  Uri uri(winrt::to_hstring(url));
  webview_.Navigate(uri);
}

void edge_html::init(const std::string js) {
  init_js_ = init_js_ + "(function(){" + js + "})();";
}

void edge_html::set_html(const std::string html) {
  webview_.NavigateToString(
      winrt::to_hstring("data:text/html," + url_encode(html)).c_str());
}

void edge_html::eval(const std::string js) {
  webview_.InvokeScriptAsync(
      L"eval", single_threaded_vector<hstring>({winrt::to_hstring(js)}));
}

void edge_html::resize(HWND wnd) {
  if (webview_ == nullptr) {
    return;
  }
  RECT r;
  GetClientRect(wnd, &r);
  Rect bounds(r.left, r.top, r.right - r.left, r.bottom - r.top);
  webview_.Bounds(bounds);
}

bool edge_chromium::embed(HWND wnd, bool debug, msg_cb_t cb) {
  std::atomic_flag flag = ATOMIC_FLAG_INIT;
  flag.test_and_set();

  wchar_t currentExePath[MAX_PATH];
  GetModuleFileNameW(NULL, currentExePath, MAX_PATH);
  wchar_t *currentExeName = PathFindFileNameW(currentExePath);

  wchar_t dataPath[MAX_PATH];
  if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, dataPath))) {
    return false;
  }
  wchar_t userDataFolder[MAX_PATH];
  PathCombineW(userDataFolder, dataPath, currentExeName);

  HRESULT res = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, userDataFolder, nullptr,
      new webview2_com_handler(wnd, cb,
                               [&](ICoreWebView2Controller *controller) {
                                 m_controller = controller;
                                 m_controller->get_CoreWebView2(&m_webview);
                                 m_webview->AddRef();
                                 flag.clear();
                               }));
  if (res != S_OK) {
    return false;
  }
  MSG msg = {};
  while (flag.test_and_set() && GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  init("window.external={invoke:s=>window.chrome.webview.postMessage(s)}");
  return true;
}

void edge_chromium::resize(HWND wnd) {
  if (m_controller == nullptr) {
    return;
  }
  RECT bounds;
  GetClientRect(wnd, &bounds);
  m_controller->put_Bounds(bounds);
}

void edge_chromium::navigate(const std::string url) {
  auto wurl = winrt::to_hstring(url);
  m_webview->Navigate(wurl.c_str());
}

void edge_chromium::set_html(const std::string html) {
  auto html2 = winrt::to_hstring("data:text/html," + url_encode(html));
  m_webview->Navigate(html2.c_str());
}

void edge_chromium::init(const std::string js) {
  auto wjs = winrt::to_hstring(js);
  m_webview->AddScriptToExecuteOnDocumentCreated(wjs.c_str(), nullptr);
}

void edge_chromium::eval(const std::string js) {
  auto wjs = winrt::to_hstring(js);
  m_webview->ExecuteScript(wjs.c_str(), nullptr);
}

HRESULT edge_chromium::webview2_com_handler::QueryInterface(const IID &riid,
                                                            LPVOID *ppv) {
  return S_OK;
}

HRESULT
edge_chromium::webview2_com_handler::Invoke(HRESULT res,
                                            ICoreWebView2Environment *env) {
  env->CreateCoreWebView2Controller(window_, this);
  return S_OK;
}

HRESULT edge_chromium::webview2_com_handler::Invoke(
    HRESULT res, ICoreWebView2Controller *controller) {
  controller->AddRef();

  ICoreWebView2 *webview;
  ::EventRegistrationToken token;
  controller->get_CoreWebView2(&webview);
  webview->add_WebMessageReceived(this, &token);
  webview->add_PermissionRequested(this, &token);

  com_handler_cb_(controller);
  return S_OK;
}

HRESULT edge_chromium::webview2_com_handler::Invoke(
    ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args) {
  LPWSTR message;
  args->TryGetWebMessageAsString(&message);
  msg_cb_(winrt::to_string(message));
  sender->PostWebMessageAsString(message);

  CoTaskMemFree(message);
  return S_OK;
}

HRESULT edge_chromium::webview2_com_handler::Invoke(
    ICoreWebView2 *sender, ICoreWebView2PermissionRequestedEventArgs *args) {
  COREWEBVIEW2_PERMISSION_KIND kind;
  args->get_PermissionKind(&kind);
  if (kind == COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ) {
    args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
  }
  return S_OK;
}

win32_edge_engine::win32_edge_engine(bool debug, void *window, webview *webview) {
  main_thread_ = GetCurrentThreadId();

  HINSTANCE hInstance = GetModuleHandle(nullptr);
  HICON icon = (HICON)LoadImage(
      hInstance, IDI_APPLICATION, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
      GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

  WNDCLASSEXW wc;
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.hInstance = hInstance;
  wc.lpszClassName = L"webview";
  wc.hIcon = icon;
  wc.hIconSm = icon;
  wc.lpfnWndProc =
      (WNDPROC)(+[](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
        auto w = (win32_edge_engine *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        switch (msg) {
        case WM_SIZE:
          w->browser_impl_->resize(hwnd);
          break;
        case WM_CLOSE:
          DestroyWindow(hwnd);
          break;
        case WM_DESTROY:
          w->terminate();
          break;
        case WM_GETMINMAXINFO: {
          auto lpmmi = (LPMINMAXINFO)lp;
          if (w == nullptr) {
            return 0;
          }
          if (w->maxsz_.x > 0 && w->maxsz_.y > 0) {
            lpmmi->ptMaxSize = w->maxsz_;
            lpmmi->ptMaxTrackSize = w->maxsz_;
          }
          if (w->minsz_.x > 0 && w->minsz_.y > 0) {
            lpmmi->ptMinTrackSize = w->minsz_;
          }
        } break;
        default:
          return DefWindowProcW(hwnd, msg, wp, lp);
        }
        return 0;
      });
  RegisterClassExW(&wc);
  window_ = CreateWindowW(L"webview", L"", (WS_BORDER), CW_USEDEFAULT,
                           CW_USEDEFAULT, 640, 480, nullptr, nullptr,
                           GetModuleHandle(nullptr), nullptr);
  SetWindowLongPtr(window_, GWLP_USERDATA, (LONG_PTR)this);

  if (window) {
    SetParent(window_, *(HWND *)window);
    SetWindowLong(window_, GWL_STYLE, 0);
    SetWindowPos(window_, nullptr, 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_FRAMECHANGED);
    RedrawWindow(window_, nullptr, nullptr, RDW_INVALIDATE);
  }

  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
  ShowWindow(window_, SW_SHOW);
  UpdateWindow(window_);
  SetFocus(window_);

  auto cb =
      std::bind(&webview::on_message, webview, std::placeholders::_1);

  if (!browser_impl_->embed(window_, debug, cb)) {
    browser_impl_ = std::make_unique<edge_html>();
    browser_impl_->embed(window_, debug, cb);
  }

  browser_impl_->resize(window_);
}

void win32_edge_engine::run() {
  MSG msg;
  BOOL res;
  while ((res = GetMessage(&msg, nullptr, 0, 0)) != -1) {
    if (msg.hwnd) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      continue;
    }
    if (msg.message == WM_APP) {
      auto f = (dispatch_fn_t *)(msg.lParam);
      (*f)();
      delete f;
    } else if (msg.message == WM_QUIT) {
      return;
    }
  }
}

void win32_edge_engine::terminate() {
  DestroyWindow(window_);
  PostQuitMessage(0);
}

void win32_edge_engine::dispatch(dispatch_fn_t f) {
  PostThreadMessage(main_thread_, WM_APP, 0, (LPARAM) new dispatch_fn_t(f));
}

void win32_edge_engine::set_title(const std::string title) {
  SetWindowTextW(window_, winrt::to_hstring(title).c_str());
}

void win32_edge_engine::set_size(int width, int height, int hints) {
  auto style = GetWindowLong(window_, GWL_STYLE);
  if (hints == WEBVIEW_HINT_FIXED) {
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  } else {
    style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
  }
  SetWindowLong(window_, GWL_STYLE, style);

  if (hints == WEBVIEW_HINT_MAX) {
    maxsz_.x = width;
    maxsz_.y = height;
  } else if (hints == WEBVIEW_HINT_MIN) {
    minsz_.x = width;
    minsz_.y = height;
  } else {
    RECT r;
    r.left = r.top = 0;
    r.right = width;
    r.bottom = height;
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, 0);
    SetWindowPos(window_, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
    browser_impl_->resize(window_);
  }
}

// static
std::unique_ptr<browser_engine> MakeEngine(bool debug, void *window, webview *webview) {
  return std::make_unique<win32_edge_engine>(debug, window, webview);
}

}  // namespace webview