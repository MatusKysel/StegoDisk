/**
* @file json_object.h
* @date 2016
* @brief Wrapper for better json manipulation
*
*/

#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H

#include <math.h>
#include <cstdlib>
#include <cstdio>

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>
#include <iterator>
#include <type_traits>

#ifdef _MSC_VER
#define SNPRINTF _snprintf_s
#pragma warning(push)
#pragma warning(disable : 4244) // conversion from int to char
#else
#define SNPRINTF snprintf
#endif

namespace json {

template<typename Iter>
Iter NextIter(Iter it) {
  return ++it;
}

static const std::string kStringChars("\\\"\n\t\r");

class JsonObject {
  friend class PJson;
public:
  typedef std::vector<JsonObject> ArrayType;
  typedef std::map<std::string, JsonObject> ObjectType;

  enum NodeType {
    NULL_VAL,
    BOOLEAN,
    NUMBER,
    STRING,
    OBJECT,
    ARRAY
  };

  JsonObject() : type_(NULL_VAL) {}
  JsonObject(const JsonObject &o) : type_(o.type_) {
    switch (type_) {
      case NULL_VAL:
        break;
      case BOOLEAN:
        bool_val_ = o.bool_val_;
        break;
      case NUMBER:
        number_val_ = o.number_val_;
        break;
      case STRING:
        string_.reset(new std::string(*o.string_));
        break;
      case ARRAY:
        array_.reset(new std::vector<JsonObject>(*o.array_));
        break;
      case OBJECT:
        object_.reset(new std::map<std::string, JsonObject>(*o.object_));
        break;
    }
  }

  JsonObject(bool val) : type_(BOOLEAN), bool_val_(val) {}

  template<typename T>
  JsonObject(T val, typename std::enable_if<std::is_integral<T>::value>::type *ignore = 0) : type_(NUMBER), number_val_(val) {}

  template<typename T>
  JsonObject(T val, typename std::enable_if<std::is_floating_point<T>::value>::type *ignore = 0) : type_(NUMBER), number_val_(val) {}

  JsonObject(const char *val) : type_(STRING), string_(new std::string(val)) {}
  JsonObject(const std::string &val) : type_(STRING), string_(new std::string(val)) {}
  JsonObject(NodeType type) : type_(type) {
    switch (type_) {
      case BOOLEAN:
        bool_val_ = false;
        break;
      case NUMBER:
        number_val_ = 0.0;
        break;
      case STRING:
        InitString();
        break;
      case ARRAY:
        InitArray();
        break;
      case OBJECT:
        InitObject();
        break;
      default:
        break;
    }
  }

  virtual ~JsonObject() {}

  inline JsonObject &operator=(const JsonObject &o) {
    if (&o == this) return *this;

    type_ = o.type_;
    switch (type_) {
      case NULL_VAL:
        break;
      case BOOLEAN:
        bool_val_ = o.bool_val_;
        break;
      case NUMBER:
        number_val_ = o.number_val_;
        break;
      case STRING:
        string_.reset(new std::string(*o.string_));
        break;
      case ARRAY:
        array_.reset(new std::vector<JsonObject>(*o.array_));
        break;
      case OBJECT:
        object_.reset(new std::map<std::string, JsonObject>(*o.object_));
        break;
    }

    return *this;
  }

  inline void Swap(JsonObject &o) {
    std::swap(type_, o.type_);
    std::swap(bool_val_, o.bool_val_);
    std::swap(number_val_, o.number_val_);
    string_.swap(o.string_);
    array_.swap(o.array_);
    object_.swap(o.object_);
  }

  inline NodeType type() const { return type_; }

  void Assign(bool val) { type_ = BOOLEAN; bool_val_ = val; }

  template<typename T>
  void Assign(T val, typename std::enable_if<std::is_integral<T>::value>::type *ignore = 0) { type_ = NUMBER; number_val_ = val; }

  template<typename T>
  void Assign(T val, typename std::enable_if<std::is_floating_point<T>::value>::type *ignore = 0) { type_ = NUMBER; number_val_ = val; }

  void SetNull() { type_ = NULL_VAL; }

  inline bool IsNull() const { return type_ == NULL_VAL; }

  bool ToBool(bool def_val = false) const {
    if (IsBool()) return bool_val_;
    else return def_val;
  }

  inline bool IsBool() const { return (type_ == BOOLEAN); }

  inline std::int64_t ToInt(std::int64_t def_val = 0) const {
    return (IsNumber()) ? static_cast<std::int64_t>(number_val_) : def_val;
  }

  inline std::uint64_t ToUInt(std::uint64_t def_val = 0) const {
    return static_cast<std::uint64_t>(ToInt(def_val));
  }

  inline bool IsNumber() const { return (type_ == NUMBER); }

  double ToNumber(double def_val = 0.0f) const {
    return (IsNumber()) ? number_val_ : def_val;
  }

  inline static const std::string &BoolToString(bool v) {
    static const std::string kTrueString("true");
    static const std::string kFalseString("false");
    return (v) ? kTrueString : kFalseString;
  }

  inline bool IsString() const { return (type_ == STRING); }
  inline const std::string &ToString() const {
    if (IsString()) {
      return *string_;
    } else if (IsBool()) {
      return BoolToString(bool_val_);
    } else {
      static const std::string kEmptyString;
      return kEmptyString;
    }
  }

  inline const std::string &ToString(const std::string &def_val) const {
    if (IsString()) return *string_;
    else if (IsBool()) return BoolToString(bool_val_);
    else return def_val;
  }

  inline std::string &ToString() {
    if (!IsString()) InitString();
    return *string_;
  }

  inline std::string ToLowerString(const std::string &def_val) const {
    if (IsString()) {
      std::string str;
      std::transform(string_->begin(), string_->end(), std::back_inserter(str), ::tolower);
      return str;
    } else if (IsBool()) {
      return BoolToString(bool_val_);
    } else {
      return def_val;
    }
  }

  inline std::string ToLowerString() const {
    if (IsString()) {
      std::string str;
      std::transform(string_->begin(), string_->end(), std::back_inserter(str), ::tolower);
      return str;
    } else if (IsBool()) {
      return BoolToString(bool_val_);
    } else {
      static const std::string kEmptyString;
      return kEmptyString;
    }
  }

  inline operator std::string() const { return ToString(); }

  inline const std::vector<JsonObject> &ToArray() const {
    if (IsArray()) {
      return *array_;
    } else {
      static const std::vector<JsonObject> kEmptyArray;
      return kEmptyArray;
    }
  }

  inline std::vector<JsonObject> &ToArray() {
    if (!IsArray()) InitArray();
    return *array_;
  }

  inline bool IsArray() const { return (type_ == ARRAY); }
  inline void AddToArray(const JsonObject &node) {
    if (type_ != ARRAY) InitArray();
    array_->push_back(node);
  }
  inline size_t ArraySize() const { return (type_ == ARRAY) ? array_->size() : 0; }

  inline void GrowArray(size_t size, NodeType type = NULL_VAL) {
    if (type_ != ARRAY) InitArray();
    while (size > array_->size()) array_->push_back(JsonObject(type));
  }

  inline JsonObject &operator[](size_t pos) {
    if (type_ != ARRAY) InitArray();
    GrowArray(pos + 1);
    return (*array_)[pos];
  }
  inline const JsonObject &operator[](size_t pos) const {
    if (type_ != ARRAY || pos > (array_->size() - 1)) return NullNode();
    return (*array_)[pos];
  }

  inline bool IsObject() const { return (type_ == OBJECT); }
  inline const std::map<std::string, JsonObject> &ToObject() const {
    if (IsObject()) {
      return *object_;
    } else {
      static const std::map<std::string, JsonObject> kEmptyObject;
      return kEmptyObject;
    }
  }

  inline std::map<std::string, JsonObject> &ToObject() {
    if (!IsObject()) InitObject();
    return *object_;
  }

  inline JsonObject &operator[](std::string key) {
    if (type_ != OBJECT) InitObject();
    return (*object_)[key];
  }

  inline const JsonObject &operator[](std::string key) const {
    if (type_ != OBJECT) return NullNode();
    return (*object_)[key];
  }


  inline bool Contains(std::string key) const {
    if (IsObject()) return object_->count(key);
    return false;
  }

  template<typename T>
  inline void AddToObject(std::string key, T value) {
    if (type_ != OBJECT) InitObject();
    (*object_)[key] = JsonObject(value);
  }

  static const JsonObject &NullNode() {
    static const JsonObject node;
    return node;
  }

  inline std::string to_str() const {
    switch (type_) {
      case NULL_VAL:
        return "null";
      case BOOLEAN:
        return bool_val_ ? "true" : "false";
      case NUMBER: {
          char buf[256];
          double tmp;
          SNPRINTF(buf, sizeof(buf), fabs(number_val_) < (1ULL << 53) && modf(number_val_, &tmp) == 0 ? "%.f" : "%.17g", number_val_);
          return buf;
        }
      case STRING:
        return *string_;
      case ARRAY:
        return "array";
      case OBJECT:
        return "object";
      default:
        return std::string();
    }
  }

  template <typename Iter>
  void static copy(const std::string &s, Iter oi) {
    std::copy(s.begin(), s.end(), oi);
  }

  template <typename Iter>
  static void serialize_str(const std::string &s, Iter oi) {
    *oi++ = '"';
    for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {
      switch (*i) {
#define MAP(val, sym) case val: copy(sym, oi); break
        MAP('"', "\\\"");
        MAP('\\', "\\\\");
        MAP('/', "\\/");
        MAP('\b', "\\b");
        MAP('\f', "\\f");
        MAP('\n', "\\n");
        MAP('\r', "\\r");
        MAP('\t', "\\t");
#undef MAP
        default:
          if ((unsigned char)*i < 0x20 || *i == 0x7f) {
            char buf[7];
            SNPRINTF(buf, sizeof(buf), "\\u%04x", *i & 0xff);
            std::copy(buf, buf + 6, oi);
          } else {
            *oi++ = *i;
          }
          break;
      }
    }
    *oi++ = '"';
  }

  template <typename Iter>
  void Serialize(Iter oi) const {
    switch (type_) {
      case STRING:
        serialize_str(*string_, oi);
        break;
      case ARRAY: {
          *oi++ = '[';
          for (ArrayType::const_iterator i = array_->begin();
               i != array_->end();
               ++i)
          {
            if (i != array_->begin()) {
              *oi++ = ',';
            }
            i->Serialize(oi);
          }
          *oi++ = ']';
          break;
        }
      case OBJECT: {
          *oi++ = '{';
          for (ObjectType::const_iterator i = object_->begin();
               i != object_->end();
               ++i)
          {
            if (i != object_->begin()) {
              *oi++ = ',';
            }
            serialize_str(i->first, oi);
            *oi++ = ':';
            i->second.Serialize(oi);
          }
          *oi++ = '}';
          break;
        }
      default:
        copy(to_str(), oi);
        break;
    }
  }

  template <typename Iter>
  void PrettySerialize(Iter oi, size_t indent = 0) const {
    switch (type_) {
      case STRING:
        serialize_str(*string_, oi);
        break;
      case ARRAY: {
          *oi++ = '[';
          *oi++ = '\n';
          for (ArrayType::const_iterator i = array_->begin();
               i != array_->end();
               ++i)
          {
            if (i != array_->begin()) {
              *oi++ = ',';
              *oi++ = '\n';
            }
            Indent(oi, indent + 1);
            i->PrettySerialize(oi, indent + 1);
          }
          *oi++ = '\n';
          Indent(oi, indent);
          *oi++ = ']';
          break;
        }
      case OBJECT: {
          *oi++ = '{';
          *oi++ = '\n';
          for (ObjectType::const_iterator i = object_->begin();
               i != object_->end();
               ++i)
          {
            if (i != object_->begin()) {
              *oi++ = ',';
              *oi++ = '\n';
            }
            Indent(oi, indent + 1);
            serialize_str(i->first, oi);
            *oi++ = ' ';
            *oi++ = ':';
            *oi++ = ' ';
            i->second.PrettySerialize(oi, indent + 1);
          }
          *oi++ = '\n';
          Indent(oi, indent);
          *oi++ = '}';
          break;
        }
      default:
        copy(to_str(), oi);
        break;
    }
  }

  inline void Serialize(std::string *s) const {
    Serialize(std::back_inserter(*s));
  }

  inline void PrettySerialize(std::string *s) const {
    PrettySerialize(std::back_inserter(*s));
  }

  inline std::string Serialize() const {
    std::string s;
    Serialize(&s);
    return s;
  }

  inline std::string PrettySerialize() const {
    std::string s;
    PrettySerialize(&s);
    return s;
  }

  inline static std::string Indent(size_t indent) { return std::string(indent, '\t'); }

  template<typename Iter>
  inline static void Indent(Iter oi, size_t indent) {
    for (size_t i = 0; i < (indent * 2); ++i) *oi++ = ' ';
  }

private:
  void InitString() {
    if (!string_) string_.reset(new std::string);
    else string_->clear();
    type_ = STRING;
  }

  void InitArray() {
    if (!array_) array_.reset(new std::vector<JsonObject>);
    else array_->clear();
    type_ = ARRAY;
  }

  void InitObject() {
    if (!object_) object_.reset(new std::map<std::string, JsonObject>);
    else object_->clear();
    type_ = OBJECT;
  }

  NodeType type_;
  bool bool_val_;
  double number_val_;
  std::unique_ptr<std::string> string_;
  std::unique_ptr<ArrayType> array_;
  std::unique_ptr<ObjectType> object_;

  static const JsonObject kNullNode;
};

inline std::ostream &operator<<(std::ostream &os, const JsonObject &o) {
  o.Serialize(std::ostream_iterator<char>(os));
  return os;
}

} // json

#endif // JSON_OBJECT_H
