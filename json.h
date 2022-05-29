
#pragma once

#include <stdint.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <initializer_list>
#include <iostream>
#include <stack>
#include <cassert>

namespace {

template <typename T>
bool within_limits(int64_t n) {
    return n >= std::numeric_limits<T>::min() && n <= std::numeric_limits<T>::max();
}

}

enum class value_t: uint8_t {
    object,
    array,
    string,
    int_num,
    uint_num,
    float_num,
    boolean,
    null,
};
class json;

template <typename N, typename V>
struct name_value {
    N name;
    V value; 
};

using object_t = std::vector<name_value<std::string, json>>;
using array_t = std::vector<json>;
using string_t = std::string;

class json {
    value_t type;
    union json_value {
        object_t* object;
        array_t* array; 
        string_t* string;
        int64_t int_num;
        uint64_t uint_num;
        double float_num;
        bool boolean;
        
        json_value() : object(nullptr) {}
        json_value(const object_t& object) : object(new object_t(object)) {}
        json_value(object_t&& object) : object(new object_t(std::move(object))) {}
        json_value(const array_t& array) : array(new array_t(array)) {}
        json_value(array_t&& array) : array(new array_t(std::move(array))) {}
        json_value(const char* str) : string(new string_t(str)) {}
        json_value(const string_t& str) : string(new string_t(str)) {}
        json_value(string_t&& str) : string(new string_t(std::move(str))) {}
        json_value(int num) : int_num(num) {}
        json_value(int64_t num) : int_num(num) {}
        json_value(uint64_t num) : uint_num(num) {}
        json_value(double num) : float_num(num) {}
        json_value(bool b) : boolean(b) {}
        json_value(value_t type) {
            switch (type) {
                case value_t::object:
                    object = new object_t();
                    break;
                case value_t::array:
                    array = new array_t();
                    break;
                case value_t::string:
                    string = new string_t();
                    break;
                case value_t::int_num:
                    int_num = 0;
                    break;
                case value_t::uint_num:
                    uint_num = 0;
                    break;
                case value_t::float_num:
                    float_num = 0;
                    break;
                case value_t::boolean:
                    boolean = false;
                    break;
                case value_t::null:
                    object = nullptr;
                    break;
            }
        }
        json_value(const json_value&) = delete;
        
        void destroy(value_t type) {
            switch (type) {
                case value_t::object:
                    delete object;
                    break;
                case value_t::array:
                    delete array;
                    break;
                case value_t::string:
                    delete string;
                    break;
                default:
                    break;
            }
        }
    } value;

public:
    json() : type(value_t::null), value() {}
    json(value_t t) : type(t), value(t) {}
    json(nullptr_t null) : type(value_t::null), value() {}
    json(const char* str) : type(value_t::string), value(str) {} 
    json(const std::string& str) : type(value_t::string), value(str) {}
    json(std::string&& str) : type(value_t::string), value(std::move(str)) {}
    json(int num) : type(value_t::int_num), value(num) {}
    json(int64_t num) : type(value_t::int_num), value(num) {}
    json(uint64_t num) : type(value_t::uint_num), value(num) {}
    json(double num) : type(value_t::float_num), value(num) {}
    json(bool b) : type(value_t::boolean), value(b) {}
    
    json(std::initializer_list<json> init) {
        bool looks_like_object = true;
        for (const auto& it : init) {
            looks_like_object = it.is_array() && it.size() == 2 && it[0].is_string();
            if (!looks_like_object) {
                break;
            }
        }
        if (looks_like_object) {
            type = value_t::object;
            value = object_t();
            value.object->reserve(init.size());
            for (auto& it : init) {
                bool found = false;
                for (auto& nv : *value.object) {
                    if (nv.name == *it[0].get<std::string>()) {
                        found = true;
                        nv.value = std::move(it[1]);
                        break;
                    }
                }
                if (!found) {
                    value.object->push_back({*it[0].get<std::string>(), std::move(it[1])});
                }
            }
        } else {
            type = value_t::array;
            value = array_t(init.begin(), init.end());
        }
    }
    
    static json object() {
        return json(value_t::object);
    }

    static json object(const object_t& o) {
        json j;
        j.type = value_t::object;
        j.value = o;
        return j;
    }
    
    static json object(object_t&& o) {
        json j;
        j.type = value_t::object;
        j.value = std::move(o);
        return j;
    }
    
    static json array() {
        return json(value_t::array);
    }

    static json array(const array_t& a) {
        json j;
        j.type = value_t::array;
        j.value = a;
        return j;
    }
    
    static json array(array_t&& a) {
        json j;
        j.type = value_t::array;
        j.value = std::move(a);
        return j;
    }

    json(const json& other) : type(other.type) {
        switch (type) {
            case value_t::object:
                value = *other.value.object;
                break;
            case value_t::array:
                value = *other.value.array;
                break;
            case value_t::string:
                value = *other.value.string;
                break;
            case value_t::int_num:
                value = other.value.int_num;
                break;
            case value_t::uint_num:
                value = other.value.uint_num;
                break;
            case value_t::float_num:
                value = other.value.float_num;
                break;
            case value_t::boolean:
                value = other.value.boolean;
                break;
            case value_t::null:
                value = {};
                break;
        }
    }

    json& operator=(const json& other) {
        if (this != &other) {
            value.destroy(type);
            type = other.type;

            switch (type) {
                case value_t::object:
                    value = *other.value.object;
                    break;
                case value_t::array:
                    value = *other.value.array;
                    break;
                case value_t::string:
                    value = *other.value.string;
                    break;
                case value_t::int_num:
                    value = other.value.int_num;
                    break;
                case value_t::uint_num:
                    value = other.value.uint_num;
                    break;
                case value_t::float_num:
                    value = other.value.float_num;
                    break;
                case value_t::boolean:
                    value = other.value.boolean;
                    break;
                case value_t::null:
                    value = {};
                    break;
            }
        }
        return *this;
    }
    
    json(json&& other) : type(other.type) {
        switch (type) {
            case value_t::object:
                value = std::move(*other.value.object);
                break;
            case value_t::array:
                value = std::move(*other.value.array);
                break;
            case value_t::string:
                value = std::move(*other.value.string);
                break;
            case value_t::int_num:
                value = other.value.int_num;
                break;
            case value_t::uint_num:
                value = other.value.uint_num;
                break;
            case value_t::float_num:
                value = other.value.float_num;
                break;
            case value_t::boolean:
                value = other.value.boolean;
                break;
            case value_t::null:
                value = {};
                break;
        }
    }
    
    json& operator=(json&& other) {
        if (this != &other) {
            value.destroy(type);
            type = other.type;

            switch (type) {
                case value_t::object:
                    value = std::move(*other.value.object);
                    break;
                case value_t::array:
                    value = std::move(*other.value.array);
                    break;
                case value_t::string:
                    value = std::move(*other.value.string);
                    break;
                case value_t::int_num:
                    value = other.value.int_num;
                    break;
                case value_t::uint_num:
                    value = other.value.uint_num;
                    break;
                case value_t::float_num:
                    value = other.value.float_num;
                    break;
                case value_t::boolean:
                    value = other.value.boolean;
                    break;
                case value_t::null:
                    value = {};
                    break;
            }
        }
        return *this;
    }

    ~json() {
        value.destroy(type);
    }
    
    inline size_t empty() const {
        if (is_object()) return value.object->empty();
        if (is_array()) return value.array->empty();
        if (is_string()) return value.string->empty();
        std::abort();
    }

    inline size_t size() const {
        if (is_object()) return value.object->size();
        if (is_array()) return value.array->size();
        if (is_string()) return value.string->size();
        std::abort();
    }

    inline bool is_object() const { return type == value_t::object; }
    inline bool is_array() const { return type == value_t::array; }
    inline bool is_string() const { return type == value_t::string; }
    inline bool is_number() const { return type == value_t::int_num || type == value_t::uint_num || type == value_t::float_num; }
    inline bool is_int() const { return type == value_t::int_num; }
    inline bool is_uint() const { return type == value_t::uint_num; }
    inline bool is_double() const { return type == value_t::float_num; }
    inline bool is_boolean() const { return type == value_t::boolean; }
    inline bool is_null() const { return type == value_t::null; }

    template <typename T>
    std::optional<T> get() const;

    template <>
    std::optional<string_t> get<std::string>() const {
        return is_string() ? std::optional<string_t>(*value.string) : std::nullopt;
    }
    
    operator std::optional<string_t>() const {
        return get<string_t>();
    }
    
    template <>
    std::optional<bool> get<bool>() const {
        return is_boolean() ? std::optional<bool>(value.boolean) : std::nullopt;
    }

    template <>
    std::optional<int8_t> get<int8_t>() const {
        if (type == value_t::int_num && within_limits<int8_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }

    template <>
    std::optional<int16_t> get<int16_t>() const {
        if (type == value_t::int_num && within_limits<int16_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }

    template <>
    std::optional<int32_t> get<int32_t>() const {
        if (type == value_t::int_num && within_limits<int32_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }
    
    template <>
    std::optional<int64_t> get<int64_t>() const {
        if (type == value_t::int_num && within_limits<int64_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }
    
    template <>
    std::optional<uint8_t> get<uint8_t>() const {
        if (type == value_t::int_num && within_limits<uint8_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }
    
    template <>
    std::optional<uint16_t> get<uint16_t>() const {
        if (type == value_t::int_num && within_limits<uint16_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }
    
    template <>
    std::optional<uint32_t> get<uint32_t>() const {
        if (type == value_t::int_num && within_limits<uint32_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }
    
    template <>
    std::optional<uint64_t> get<uint64_t>() const {
        if (type == value_t::int_num && within_limits<uint64_t>(value.int_num)) {
            return value.int_num;
        }
        return std::nullopt;
    }
    
    template <>
    std::optional<float> get<float>() const {
        switch (type) {
            case value_t::float_num:
                return value.float_num;
            case value_t::int_num:
                return value.int_num;
            case value_t::uint_num: 
                return value.uint_num;
            default:
                return std::nullopt;
        }
    }
    
    template <>
    std::optional<double> get<double>() const {
        switch (type) {
            case value_t::float_num:
                return value.float_num;
            case value_t::int_num:
                return value.int_num;
            case value_t::uint_num: 
                return value.uint_num;
            default:
                return std::nullopt;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const json& j) {
        switch (j.type) {
            case value_t::object: {
                os << "{";
                if (j.value.object->empty()) {
                    os << "}"; 
                    break;
                }
                auto b = j.value.object->begin();
                auto n = b;
                n++;
                auto e = j.value.object->end();
                while (n != e) {
                    os << "\"" << b->name << "\": " << b->value << ", ";
                    ++b;
                    ++n;
                }
                os << "\"" << b->name << "\": " << b->value << "}";
                break;
            }
            case value_t::array: {
                os << "[";
                if (j.value.array->empty()) {
                    os << "]";
                    break;
                }
                auto b = j.value.array->begin();
                auto e = --j.value.array->end();
                while (b != e) {
                    os << *b++ << ", ";
                }
                os << *b << "]";
                break;
            }
            case value_t::string:
                os << "\"" << *j.value.string << "\"";
                break;
            case value_t::int_num:
                os << j.value.int_num;
                break;
            case value_t::uint_num:
                os << j.value.uint_num;
                break;
            case value_t::float_num:
                os << j.value.float_num;
                break;
            case value_t::boolean:
                os << (j.value.boolean ? "true" : "false");
                break;
            case value_t::null:
                os << "null";
                break;
        }
        return os;
    }

    // Array Operations
    
    void push_back(const json& j) {
        if (is_null()) {
            type = value_t::array;
            value = json_value(value_t::array);
        }
        value.array->push_back(j);
    }

    void push_back(json&& j) {
        if (is_null()) {
            type = value_t::array;
            value = json_value(value_t::array);
        }
        value.array->push_back(std::move(j));
    }

    json& operator[](int i) {
        if (is_null()) {
            type = value_t::array;
            value = json_value(value_t::array);
        }
        return (*value.array)[i];        
    }

    const json& operator[](int i) const {
        return (*value.array)[i];        
    }

    // Object Operations

    json& operator[](const char* k) {
        if (is_null()) {
            type = value_t::object;
            value = json_value(value_t::object);
        }

        for (auto& [name, value] : *value.object) {
            if (name == k) {
                return value;
            }
        }

        value.object->push_back({k, json()});
        return value.object->back().value;
    }

    json& operator[](const std::string& k) {
        if (is_null()) {
            type = value_t::object;
            value = json_value(value_t::object);
        }

        for (auto& [name, value] : *value.object) {
            if (name == k) {
                return value;
            }
        }

        value.object->push_back({k, json()});
        return value.object->back().value;
    }
    
    const json& operator[](const std::string& k) const {
        for (auto& [name, value] : *value.object) {
            if (name == k) {
                return value;
            }
        }

        value.object->push_back({k, json()});
        return value.object->back().value;
    }

    template<typename ValueT, typename ObjectItT, typename ArrayItT>
    struct basic_iterator {
        using value_type = ValueT;
        using reference_type = ValueT&;
        using pointer_type = ValueT*;

        value_t type;
        union {
            void* null;
            ObjectItT object_it;
            ArrayItT array_it;
        };
        basic_iterator() : type(value_t::null), null(nullptr) {}
        basic_iterator(reference_type o, ObjectItT it) : type(o.type), object_it(it) {}
        basic_iterator(reference_type o, ArrayItT it) : type(o.type), array_it(it) {}
    
        reference_type operator*() {
            switch (type) {
                case value_t::object: 
                    return object_it->value;
                case value_t::array: 
                    return *array_it;
                case value_t::string:
                case value_t::int_num:
                case value_t::uint_num:
                case value_t::float_num:
                case value_t::boolean:
                case value_t::null:
                    std::abort();
            }
        }
        
        reference_type operator->() {
            switch (type) {
                case value_t::object: 
                    return object_it->second;
                case value_t::array: 
                    return *array_it;
                case value_t::string:
                case value_t::int_num:
                case value_t::uint_num:
                case value_t::float_num:
                case value_t::boolean:
                case value_t::null:
                    std::abort();
            }
        }
        
        const string_t& name() {
            if (type == value_t::object) {
                return object_it->name;
            }
            std::abort();
        }

        const string_t& key() {
            if (type == value_t::object) {
                return object_it->name;
            }
            std::abort();
        }

        const string_t& first() {
            if (type == value_t::object) {
                return object_it->name;
            }
            std::abort();
        }
        
        reference_type value() {
            if (type == value_t::object) {
                return object_it->value;
            }
            std::abort();
        }
        
        reference_type second() {
            if (type == value_t::object) {
                return object_it->value;
            }
            std::abort();
        }

        basic_iterator& operator++() {
            switch (type) {
                case value_t::object: 
                    ++object_it;
                    break;
                case value_t::array: 
                    ++array_it;
                    break;
                case value_t::string:
                case value_t::int_num:
                case value_t::uint_num:
                case value_t::float_num:
                case value_t::boolean:
                case value_t::null:
                    std::abort();
            }
            return *this;
        }

        basic_iterator& operator++(int) {
            switch (type) {
                case value_t::object: 
                    object_it++; 
                    break;
                case value_t::array: 
                    array_it++;
                    break;
                case value_t::string:
                case value_t::int_num:
                case value_t::uint_num:
                case value_t::float_num:
                case value_t::boolean:
                case value_t::null:
                    std::abort();
            }
            return *this;
        }

        bool operator==(const basic_iterator& other) const {
            if (type == other.type) {
                switch (type) {
                    case value_t::object: 
                        return object_it == other.object_it;
                    case value_t::array: 
                        return array_it == other.array_it;
                    case value_t::string:
                    case value_t::int_num:
                    case value_t::uint_num:
                    case value_t::float_num:
                    case value_t::boolean:
                    case value_t::null:
                        return true;
                }
            }
            return false; 
        }

        bool operator!=(const basic_iterator& other) const {
            return !(*this == other);
        }
    };

    using iterator = basic_iterator<json, object_t::iterator, array_t::iterator>;
    using const_iterator = basic_iterator<const json, object_t::const_iterator, array_t::const_iterator>;

    iterator begin() { 
        switch (type) {
            case value_t::object: return iterator(*this, value.object->begin());
            case value_t::array: return iterator(*this, value.array->begin());
            default: return iterator();
        }
    }

    iterator end() { 
        switch (type) {
            case value_t::object: return iterator(*this, value.object->end());
            case value_t::array: return iterator(*this, value.array->end());
            default: return iterator();
        }
    }

    const_iterator begin() const { 
        return cbegin();
    }

    const_iterator end() const { 
        return cend();
    }

    const_iterator cbegin() const { 
        switch (type) {
            case value_t::object: return const_iterator(*this, value.object->cbegin());
            case value_t::array: return const_iterator(*this, value.array->cbegin());
            default: return const_iterator();
        }
    }

    const_iterator cend() const { 
        switch (type) {
            case value_t::object: return const_iterator(*this, value.object->cend());
            case value_t::array: return const_iterator(*this, value.array->cend());
            default: return const_iterator();
        }
    }

    struct items_proxy {
        object_t& o;

        object_t::iterator begin() { return o.begin(); }
        object_t::iterator end() { return o.end(); }
        object_t::const_iterator begin() const { return o.cbegin(); }
        object_t::const_iterator end() const { return o.cend(); }
        object_t::const_iterator cbegin() { return o.cbegin(); }
        object_t::const_iterator cend() { return o.cend(); }
    };

    items_proxy items() {
        if (type == value_t::object) {
            return {*value.object};
        }
        std::abort();
    }
};

// Parsing
struct string {
    const char* data;
    size_t size;
};

struct parsed_string {
    enum class type {
        string,
        error,
    };
    const char* end;
    type t;
    union {
        string s;
        const char* error;
    };

    parsed_string(const char* end, string s) : end(end), t(type::string), s(s) {}
    parsed_string(const char* end, const char* e) : end(end), t(type::error), error(e) {}
};

parsed_string parse_string(const char* c, const char* cend) {
    assert(*c == '"');
    c++;
    const char* data = c;

    // TODO: UTF-8 validation
    while (c != cend - 1) {
        if (c[0] == '\\') {
            switch (c[1]) {
                case '"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                    c += 2;
                    break;
                case 'u':
                    // TODO: 4 hex digits
                    return parsed_string(c, "\\u Hex digits are not supported\n");
            }
        } else if (c[0] == '"') {
            return parsed_string(c, string{data, static_cast<size_t>(c - data)});
        } else if (c[1] == '"') {
            c++;
            return parsed_string(c, string{data, static_cast<size_t>(c - data)});
        } else {
            c++;
        }
    }

    if (c[0] == '"') {
        return parsed_string(c, string{data, static_cast<size_t>(c - data)});
    }

    return parsed_string(c, "Unexpected end of string while parsing string");
}

enum class number_t {
    int_num,
    uint_num,
    real_num,    
    error,
};

struct parsed_number {
    const char* end;
    number_t type;
    union {
        int64_t i;
        uint64_t u;
        double d;
        const char* what;
    };

    parsed_number(const char* end, int64_t i) : end(end), type(number_t::int_num), i(i) {}
    parsed_number(const char* end, uint64_t u) : end(end), type(number_t::uint_num), u(u) {}
    parsed_number(const char* end, double d) : end(end), type(number_t::real_num), d(d) {}
    parsed_number(const char* end, const char* w) : end(end), type(number_t::error), what(w) {}
};

parsed_number parse_number(const char* str, const char* end) {
    enum class parse_phase {
        begin, // Allows '-' or any digit
        unsigned_digits, // 1-9 or '.' 
        signed_digits_1, // Follows leading '-'. Any digit but 0 promotes to real
        signed_digits_2, // any digit, '.', 'e', or 'E' promotes to real
        real_significand_1, // Can only be '0.'
        real_significand_2, // significand after '.'
        real_exponent_1, // exponent immediatley after 'e' or 'E'. '+' or '-' or any digit
        real_exponent_2, // any digit, 0s ignored
        real_exponent_3, // any digit, 0s not ignored
    };
 
    const char* c = str;
    const char* c_begin = str;
    const char* cend = end;

    const char* error = nullptr;
    int sign = 1;
    uint64_t u = 0;
    int exponent_sign = 1;
    int64_t implicit_exponent = 0;
    uint64_t explicit_exponent = 0;
    parse_phase phase = parse_phase::begin;

    for (;c != cend;++c) {
        switch(phase) {
            case parse_phase::begin:
                // std::cout << "begin\n"; 
                switch(*c) {
                    case '-':
                        sign = -1;
                        phase = parse_phase::signed_digits_1;
                        break;
                    case '0':
                        phase = parse_phase::real_significand_1;
                        break; 
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        phase = parse_phase::unsigned_digits;
                        u = u * 10 + (*c - '0');
                        break;
                    default:
                        return parsed_number(c, "Unexpected token when parsing number");
                }
                break;
            case parse_phase::unsigned_digits:
                // std::cout << "unsigned_digits\n"; 
                switch(*c) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        u = u * 10 + (*c - '0');
                        break;
                    case '.':
                        phase = parse_phase::real_significand_2;
                        break;
                    case 'e':
                    case 'E':
                        phase = parse_phase::real_exponent_1;
                    default:
                        // Return unsigned number
                        if (c - c_begin <= 19 || (c - c_begin == 20 && u >= 10000000000000000000ull)) {
                            return parsed_number(c, u);
                        } else {
                            return parsed_number(c, "Overflow while parsing unsigned int");
                        }
                }
                break;
            case parse_phase::signed_digits_1:
                // std::cout << "signed_digits_1\n"; 
                switch(*c) {
                    case '0':
                        phase = parse_phase::real_significand_1;
                        break; 
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        phase = parse_phase::signed_digits_2;
                        u = u * 10 + (*c - '0');
                        break;
                    default:
                        return parsed_number(c, "Expected digit after '-'");
                }
                break;
            case parse_phase::signed_digits_2:
                // std::cout << "signed_digits_2\n"; 
                switch(*c) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        u = u * 10 + (*c - '0');
                        break;
                    case '.':
                        phase = parse_phase::real_significand_2;
                        break;
                    case 'e':
                    case 'E':
                        phase = parse_phase::real_exponent_1;
                        break;
                    default:
                        if (u <= 9223372036854775808ull) {
                            return parsed_number(c, -1 * static_cast<int64_t>(u));
                        } else {
                            return parsed_number(c, "Overflow while parsing signed int");
                        }
                }
                break;
            case parse_phase::real_significand_1:
                // std::cout << "real_significand_1\n"; 
                switch(*c) {
                    case '.':
                        phase = parse_phase::real_significand_2;
                        break;
                    default:
                        return parsed_number(c, "Expected '.' while parsing real number");
                }
                break;
            case parse_phase::real_significand_2:
                // After decimal
                // std::cout << "real_significand_2\n"; 
                switch(*c) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        implicit_exponent -= 1;
                        u = u * 10 + (*c - '0');
                        break;
                    case 'e':
                    case 'E':
                        phase = parse_phase::real_exponent_1;
                        break;
                    default:
                        return parsed_number(c, "Expected 'e', 'E' or digit after '.'");
                }
                break;
            case parse_phase::real_exponent_1:
                // std::cout << "real_exponent_1\n"; 
                switch (*c) {
                    case '0':
                        // Ignore leading 0s
                        phase = parse_phase::real_exponent_2;
                        break;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        explicit_exponent = explicit_exponent * 10 + (*c - '0');
                        phase = parse_phase::real_exponent_3;
                        break;
                    case '-':
                        exponent_sign = -1;
                        phase = parse_phase::real_exponent_3;
                        break;
                    case '+':
                        exponent_sign = 1;
                        phase = parse_phase::real_exponent_3;
                        break;
                    default:
                        return parsed_number(c, "Expected '+', '-', or digit while parsing exponent");
                }
                break;
            case parse_phase::real_exponent_2:
                // std::cout << "real_exponent_2\n"; 
                switch (*c) {
                    case '0':
                        // Ignore leading 0s
                        break;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        explicit_exponent = explicit_exponent * 10 + (*c - '0');
                        phase = parse_phase::real_exponent_3;
                        break;
                    default: {
                        // TODO: https://r-libre.teluq.ca/2259/1/floatparsing-11.pdf
                        int64_t exponent = implicit_exponent + (explicit_exponent * exponent_sign); 
                        return parsed_number(c, static_cast<double>(u) * pow(10.0, static_cast<double>(exponent)) * sign);
                    }
                }
                break;
            case parse_phase::real_exponent_3:
                // std::cout << "real_exponent_3\n"; 
                switch (*c) {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        explicit_exponent = explicit_exponent * 10 + (*c - '0');
                        break;
                    default: {
                        // TODO: https://r-libre.teluq.ca/2259/1/floatparsing-11.pdf
                        int64_t exponent = implicit_exponent + (explicit_exponent * exponent_sign); 
                        return parsed_number(c, static_cast<double>(u) * pow(10.0, static_cast<double>(exponent)) * sign);
                    }
                }
                break;
        }
    }
    
    // We can only be here because we ran out of chars
    switch (phase) {
        case parse_phase::begin:
            return parsed_number(c, "Unexpected end of string while parsing number");
        case parse_phase::unsigned_digits:
            // Return unsigned number
            if (c - c_begin <= 19 || (c - c_begin == 20 && u >= 10000000000000000000ull)) {
                return parsed_number(c, u);
            } else {
                return parsed_number(c, "Overflow while parsing unsigned int");
            }
        case parse_phase::signed_digits_1:
            return parsed_number(c, "Expected digit after '-'");
        case parse_phase::signed_digits_2:
            if (u <= 9223372036854775808ull) {
                return parsed_number(c, -1 * static_cast<int64_t>(u));
            } else {
                return parsed_number(c, "Overflow while parsing signed int");
            }
        case parse_phase::real_significand_1:
            return parsed_number(c, 0.0);
        case parse_phase::real_significand_2:
            return parsed_number(c, "Expected 'e', 'E' or digit after '.'");
        case parse_phase::real_exponent_1:
            return parsed_number(c, "Expected '+', '-', or digit while parsing exponent");
        case parse_phase::real_exponent_2:
            // fallthrough
        case parse_phase::real_exponent_3: {
            // TODO: https://r-libre.teluq.ca/2259/1/floatparsing-11.pdf
            int64_t exponent = implicit_exponent + (explicit_exponent * exponent_sign);
            return parsed_number(c, static_cast<double>(u) * pow(10.0, static_cast<double>(exponent)) * sign);
        }
    }
}

const char* skip_whitespace(const char* c, const char* cend) {
    while (c != cend) {
        switch(*c) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                break;
            default:
                return c;
        }
        c++;
    }
    return c;
}

std::optional<json> parse(const std::string& s) {
    std::stack<json> stack;
    stack.push(json());

    const char* c = s.data();
    const char* end = c + s.size();

    while (c != end) {
        c = skip_whitespace(c, end);

        switch (*c) {
            case '{': // Begin Object
                break;
            case '[': // Begin Array
                break;
            case '"': // Begin String
                break;
            case 't': // begin true
                if (end - c < 4) {
                    std::cout << "Unexpected value \"" << std::string_view(c, end-c) << "\"\n";
                    return std::nullopt;
                } else if (c[1] == 'r' && c[2] == 'u' && c[3] == 'e') {
                    stack.top() = json(true); 
                    c += 4;
                } else {
                    std::cout << "Unexpected value \"" << std::string_view(c, 4) << "\"\n";
                    return std::nullopt;
                }
                break;
            case 'f': // Begin false
                if (end - c < 5) {
                    std::cout << "Unexpected value \"" << std::string_view(c, end - c) << "\"\n";
                    return std::nullopt;
                } else if (c[1] == 'a' && c[2] == 'l' && c[3] == 's' && c[4] == 'e') {
                    stack.top() = json(false); 
                    c += 5;
                } else {
                    std::cout << "Unexpected value \"" << std::string_view(c, 5) << "\"\n";
                    return std::nullopt;
                }
                break;
            case 'n': // begin null
                if (end - c < 4) {
                    std::cout << "Unexpected value \"" << std::string_view(c, end-c) << "\"\n";
                    return std::nullopt;
                } else if (c[1] == 'u' && c[2] == 'l' && c[3] == 'l') {
                    stack.top() = json(); 
                    c += 4;
                } else {
                    std::cout << "Unexpected value \"" << std::string_view(c, 4) << "\"\n";
                    return std::nullopt;
                }
                break;
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': { // begin number
                parsed_number n = parse_number(c, end);
                switch (n.type) {
                    case number_t::int_num:
                        stack.top() = json(n.i);
                        break;
                    case number_t::uint_num:
                        stack.top() = json(n.u);
                        break;
                    case number_t::real_num:
                        stack.top() = json(n.d);
                        break;
                    case number_t::error:
                        std::cout << "error: " << n.what << "\n";
                        return std::nullopt;
                }
                c = n.end;
                break;
            }
            default:
                std::cout << "invalid json\n";
                return std::nullopt;
        }
    }

    json j = stack.top();
    stack.pop();
    return j;
}

