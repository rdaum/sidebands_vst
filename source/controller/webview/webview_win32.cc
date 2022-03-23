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
  m_webview = op.GetResults();
  m_webview.Settings().IsScriptNotifyAllowed(true);
  m_webview.IsVisible(true);
  m_webview.ScriptNotify([=](auto const &sender, auto const &args) {
    std::string s = winrt::to_string(args.Value());
    cb(s.c_str());
  });
  m_webview.NavigationStarting([=](auto const &sender, auto const &args) {
    m_webview.AddInitializeScript(winrt::to_hstring(init_js));
  });
  init("window.external.invoke = s => window.external.notify(s)");
  return true;
}

void edge_html::navigate(const std::string url) {
  Uri uri(winrt::to_hstring(url));
  m_webview.Navigate(uri);
}

void edge_html::init(const std::string js) {
  init_js = init_js + "(function(){" + js + "})();";
}

void edge_html::set_html(const std::string html) {
  m_webview.NavigateToString(
      winrt::to_hstring("data:text/html," + url_encode(html)).c_str());
}

void edge_html::eval(const std::string js) {
  m_webview.InvokeScriptAsync(
      L"eval", single_threaded_vector<hstring>({winrt::to_hstring(js)}));
}

void edge_html::resize(HWND wnd) {
  if (m_webview == nullptr) {
    return;
  }
  RECT r;
  GetClientRect(wnd, &r);
  Rect bounds(r.left, r.top, r.right - r.left, r.bottom - r.top);
  m_webview.Bounds(bounds);
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
  env->CreateCoreWebView2Controller(m_window, this);
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

  m_cb(controller);
  return S_OK;
}

HRESULT edge_chromium::webview2_com_handler::Invoke(
    ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args) {
  LPWSTR message;
  args->TryGetWebMessageAsString(&message);
  m_msgCb(winrt::to_string(message));
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
          w->m_browser->resize(hwnd);
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
          if (w->m_maxsz.x > 0 && w->m_maxsz.y > 0) {
            lpmmi->ptMaxSize = w->m_maxsz;
            lpmmi->ptMaxTrackSize = w->m_maxsz;
          }
          if (w->m_minsz.x > 0 && w->m_minsz.y > 0) {
            lpmmi->ptMinTrackSize = w->m_minsz;
          }
        } break;
        default:
          return DefWindowProcW(hwnd, msg, wp, lp);
        }
        return 0;
      });
  RegisterClassExW(&wc);
  m_window = CreateWindowW(L"webview", L"", (WS_BORDER), CW_USEDEFAULT,
                           CW_USEDEFAULT, 640, 480, nullptr, nullptr,
                           GetModuleHandle(nullptr), nullptr);
  SetWindowLongPtr(m_window, GWLP_USERDATA, (LONG_PTR)this);

  if (window) {
    SetParent(m_window, *(HWND *)window);
    SetWindowLong(m_window, GWL_STYLE, 0);
    SetWindowPos(m_window, nullptr, 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_FRAMECHANGED);
    RedrawWindow(m_window, nullptr, nullptr, RDW_INVALIDATE);
  }

  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
  ShowWindow(m_window, SW_SHOW);
  UpdateWindow(m_window);
  SetFocus(m_window);

  auto cb =
      std::bind(&webview::on_message, webview, std::placeholders::_1);

  if (!m_browser->embed(m_window, debug, cb)) {
    m_browser = std::make_unique<edge_html>();
    m_browser->embed(m_window, debug, cb);
  }

  m_browser->resize(m_window);
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
  DestroyWindow(m_window);
  PostQuitMessage(0);
}

void win32_edge_engine::dispatch(dispatch_fn_t f) {
  PostThreadMessage(m_main_thread, WM_APP, 0, (LPARAM) new dispatch_fn_t(f));
}

void win32_edge_engine::set_title(const std::string title) {
  SetWindowTextW(m_window, winrt::to_hstring(title).c_str());
}

void win32_edge_engine::set_size(int width, int height, int hints) {
  auto style = GetWindowLong(m_window, GWL_STYLE);
  if (hints == WEBVIEW_HINT_FIXED) {
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  } else {
    style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
  }
  SetWindowLong(m_window, GWL_STYLE, style);

  if (hints == WEBVIEW_HINT_MAX) {
    m_maxsz.x = width;
    m_maxsz.y = height;
  } else if (hints == WEBVIEW_HINT_MIN) {
    m_minsz.x = width;
    m_minsz.y = height;
  } else {
    RECT r;
    r.left = r.top = 0;
    r.right = width;
    r.bottom = height;
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, 0);
    SetWindowPos(
        m_window, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
    m_browser->resize(m_window);
  }
}

// static
std::unique_ptr<browser_engine> MakeEngine(bool debug, void *window, webview *webview) {
  return std::make_unique<win32_edge_engine>(debug, window, webview);
}

}  // namespace webview