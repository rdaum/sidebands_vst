#pragma once

/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once


// Window size hints
#define WEBVIEW_HINT_NONE 0  // Width and height are default size
#define WEBVIEW_HINT_MIN 1   // Width and height are minimum bounds
#define WEBVIEW_HINT_MAX 2   // Width and height are maximum bounds
#define WEBVIEW_HINT_FIXED 3 // Window size can not be changed by a user

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cstring>

namespace webview {
using dispatch_fn_t = std::function<void()>;

std::string url_encode(const std::string s);

class browser_engine {
public:
  virtual void run() = 0;
  virtual void terminate() = 0;
  virtual void dispatch(dispatch_fn_t f) = 0;
  virtual void set_title(const std::string title) = 0;
  virtual void set_size(int width, int height, int hints) = 0;
  virtual void navigate(const std::string url) = 0;
  virtual void set_html(const std::string html) = 0;
  virtual void eval(const std::string js) = 0;
  virtual void init(const std::string js) = 0;
  virtual void *window() const = 0;
};


// Facade class, delegates to underlying implementation (engine_)
class webview : public browser_engine {
public:

  using FactoryFn = std::function<std::unique_ptr<browser_engine>(webview *webview)>;
  explicit webview(FactoryFn factory_fn) {
      engine_ = factory_fn(this);
  }

  using binding_t = std::function<void(std::string, std::string, void *)>;
  using binding_ctx_t = std::pair<binding_t *, void *>;

  using sync_binding_t = std::function<std::string(std::string)>;
  using sync_binding_ctx_t = std::pair<webview *, sync_binding_t>;

  void bind(const std::string name, sync_binding_t fn);
  void bind(const std::string name, binding_t f, void *arg);
  void unbind(const std::string name);
  void resolve(const std::string seq, int status, const std::string result);

  void on_message(const std::string msg);

  // browser_engine overrides
  void run() override;
  void terminate() override;
  void dispatch(dispatch_fn_t f) override;
  void set_title(const std::string title) override;
  void set_size(int width, int height, int hints) override;
  void set_html(const std::string html) override;
  void eval(const std::string js) override;
  void init(const std::string js) override;
  void navigate(const std::string url) override;
  void *window() const override;

private:
  std::map<std::string, binding_ctx_t *> bindings_;
  std::unique_ptr<browser_engine> engine_;
};

std::unique_ptr<browser_engine> MakeEngine(bool debug, void *window, webview *webview);

webview *MakeWebview(bool debug, void *wnd);

} // namespace webview
