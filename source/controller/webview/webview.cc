#include "controller/webview/webview.h"

namespace webview {

namespace {

// Convert ASCII hex digit to a nibble (four bits, 0 - 15).
//
// Use unsigned to avoid signed overflow UB.
static inline unsigned char hex2nibble(unsigned char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return 10 + (c - 'a');
  } else if (c >= 'A' && c <= 'F') {
    return 10 + (c - 'A');
  }
  return 0;
}

// Convert ASCII hex string (two characters) to byte.
//
// E.g., "0B" => 0x0B, "af" => 0xAF.
static inline char hex2char(const char *p) {
  return hex2nibble(p[0]) * 16 + hex2nibble(p[1]);
}

inline std::string url_decode(const std::string st) {
  std::string decoded;
  const char *s = st.c_str();
  size_t length = strlen(s);
  for (unsigned int i = 0; i < length; i++) {
    if (s[i] == '%') {
      decoded.push_back(hex2char(s + i + 1));
      i = i + 2;
    } else if (s[i] == '+') {
      decoded.push_back(' ');
    } else {
      decoded.push_back(s[i]);
    }
  }
  return decoded;
}

inline std::string html_from_uri(const std::string s) {
  if (s.substr(0, 15) == "data:text/html,") {
    return url_decode(s.substr(15));
  }
  return "";
}

inline int json_parse_c(const char *s, size_t sz, const char *key, size_t keysz,
                        const char **value, size_t *valuesz) {
  enum {
    JSON_STATE_VALUE,
    JSON_STATE_LITERAL,
    JSON_STATE_STRING,
    JSON_STATE_ESCAPE,
    JSON_STATE_UTF8
  } state = JSON_STATE_VALUE;
  const char *k = NULL;
  int index = 1;
  int depth = 0;
  int utf8_bytes = 0;

  if (key == NULL) {
    index = keysz;
    keysz = 0;
  }

  *value = NULL;
  *valuesz = 0;

  for (; sz > 0; s++, sz--) {
    enum {
      JSON_ACTION_NONE,
      JSON_ACTION_START,
      JSON_ACTION_END,
      JSON_ACTION_START_STRUCT,
      JSON_ACTION_END_STRUCT
    } action = JSON_ACTION_NONE;
    unsigned char c = *s;
    switch (state) {
    case JSON_STATE_VALUE:
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' ||
          c == ':') {
        continue;
      } else if (c == '"') {
        action = JSON_ACTION_START;
        state = JSON_STATE_STRING;
      } else if (c == '{' || c == '[') {
        action = JSON_ACTION_START_STRUCT;
      } else if (c == '}' || c == ']') {
        action = JSON_ACTION_END_STRUCT;
      } else if (c == 't' || c == 'f' || c == 'n' || c == '-' ||
                 (c >= '0' && c <= '9')) {
        action = JSON_ACTION_START;
        state = JSON_STATE_LITERAL;
      } else {
        return -1;
      }
      break;
    case JSON_STATE_LITERAL:
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' ||
          c == ']' || c == '}' || c == ':') {
        state = JSON_STATE_VALUE;
        s--;
        sz++;
        action = JSON_ACTION_END;
      } else if (c < 32 || c > 126) {
        return -1;
      } // fallthrough
    case JSON_STATE_STRING:
      if (c < 32 || (c > 126 && c < 192)) {
        return -1;
      } else if (c == '"') {
        action = JSON_ACTION_END;
        state = JSON_STATE_VALUE;
      } else if (c == '\\') {
        state = JSON_STATE_ESCAPE;
      } else if (c >= 192 && c < 224) {
        utf8_bytes = 1;
        state = JSON_STATE_UTF8;
      } else if (c >= 224 && c < 240) {
        utf8_bytes = 2;
        state = JSON_STATE_UTF8;
      } else if (c >= 240 && c < 247) {
        utf8_bytes = 3;
        state = JSON_STATE_UTF8;
      } else if (c >= 128 && c < 192) {
        return -1;
      }
      break;
    case JSON_STATE_ESCAPE:
      if (c == '"' || c == '\\' || c == '/' || c == 'b' || c == 'f' ||
          c == 'n' || c == 'r' || c == 't' || c == 'u') {
        state = JSON_STATE_STRING;
      } else {
        return -1;
      }
      break;
    case JSON_STATE_UTF8:
      if (c < 128 || c > 191) {
        return -1;
      }
      utf8_bytes--;
      if (utf8_bytes == 0) {
        state = JSON_STATE_STRING;
      }
      break;
    default:
      return -1;
    }

    if (action == JSON_ACTION_END_STRUCT) {
      depth--;
    }

    if (depth == 1) {
      if (action == JSON_ACTION_START || action == JSON_ACTION_START_STRUCT) {
        if (index == 0) {
          *value = s;
        } else if (keysz > 0 && index == 1) {
          k = s;
        } else {
          index--;
        }
      } else if (action == JSON_ACTION_END ||
                 action == JSON_ACTION_END_STRUCT) {
        if (*value != NULL && index == 0) {
          *valuesz = (size_t)(s + 1 - *value);
          return 0;
        } else if (keysz > 0 && k != NULL) {
          if (keysz == (size_t)(s - k - 1) && memcmp(key, k + 1, keysz) == 0) {
            index = 0;
          } else {
            index = 2;
          }
          k = NULL;
        }
      }
    }

    if (action == JSON_ACTION_START_STRUCT) {
      depth++;
    }
  }
  return -1;
}

inline std::string json_escape(std::string s) {
  // TODO: implement
  return '"' + s + '"';
}

inline int json_unescape(const char *s, size_t n, char *out) {
  int r = 0;
  if (*s++ != '"') {
    return -1;
  }
  while (n > 2) {
    char c = *s;
    if (c == '\\') {
      s++;
      n--;
      switch (*s) {
      case 'b':
        c = '\b';
        break;
      case 'f':
        c = '\f';
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      case '\\':
        c = '\\';
        break;
      case '/':
        c = '/';
        break;
      case '\"':
        c = '\"';
        break;
      default: // TODO: support unicode decoding
        return -1;
      }
    }
    if (out != NULL) {
      *out++ = c;
    }
    s++;
    n--;
    r++;
  }
  if (*s != '"') {
    return -1;
  }
  if (out != NULL) {
    *out = '\0';
  }
  return r;
}

inline std::string json_parse(const std::string s, const std::string key,
                              const int index) {
  const char *value;
  size_t value_sz;
  if (key == "") {
    json_parse_c(s.c_str(), s.length(), nullptr, index, &value, &value_sz);
  } else {
    json_parse_c(s.c_str(), s.length(), key.c_str(), key.length(), &value,
                 &value_sz);
  }
  if (value != nullptr) {
    if (value[0] != '"') {
      return std::string(value, value_sz);
    }
    int n = json_unescape(value, value_sz, nullptr);
    if (n > 0) {
      char *decoded = new char[n + 1];
      json_unescape(value, value_sz, decoded);
      std::string result(decoded, n);
      delete[] decoded;
      return result;
    }
  }
  return "";
}

}  // namespace

std::string url_encode(const std::string s) {
  std::string encoded;
  for (unsigned int i = 0; i < s.length(); i++) {
    auto c = s[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded = encoded + c;
    } else {
      char hex[4];
      snprintf(hex, sizeof(hex), "%%%02x", c);
      encoded = encoded + hex;
    }
  }
  return encoded;
}

void webview::navigate(const std::string url) {
  if (url == "") {
    engine_->navigate("data:text/html," +
        url_encode("<html><body>Hello</body></html>"));
    return;
  }
  engine_->navigate(url);
}

void webview::bind(const std::string name, webview::sync_binding_t fn) {
  bind(
      name,
      [](std::string seq, std::string req, void *arg) {
        auto pair = static_cast<sync_binding_ctx_t *>(arg);
        pair->first->resolve(seq, 0, pair->second(req));
      },
      new sync_binding_ctx_t(this, fn));
}

void webview::bind(const std::string name, webview::binding_t f, void *arg) {
  auto js = "(function() { var name = '" + name + "';" + R"(
     var RPC = window._rpc = (window._rpc || {nextSeq: 1});
     window[name] = function() {
       var seq = RPC.nextSeq++;
       var promise = new Promise(function(resolve, reject) {
         RPC[seq] = {
           resolve: resolve,
           reject: reject,
         };
       });
       window.external.invoke(JSON.stringify({
         id: seq,
         method: name,
         params: Array.prototype.slice.call(arguments),
       }));
       return promise;
     }
   })())";
  engine_->init(js);
  bindings[name] = new binding_ctx_t(new binding_t(f), arg);
}

void webview::unbind(const std::string name) {
  if (bindings.find(name) != bindings.end()) {
    auto js = "delete window['" + name + "'];";
    engine_->init(js);
    engine_->eval(js);
    delete bindings[name]->first;
    delete static_cast<sync_binding_ctx_t *>(bindings[name]->second);
    delete bindings[name];
    bindings.erase(name);
  }
}

void webview::resolve(const std::string seq, int status,
                      const std::string result) {
  engine_->dispatch([=]() {
    if (status == 0) {
      engine_->eval("window._rpc[" + seq + "].resolve(" + result +
          "); delete window._rpc[" + seq + "]");
    } else {
      engine_->eval("window._rpc[" + seq + "].reject(" + result +
          "); delete window._rpc[" + seq + "]");
    }
  });
}

void webview::on_message(const std::string msg) {
  auto seq = json_parse(msg, "id", 0);
  auto name = json_parse(msg, "method", 0);
  auto args = json_parse(msg, "params", 0);
  if (bindings.find(name) == bindings.end()) {
    return;
  }
  auto fn = bindings[name];
  (*fn->first)(seq, args, fn->second);
}

void webview::run() {
  engine_->run();
}

void webview::terminate() {
  engine_->terminate();
}

void webview::dispatch(dispatch_fn_t f) {
  engine_->dispatch(f);
}

void webview::set_title(const std::string title) {
  engine_->set_title(title);
}

void webview::set_size(int width, int height, int hints) {
  engine_->set_size(width, height, hints);
}

void webview::set_html(const std::string html) {
  engine_->set_html(html);
}

void webview::eval(const std::string js) {
  engine_->eval(js);
}

void webview::init(const std::string js) {
  engine_->init(js);
}

void *webview::window() const { return engine_->window(); }

webview *MakeWebview(bool debug, void *wnd) {  {
    webview::FactoryFn factory_fn =
        std::bind(MakeEngine, debug, wnd, std::placeholders::_1);
    return new webview(factory_fn);
  }
}

}  // namespace webview