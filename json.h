
#pragma once

#include <stdint.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <initializer_list>
#include <iostream>
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

using object_t = std::unordered_map<std::string, json>;
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
            for (auto& it : init) {
                value.object->insert(std::make_pair(*it[0].get<std::string>(), std::move(it[1])));
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
                    os << "\"" << b->first << "\": " << b->second << ", ";
                    ++b;
                    ++n;
                }
                os << "\"" << b->first << "\": " << b->second << "}";
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
        return (*value.object)[k];
    }
    
    const json& operator[](const char* k) const {
        return (*value.object)[k];
    }

    json& operator[](const std::string& k) {
        if (is_null()) {
            type = value_t::object;
            value = json_value(value_t::object);
        }
        return (*value.object)[k];
    }
    
    const json& operator[](const std::string& k) const {
        return (*value.object)[k];
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

        const string_t& key() {
            if (type == value_t::object) {
                return object_it->first;
            }
            std::abort();
        }

        const string_t& first() {
            if (type == value_t::object) {
                return object_it->first;
            }
            std::abort();
        }
        
        reference_type value() {
            if (type == value_t::object) {
                return object_it->second;
            }
            std::abort();
        }
        
        reference_type second() {
            if (type == value_t::object) {
                return object_it->second;
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

enum class number_t {
    int_num,
    uint_num,
    real_num,    
    error,
};

struct parsed_number {
    number_t type;
    union {
        int64_t i;
        uint64_t u;
        double d;
        const char* what;
    };
};

enum class number_parse_phase {
    begin, // Allows '-' or any digit
    unsigned_digits, // 1-9 or '.' 
    signed_digits_1,
    signed_digits_2,
    real_significand_1, // Can only be '0.'
    real_significand_2, // significand after '.'
    real_exponent_1, // expoent after 'e' or 'E'. '+' or '-' or any digit
    real_exponent_2, // expoent after 'e' or 'E'. any digit 
};

parsed_number parse_number(const std::string& s) {
    const char* c = s.c_str();
    const char* c_begin = c;
    const char* cend = s.c_str() + s.size();

    parsed_number ret { number_t::error, 0 };
    const char* error = nullptr;
    bool is_signed = false;
    bool is_real = false;
    bool done = false;
    uint64_t u = 0;
    bool exponent_is_signed = false;
    int64_t implicit_exponent = 0;
    uint64_t explicit_exponent = 0;
    number_parse_phase phase = number_parse_phase::begin;

    while (!done && c != cend) {
        switch(phase) {
            case number_parse_phase::begin:
                // std::cout << "begin\n"; 
                switch(*c) {
                    case '-':
                        is_signed = true; 
                        phase = number_parse_phase::signed_digits_1;
                        break;
                    case '0':
                        is_real = true;
                        phase = number_parse_phase::real_significand_1;
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
                        phase = number_parse_phase::unsigned_digits;
                        u = u * 10 + (*c - '0');
                        break;
                    default:
                        done = true;
                        error = "Unexpected token when parsing number";
                        break;
                }
                break;
            case number_parse_phase::unsigned_digits:
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
                        is_real = true;
                        phase = number_parse_phase::real_significand_2;
                        break;
                    case 'e':
                    case 'E':
                        is_real = true;
                        phase = number_parse_phase::real_exponent_1;
                    default:
                        done = true;
                        break;
                }
                break;
            case number_parse_phase::signed_digits_1:
                // std::cout << "signed_digits_1\n"; 
                switch(*c) {
                    case '0':
                        is_real = true;
                        phase = number_parse_phase::real_significand_1;
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
                        phase = number_parse_phase::signed_digits_2;
                        u = u * 10 + (*c - '0');
                        break;
                    case '.':
                        is_real = true; 
                        phase = number_parse_phase::real_significand_2;
                        break;
                    default:
                        done = true;
                        break;
                }
                break;
            case number_parse_phase::signed_digits_2:
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
                        is_real = true; 
                        phase = number_parse_phase::real_significand_2;
                        break;
                    case 'e':
                    case 'E':
                        is_real = true;
                        phase = number_parse_phase::real_exponent_1;
                        break;
                    default:
                        done = true;
                        break;
                }
                break;
            case number_parse_phase::real_significand_1:
                // std::cout << "real_significand_1\n"; 
                assert(is_real);
                switch(*c) {
                    case '.':
                        phase = number_parse_phase::real_significand_2;
                        break;
                    default:
                        error = "Expected '.' while parsing real number";
                        break;
                }
                break;
            case number_parse_phase::real_significand_2:
                // After decimal
                // std::cout << "real_significand_2\n"; 
                assert(is_real);
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
                        phase = number_parse_phase::real_exponent_1;
                        break;
                    default:
                        done = true;
                        error = "Expected 'e', 'E' or digit after '.'";
                        break;
                }
                break;
            case number_parse_phase::real_exponent_1:
                // std::cout << "real_exponent_1\n"; 
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
                        phase = number_parse_phase::real_exponent_2;
                        break;
                    case '-':
                        exponent_is_signed = true;
                        // fallthrough
                    case '+':
                        phase = number_parse_phase::real_exponent_2;
                        break;
                    default:
                        done = true;
                        error = "Expected '+', '-', or digit while parsing exponent";
                        break;
                }
                break;
            case number_parse_phase::real_exponent_2:
                // std::cout << "real_exponent_2\n"; 
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
                    default:
                        done = true;
                        break;
                }
                break;
            
        }
        c++;
    }

    if (is_real && is_signed && u == 0) {
        ret.type = number_t::real_num;
        ret.d = -0;
    } else if (!done) {
        ret.type = number_t::error;
        ret.what = "unexpected end of string while parsing number";
    } if (error) {
        ret.type = number_t::error; 
        ret.what = error;
    } else if (!is_signed && !is_real) {
        if (c - c_begin <= 19 || (c - c_begin == 20 && u >= 10000000000000000000ull)) {
            ret.type = number_t::uint_num;
            ret.u = u;
        } else {
            ret.type = number_t::error;
            ret.what = "Overflow while parsing unsigned int";
        }
    } else if (is_signed && !is_real) {
        if (u <= 9223372036854775808ull) {
            ret.type = number_t::int_num;
            ret.i = -1 * static_cast<int64_t>(u);
        } else {
            ret.type = number_t::error;
            ret.what = "Overflow while parsing signed int";
        }
    } else {
        // Real
        int64_t exponent = implicit_exponent + (explicit_exponent * (exponent_is_signed ? -1 : 1)); 
        ret.type = number_t::real_num;
        // TODO: https://r-libre.teluq.ca/2259/1/floatparsing-11.pdf
        ret.d = static_cast<double>(u) * pow(10.0, static_cast<double>(exponent)) * (is_signed ? -1 : 1);
    }
    return ret;
}

