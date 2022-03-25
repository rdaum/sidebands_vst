#include "controller/webview/win32/webview_win32.h"

#include <glog/logging.h>

#include "controller/webview/webview.h"
#include "controller/webview/win32/webview_edge_chromium.h"

#include "winrt/base.h"
namespace winrt::impl {
template <typename Async>
auto wait_for(Async const &async, Windows::Foundation::TimeSpan const &timeout);
}
#include <objbase.h>
#include <winrt/Windows.Foundation.h>

namespace webview {

Win32BrowserEngine::Win32BrowserEngine(HWND parent_window, bool debug,
                                       WebviewCreatedCallback created_cb)
    : debug_(debug), created_cb_(created_cb) {
  main_thread_ = GetCurrentThreadId();

  HINSTANCE hInstance = GetModuleHandle(nullptr);
  HICON icon = (HICON)LoadImage(hInstance, IDI_APPLICATION, IMAGE_ICON,
                                GetSystemMetrics(SM_CXSMICON),
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
        auto *w = reinterpret_cast<Win32BrowserEngine *>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA));
        switch (msg) {
        case WM_SIZE:
          w->resize();
          break;
        case WM_CLOSE:
          DestroyWindow(hwnd);
          break;
        case WM_DESTROY:
          w->Terminate();
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
  window_ = CreateWindowW(L"webview", L"", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                          CW_USEDEFAULT, 640, 480, nullptr, nullptr,
                          GetModuleHandle(nullptr), nullptr);
  SetWindowLongPtr(window_, GWLP_USERDATA, (LONG_PTR)this);

  if (parent_window) {
    SetParent(window_, parent_window);
    SetWindowLong(window_, GWL_STYLE, WS_CHILD);
    SetWindowPos(window_, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED);
    RedrawWindow(window_, nullptr, nullptr, RDW_INVALIDATE);
  }

  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
  ShowWindow(window_, SW_SHOW);
  UpdateWindow(window_);
  SetFocus(window_);

  resize();
}

void Win32BrowserEngine::Terminate() { PostQuitMessage(0); }

void Win32BrowserEngine::SetTitle(const std::string &title) {
  SetWindowTextW(window_, winrt::to_hstring(title).c_str());
}

void Win32BrowserEngine::SetViewSize(int width, int height, SizeHint hints) {
  auto style = GetWindowLong(window_, GWL_STYLE);
  if (hints == SizeHint::WEBVIEW_HINT_FIXED) {
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  } else {
    style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
  }
  SetWindowLong(window_, GWL_STYLE, style);

  if (hints == SizeHint::WEBVIEW_HINT_MAX) {
    maxsz_.x = width;
    maxsz_.y = height;
  } else if (hints == SizeHint::WEBVIEW_HINT_MIN) {
    minsz_.x = width;
    minsz_.y = height;
  } else {
    RECT r;
    r.left = r.top = 0;
    r.right = width;
    r.bottom = height;
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, 0);
    SetWindowPos(window_, NULL, r.left, r.top, r.right - r.left,
                 r.bottom - r.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
    resize();
  }
}


// static
std::unique_ptr<Webview> MakeWebview(bool debug, void *window,
                                     WebviewCreatedCallback created_cb) {
  auto webview =
      std::make_unique<EdgeChromiumBrowser>((HWND)window, debug, created_cb);
  if (webview->embed()) {
    return std::move(webview);
  }
  return nullptr;
}

} // namespace webview