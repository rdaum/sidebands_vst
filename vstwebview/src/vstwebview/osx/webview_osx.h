#pragma once

#include "webview.h"

//
// ====================================================================
//
// This implementation uses Cocoa WKWebView backend on macOS. It is
// written using ObjC runtime and uses WKWebView class as a browser runtime.
// You should pass "-framework Webkit" flag to the compiler.
//
// ====================================================================
//

#include <CoreGraphics/CoreGraphics.h>
#include <objc/objc-runtime.h>

#define NSBackingStoreBuffered 2

#define NSWindowStyleMaskResizable 8
#define NSWindowStyleMaskMiniaturizable 4
#define NSWindowStyleMaskTitled 1
#define NSWindowStyleMaskClosable 2

#define NSApplicationActivationPolicyRegular 0

#define WKUserScriptInjectionTimeAtDocumentStart 0

namespace vstwebview {

// Helpers to avoid too much typing
id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
id operator"" _str(const char *s, std::size_t) {
  return ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel,
                                                      s);
}

class cocoa_wkwebview_engine {
 public:
  cocoa_wkwebview_engine(bool debug, void *window) {
    // Application
    id app = ((id(*)(id, SEL))objc_msgSend)("NSApplication"_cls, "sharedApplication"_sel);
    ((void (*)(id, SEL, long))objc_msgSend)(app, "setActivationPolicy:"_sel,
                                            NSApplicationActivationPolicyRegular);

    // Delegate
    auto cls = objc_allocateClassPair((Class) "NSResponder"_cls, "AppDelegate", 0);
    class_addProtocol(cls, objc_getProtocol("NSTouchBarProvider"));
    class_addMethod(cls, "applicationShouldTerminateAfterLastWindowClosed:"_sel,
                    (IMP)(+[](id, SEL, id) -> BOOL { return 1; }), "c@:@");
    class_addMethod(cls, "userContentController:didReceiveScriptMessage:"_sel,
                    (IMP)(+[](id self, SEL, id, id msg) {
                      auto w = (cocoa_wkwebview_engine *)objc_getAssociatedObject(self, "webview");
                      assert(w);
                      w->on_message(((const char *(*)(id, SEL))objc_msgSend)(
                          ((id(*)(id, SEL))objc_msgSend)(msg, "body"_sel), "UTF8String"_sel));
                    }),
                    "v@:@@");
    objc_registerClassPair(cls);

    auto delegate = ((id(*)(id, SEL))objc_msgSend)((id)cls, "new"_sel);
    objc_setAssociatedObject(delegate, "webview", (id)this, OBJC_ASSOCIATION_ASSIGN);
    ((void (*)(id, SEL, id))objc_msgSend)(app, sel_registerName("setDelegate:"), delegate);

    // Main window
    if (window == nullptr) {
      m_window = ((id(*)(id, SEL))objc_msgSend)("NSWindow"_cls, "alloc"_sel);
      m_window = ((id(*)(id, SEL, CGRect, int, unsigned long, int))objc_msgSend)(
          m_window, "initWithContentRect:styleMask:backing:defer:"_sel, CGRectMake(0, 0, 0, 0), 0,
          NSBackingStoreBuffered, 0);
    } else {
      m_window = (id)window;
    }

    // Webview
    auto config = ((id(*)(id, SEL))objc_msgSend)("WKWebViewConfiguration"_cls, "new"_sel);
    m_manager = ((id(*)(id, SEL))objc_msgSend)(config, "userContentController"_sel);
    m_webview = ((id(*)(id, SEL))objc_msgSend)("WKWebView"_cls, "alloc"_sel);

    if (debug) {
      // Equivalent Obj-C:
      // [[config preferences] setValue:@YES forKey:@"developerExtrasEnabled"];
      ((id(*)(id, SEL, id, id))objc_msgSend)(
          ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel), "setValue:forKey:"_sel,
          ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls, "numberWithBool:"_sel, 1),
          "developerExtrasEnabled"_str);
    }

    // Equivalent Obj-C:
    // [[config preferences] setValue:@YES forKey:@"fullScreenEnabled"];
    ((id(*)(id, SEL, id, id))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel), "setValue:forKey:"_sel,
        ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls, "numberWithBool:"_sel, 1),
        "fullScreenEnabled"_str);

    // Equivalent Obj-C:
    // [[config preferences] setValue:@YES
    // forKey:@"javaScriptCanAccessClipboard"];
    ((id(*)(id, SEL, id, id))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel), "setValue:forKey:"_sel,
        ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls, "numberWithBool:"_sel, 1),
        "javaScriptCanAccessClipboard"_str);

    // Equivalent Obj-C:
    // [[config preferences] setValue:@YES forKey:@"DOMPasteAllowed"];
    ((id(*)(id, SEL, id, id))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel), "setValue:forKey:"_sel,
        ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls, "numberWithBool:"_sel, 1),
        "DOMPasteAllowed"_str);

    ((void (*)(id, SEL, CGRect, id))objc_msgSend)(m_webview, "initWithFrame:configuration:"_sel,
                                                  CGRectMake(0, 0, 0, 0), config);
    ((void (*)(id, SEL, id, id))objc_msgSend)(m_manager, "addScriptMessageHandler:name:"_sel,
                                              delegate, "external"_str);

    init(R"script(
                     window.external = {
                       invoke: function(s) {
                         window.webkit.messageHandlers.external.postMessage(s);
                       },
                     };
                    )script");
    ((void (*)(id, SEL, id))objc_msgSend)(m_window, "setContentView:"_sel, m_webview);
    ((void (*)(id, SEL, id))objc_msgSend)(m_window, "makeKeyAndOrderFront:"_sel, nullptr);
  }
  ~cocoa_wkwebview_engine() { close(); }
  void *window() { return (void *)m_window; }
  void terminate() {
    close();
    ((void (*)(id, SEL, id))objc_msgSend)("NSApp"_cls, "terminate:"_sel, nullptr);
  }
  void run() {
    id app = ((id(*)(id, SEL))objc_msgSend)("NSApplication"_cls, "sharedApplication"_sel);
    dispatch([&]() {
      ((void (*)(id, SEL, BOOL))objc_msgSend)(app, "activateIgnoringOtherApps:"_sel, 1);
    });
    ((void (*)(id, SEL))objc_msgSend)(app, "run"_sel);
  }
  void dispatch(std::function<void()> f) {
    dispatch_async_f(dispatch_get_main_queue(), new dispatch_fn_t(f),
                     (dispatch_function_t)([](void *arg) {
                       auto f = static_cast<dispatch_fn_t *>(arg);
                       (*f)();
                       delete f;
                     }));
  }
  void set_title(const std::string title) {
    ((void (*)(id, SEL, id))objc_msgSend)(
        m_window, "setTitle:"_sel,
        ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel,
                                                     title.c_str()));
  }
  void set_size(int width, int height, int hints) {
    auto style =
        NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    if (hints != WEBVIEW_HINT_FIXED) {
      style = style | NSWindowStyleMaskResizable;
    }
    ((void (*)(id, SEL, unsigned long))objc_msgSend)(m_window, "setStyleMask:"_sel, style);

    if (hints == WEBVIEW_HINT_MIN) {
      ((void (*)(id, SEL, CGSize))objc_msgSend)(m_window, "setContentMinSize:"_sel,
                                                CGSizeMake(width, height));
    } else if (hints == WEBVIEW_HINT_MAX) {
      ((void (*)(id, SEL, CGSize))objc_msgSend)(m_window, "setContentMaxSize:"_sel,
                                                CGSizeMake(width, height));
    } else {
      ((void (*)(id, SEL, CGRect, BOOL, BOOL))objc_msgSend)(
          m_window, "setFrame:display:animate:"_sel, CGRectMake(0, 0, width, height), 1, 0);
    }
    ((void (*)(id, SEL))objc_msgSend)(m_window, "center"_sel);
  }
  void navigate(const std::string url) {
    auto nsurl = ((id(*)(id, SEL, id))objc_msgSend)(
        "NSURL"_cls, "URLWithString:"_sel,
        ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel,
                                                     url.c_str()));

    ((void (*)(id, SEL, id))objc_msgSend)(
        m_webview, "loadRequest:"_sel,
        ((id(*)(id, SEL, id))objc_msgSend)("NSURLRequest"_cls, "requestWithURL:"_sel, nsurl));
  }
  void set_html(const std::string html) {
    ((void (*)(id, SEL, id, id))objc_msgSend)(
        m_webview, "loadHTMLString:baseURL:"_sel,
        ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel,
                                                     html.c_str()),
        nullptr);
  }
  void init(const std::string js) {
    // Equivalent Obj-C:
    // [m_manager addUserScript:[[WKUserScript alloc] initWithSource:[NSString
    // stringWithUTF8String:js.c_str()]
    // injectionTime:WKUserScriptInjectionTimeAtDocumentStart
    // forMainFrameOnly:YES]]
    ((void (*)(id, SEL, id))objc_msgSend)(
        m_manager, "addUserScript:"_sel,
        ((id(*)(id, SEL, id, long, BOOL))objc_msgSend)(
            ((id(*)(id, SEL))objc_msgSend)("WKUserScript"_cls, "alloc"_sel),
            "initWithSource:injectionTime:forMainFrameOnly:"_sel,
            ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls,
                                                         "stringWithUTF8String:"_sel, js.c_str()),
            WKUserScriptInjectionTimeAtDocumentStart, 1));
  }
  void eval(const std::string js) {
    ((void (*)(id, SEL, id, id))objc_msgSend)(
        m_webview, "evaluateJavaScript:completionHandler:"_sel,
        ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel,
                                                     js.c_str()),
        nullptr);
  }

 private:
  virtual void on_message(const std::string msg) = 0;
  void close() { ((void (*)(id, SEL))objc_msgSend)(m_window, "close"_sel); }
  id m_window;
  id m_webview;
  id m_manager;
};

using browser_engine = cocoa_wkwebview_engine;

}  // namespace vstwebview
