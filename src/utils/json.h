/*
 * Copyright 2009-2010 Cybozu Labs, Inc.
 * Copyright 2011 Kazuho Oku
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY CYBOZU LABS, INC. ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL CYBOZU LABS, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Cybozu Labs, Inc.
 *
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "api_mask.h"
#include "utils/json_object.h"

namespace json {
template <typename Iter>
class input {
protected:
  Iter cur_, end_;
  int last_ch_;
  bool ungot_;
  int line_;
public:
  input(const Iter& first, const Iter& last) : cur_(first), end_(last), last_ch_(-1), ungot_(false), line_(1) {}
  int getc() {
    if (ungot_) {
      ungot_ = false;
      return last_ch_;
    }
    if (cur_ == end_) {
      last_ch_ = -1;
      return -1;
    }
    if (last_ch_ == '\n') {
      line_++;
    }
    last_ch_ = *cur_++ & 0xff;
    return last_ch_;
  }
  void ungetc() {
    if (last_ch_ != -1) {
      ungot_ = true;
    }
  }
  Iter cur() const { return cur_; }
  int line() const { return line_; }
  void skip_ws() {
    while (1) {
      int ch = getc();
      if (! (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')) {
        ungetc();
        break;
      }
    }
  }
  bool expect(int expect) {
    skip_ws();
    if (getc() != expect) {
      ungetc();
      return false;
    }
    return true;
  }
  bool match(const std::string& pattern) {
    for (std::string::const_iterator pi(pattern.begin());
         pi != pattern.end();
         ++pi) {
      if (getc() != *pi) {
        ungetc();
        return false;
      }
    }
    return true;
  }
};

template<typename Iter>
inline int _parse_quadhex(input<Iter> &in) {
  int uni_ch = 0, hex;
  for (int i = 0; i < 4; i++) {
    if ((hex = in.getc()) == -1) {
      return -1;
    }
    if ('0' <= hex && hex <= '9') {
      hex -= '0';
    } else if ('A' <= hex && hex <= 'F') {
      hex -= 'A' - 0xa;
    } else if ('a' <= hex && hex <= 'f') {
      hex -= 'a' - 0xa;
    } else {
      in.ungetc();
      return -1;
    }
    uni_ch = uni_ch * 16 + hex;
  }
  return uni_ch;
}

template<typename String, typename Iter>
inline bool _parse_codepoint(String& out, input<Iter>& in) {
  int uni_ch;
  if ((uni_ch = _parse_quadhex(in)) == -1) {
    return false;
  }
  if (0xd800 <= uni_ch && uni_ch <= 0xdfff) {
    if (0xdc00 <= uni_ch) {
      // a second 16-bit of a surrogate pair appeared
      return false;
    }
    // first 16-bit of surrogate pair, get the next one
    if (in.getc() != '\\' || in.getc() != 'u') {
      in.ungetc();
      return false;
    }
    int second = _parse_quadhex(in);
    if (! (0xdc00 <= second && second <= 0xdfff)) {
      return false;
    }
    uni_ch = ((uni_ch - 0xd800) << 10) | ((second - 0xdc00) & 0x3ff);
    uni_ch += 0x10000;
  }
  if (uni_ch < 0x80) {
    out.push_back(uni_ch);
  } else {
    if (uni_ch < 0x800) {
      out.push_back(0xc0 | (uni_ch >> 6));
    } else {
      if (uni_ch < 0x10000) {
        out.push_back(0xe0 | (uni_ch >> 12));
      } else {
        out.push_back(0xf0 | (uni_ch >> 18));
        out.push_back(0x80 | ((uni_ch >> 12) & 0x3f));
      }
      out.push_back(0x80 | ((uni_ch >> 6) & 0x3f));
    }
    out.push_back(0x80 | (uni_ch & 0x3f));
  }
  return true;
}

template<typename String, typename Iter>
inline bool _parse_string(String& out, input<Iter>& in) {
  while (1) {
    int ch = in.getc();
    if (ch < ' ') {
      in.ungetc();
      return false;
    } else if (ch == '"') {
      return true;
    } else if (ch == '\\') {
      if ((ch = in.getc()) == -1) {
        return false;
      }
      switch (ch) {
#define MAP(sym, val) case sym: out.push_back(val); break
        MAP('"', '\"');
        MAP('\\', '\\');
        MAP('/', '/');
        MAP('b', '\b');
        MAP('f', '\f');
        MAP('n', '\n');
        MAP('r', '\r');
        MAP('t', '\t');
#undef MAP
        case 'u':
          if (! _parse_codepoint(out, in)) {
            return false;
          }
          break;
        default:
          return false;
      }
    } else {
      out.push_back(ch);
    }
  }
  return false;
}

template <typename Context, typename Iter>
inline bool _parse_array(Context& ctx, input<Iter>& in) {
  if (! ctx.parse_array_start()) {
    return false;
  }
  size_t idx = 0;
  if (in.expect(']')) {
    return ctx.parse_array_stop(idx);
  }
  do {
    if (! ctx.parse_array_item(in, idx)) {
      return false;
    }
    idx++;
  } while (in.expect(','));
  return in.expect(']') && ctx.parse_array_stop(idx);
}

template <typename Context, typename Iter>
inline bool _parse_object(Context& ctx, input<Iter>& in) {
  if (! ctx.parse_object_start()) {
    return false;
  }
  if (in.expect('}')) {
    return true;
  }
  do {
    std::string key;
    if (! in.expect('"')
        || ! _parse_string(key, in)
        || ! in.expect(':')) {
      return false;
    }
    if (! ctx.parse_object_item(in, key)) {
      return false;
    }
  } while (in.expect(','));
  return in.expect('}');
}

template <typename Iter>
inline bool _parse_number(double& out, input<Iter>& in) {
  std::string num_str;
  while (1) {
    int ch = in.getc();
    if (('0' <= ch && ch <= '9') || ch == '+' || ch == '-' || ch == '.'
        || ch == 'e' || ch == 'E') {
      num_str.push_back(ch);
    } else {
      in.ungetc();
      break;
    }
  }
  char* endp;
  out = strtod(num_str.c_str(), &endp);
  return endp == num_str.c_str() + num_str.size();
}

template <typename Context, typename Iter>
inline bool _parse(Context& ctx, input<Iter>& in) {
  in.skip_ws();
  int ch = in.getc();
  switch (ch) {
#define IS(ch, text, op) case ch: \
  if (in.match(text) && op) { \
  return true; \
  } else { \
  return false; \
  }
    IS('n', "ull", ctx.set_null());
    IS('f', "alse", ctx.set_bool(false));
    IS('t', "rue", ctx.set_bool(true));
#undef IS
    case '"':
      return ctx.parse_string(in);
    case '[':
      return _parse_array(ctx, in);
    case '{':
      return _parse_object(ctx, in);
    default:
      if (('0' <= ch && ch <= '9') || ch == '-') {
        in.ungetc();
        double f;
        if (_parse_number(f, in)) {
          ctx.set_number(f);
          return true;
        } else {
          return false;
        }
      }
      break;
  }
  in.ungetc();
  return false;
}

class deny_parse_context {
public:
  bool set_null() { return false; }
  bool set_bool(bool) { return false; }
  bool set_number(double) { return false; }
  template <typename Iter> bool parse_string(input<Iter>&) { return false; }
  bool parse_array_start() { return false; }
  template <typename Iter> bool parse_array_item(input<Iter>&, size_t) {
    return false;
  }
  bool parse_array_stop(size_t) { return false; }
  bool parse_object_start() { return false; }
  template <typename Iter> bool parse_object_item(input<Iter>&, const std::string&) {
    return false;
  }
};

class default_parse_context {
protected:
  JsonObject* out_;
public:
  default_parse_context(JsonObject* out) : out_(out) {}
  bool set_null() {
    *out_ = JsonObject();
    return true;
  }
  bool set_bool(bool b) {
    *out_ = JsonObject(b);
    return true;
  }
  bool set_number(double f) {
    *out_ = JsonObject(f);
    return true;
  }
  template<typename Iter>
  bool parse_string(input<Iter>& in) {
    *out_ = JsonObject(JsonObject::STRING);
    return _parse_string(out_->ToString(), in);
  }
  bool parse_array_start() {
    *out_ = JsonObject(JsonObject::ARRAY);
    return true;
  }
  template <typename Iter>
  bool parse_array_item(input<Iter>& in, size_t) {
    std::vector<JsonObject> &a = out_->ToArray();
    a.push_back(JsonObject());
    default_parse_context ctx(&a.back());
    return _parse(ctx, in);
  }
  bool parse_array_stop(size_t) { return true; }
  bool parse_object_start() {
    *out_ = JsonObject(JsonObject::OBJECT);
    return true;
  }
  template <typename Iter>
  bool parse_object_item(input<Iter>& in, const std::string& key) {
    //    object& o = out_->get<object>();
    std::map<std::string, JsonObject> &o = out_->ToObject();
    default_parse_context ctx(&o[key]);
    return _parse(ctx, in);
  }
private:
  default_parse_context(const default_parse_context&);
  default_parse_context& operator=(const default_parse_context&);
};

class null_parse_context {
public:
  struct dummy_str {
    void push_back(int) {}
  };
public:
  null_parse_context() {}
  bool set_null() { return true; }
  bool set_bool(bool) { return true; }
  bool set_number(double) { return true; }
  template <typename Iter>
  bool parse_string(input<Iter>& in) {
    dummy_str s;
    return _parse_string(s, in);
  }
  bool parse_array_start() { return true; }
  template <typename Iter>
  bool parse_array_item(input<Iter>& in, size_t) {
    return _parse(*this, in);
  }
  bool parse_array_stop(size_t) { return true; }
  bool parse_object_start() { return true; }
  template <typename Iter>
  bool parse_object_item(input<Iter>& in, const std::string&) {
    return _parse(*this, in);
  }
private:
  null_parse_context(const null_parse_context&);
  null_parse_context& operator=(const null_parse_context&);
};

template <typename Context, typename Iter>
inline Iter _parse(Context& ctx, const Iter& first, const Iter& last, std::string* err) {
  input<Iter> in(first, last);
  if (! _parse(ctx, in) && err != NULL) {
    char buf[64];
    SNPRINTF(buf, sizeof(buf), "syntax error at line %d near: ", in.line());
    *err = buf;
    while (1) {
      int ch = in.getc();
      if (ch == -1 || ch == '\n') {
        break;
      } else if (ch >= ' ') {
        err->push_back(ch);
      }
    }
  }
  return in.cur();
}

template <typename Iter>
inline static Iter parse(JsonObject& out, const Iter& first, const Iter& last, std::string* err) {
  default_parse_context ctx(&out);
  return _parse(ctx, first, last, err);
}

inline static std::string parse(JsonObject& out, std::istream& is) {
  std::string err;
  parse(out, std::istreambuf_iterator<char>(is.rdbuf()),
        std::istreambuf_iterator<char>(), &err);
  return err;
}

inline static std::string Parse(const std::string &json, JsonObject *root) {
  std::string err;

  parse(*root, json.begin(), json.end(), &err);

  return err;
}

} // json
