
#pragma once

#include <stdint.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <initializer_list>
#include <iostream>
#include <stack>
#include <deque>
#include <string_view>
#include <cassert>
#include <cfloat>

namespace fe {
namespace {
template <typename T, typename U>
bool within_limits(U n) {
    return n >= std::numeric_limits<T>::min() && n <= std::numeric_limits<T>::max();
}

template <typename T, typename U>
bool under_max(U n) {
    static_assert(std::is_unsigned<U>::value);
    return n <= std::numeric_limits<T>::max();
}
} // namespace

template <typename E>
struct error {
    E e;
    error(const E& e) : e(e) {}
    error(E&& e) : e(std::move(e)) {}
};

template <typename T, typename E>
class result {
    union { T v_; E e_; };
    bool ok_ = true;
public:
    result() : ok_(true) {
        new(&v_) T();
    }
    result(const T& v) : ok_(true) {
        new (&v_) T(v);
    }
    result(T&& v) : ok_(true) {
        new(&v_) T(std::move(v));
    }
    template <typename U>
    result(const error<U>& e) : ok_(false) {
        new (&e_) E(e.e);
    }
    template <typename U>
    result(error<U>&& e) : ok_(false) {
        new(&e_) E(std::move(e.e));
    }
    ~result() {
        if (ok_) v_.~T();
        else e_.~E();
    }

    result(const result& rhs) : ok_(rhs.ok_) {
        if (ok_) new(&v_) T(rhs.v_);
        else new (&e_) E(rhs.e_);
    }
    result(result&& rhs) : ok_(rhs.ok_) {
        if (ok_) new(&v_) T(std::move(rhs.v_));
        else new (&e_) E(std::move(rhs.e_));
    }

    result& operator=(const result& rhs) {
        if (ok_) {
            if (rhs.ok_) v_ = rhs.v_;
            else {
                v_.~T();
                new (&e_) E(rhs.e_);
                ok_ = false;
            }
        } else {
            if (rhs.ok_) {
                e_.~E();
                new(&v_) T(rhs.v_); 
                ok_ = true;
            } else {
                e_ = rhs.e_;
            }
        }
        return *this;
    }
    result& operator=(result&& rhs) {
        if (ok_) {
            if (rhs.ok_) v_ = std::move(rhs.v_);
            else {
                v_.~T();
                new (&e_) E(std::move(rhs.e_));
                ok_ = false;
            }
        } else {
            if (rhs.ok_) {
                e_.~E();
                new(&v_) T(std::move(rhs.v_)); 
                ok_ = true;
            } else {
                e_ = std::move(rhs.e_);
            }
        }
        return *this;
    }

    friend bool operator==(const result& lhs, const result& rhs) {
        if (lhs.ok_ && rhs.ok_) return lhs.v_ == rhs.v_;
        else if (!lhs.ok_ && !rhs.ok_) return lhs.e_ == rhs.e_;
        else return false;
    }
    friend bool operator!=(const result& lhs, const result& rhs) {
        return !(lhs == rhs);
    }

    operator bool() const { return ok_; }
    bool is_ok() const { return ok_; }
    bool is_err() const { return !ok_; }

    T& value() & {
        if (ok_) return v_;
        std::abort();
    }

    const T& value() const& {
        if (ok_) return v_;
        std::abort();
    }

    T&& value() && {
        if (ok_) return std::move(v_);
        std::abort();
    }

    const T&& value() const&& {
        if (ok_) return std::move(v_);
        std::abort();
    }

    E& error() & {
        if (!ok_) return e_;
        std::abort();
    }
    
    const E& error() const& {
        if (!ok_) return e_;
        std::abort();
    }

    E&& error() && {
        if (!ok_) return std::move(e_);
        std::abort();
    }
    
    const E&& error() const&& {
        if (!ok_) return std::move(e_);
        std::abort();
    }
};

enum class json_error: uint8_t {
    invalid_type,
};

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
using object_t = std::vector<std::pair<std::string, json>>;
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
    } value;

    inline void destroy() {
        switch (type) {
            case value_t::object:
                delete value.object;
                break;
            case value_t::array:
                delete value.array;
                break;
            case value_t::string:
                delete value.string;
                break;
            default:
                break;
        }
    }
public:
    json() : type(value_t::null), value{.object = nullptr} {}
    json(value_t t) : type(t) {
        switch (type) {
            case value_t::object:
                value.object = new object_t();
                return;
            case value_t::array:
                value.array = new array_t();
                return;
            case value_t::string:
                value.string = new string_t();
                return;
            case value_t::int_num:
                value.int_num = 0;
                return;
            case value_t::uint_num:
                value.uint_num = 0;
                return;
            case value_t::float_num:
                value.float_num = 0;
                return;
            case value_t::boolean:
                value.boolean = false;
                return;
            case value_t::null:
                value.object = nullptr;
                return;
        }
    }
    json(nullptr_t null) : type(value_t::null), value{.object = nullptr} {}
    json(const char* str) : type(value_t::string), value{.string = new string_t(str)} {} 
    json(const std::string& str) : type(value_t::string), value{.string = new string_t(str)} {}
    json(std::string&& str) : type(value_t::string), value{.string = new string_t(std::move(str))} {}
    json(int num) : type(value_t::int_num), value{.int_num = num} {}
    json(int64_t num) : type(value_t::int_num), value{.int_num = num} {}
    json(uint64_t num) : type(value_t::uint_num), value{.uint_num = num} {}
    json(double num) : type(value_t::float_num), value{.float_num = num} {}
    json(bool b) : type(value_t::boolean), value{.boolean = b} {}
    json(const object_t& o) : type(value_t::object), value{.object = new object_t(o)} {}
    json(object_t&& o) : type(value_t::object), value{.object = new object_t(std::move(o))} {}
    json(const array_t& a) : type(value_t::array), value{.array = new array_t(a)} {}
    json(array_t&& a) : type(value_t::array), value{.array = new array_t(std::move(a))} {}

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
            value.object = new object_t();
            value.object->reserve(init.size());
            for (auto& it : init) {
                value.object->push_back({it[0].get<std::string>().value(), std::move(it[1])});
            }
        } else {
            type = value_t::array;
            value.array = new array_t(init.begin(), init.end());
        }
    }
    
    static json object() {
        return json(value_t::object);
    }

    static json object(const object_t& o) {
        return json(o);
    }
    
    static json object(object_t&& o) {
        return json(std::move(o));
    }
    
    template <typename ...Args>
    static json object(Args&& ...args) {
        return json(object_t{std::forward<Args>(args)...});
    }
    
    static json array() {
        return json(value_t::array);
    }

    static json array(const array_t& a) {
        return json(a);
    }
    
    static json array(array_t&& a) {
        return json(std::move(a));
    }
    
    template <typename ...Args>
    static json array(Args&& ...args) {
        return json(array_t{std::forward<Args>(args)...});
    }

    json(const json& other) : type(other.type) {
        switch (type) {
            case value_t::object:
                value.object = new object_t(*other.value.object);
                break;
            case value_t::array:
                value.array = new array_t(*other.value.array);
                break;
            case value_t::string:
                value.string = new string_t(*other.value.string);
                break;
            case value_t::int_num:
                value.int_num = other.value.int_num;
                break;
            case value_t::uint_num:
                value.uint_num = other.value.uint_num;
                break;
            case value_t::float_num:
                value.float_num = other.value.float_num;
                break;
            case value_t::boolean:
                value.boolean = other.value.boolean;
                break;
            case value_t::null:
                value.object = nullptr;
                break;
        }
    }

    json& operator=(const json& other) {
        if (this != &other) {
            destroy();
            type = other.type;
            switch (type) {
                case value_t::object:
                    value.object = new object_t(*other.value.object);
                    break;
                case value_t::array:
                    value.array = new array_t(*other.value.array);
                    break;
                case value_t::string:
                    value.string = new string_t(*other.value.string);
                    break;
                case value_t::int_num:
                    value.int_num = other.value.int_num;
                    break;
                case value_t::uint_num:
                    value.uint_num = other.value.uint_num;
                    break;
                case value_t::float_num:
                    value.float_num = other.value.float_num;
                    break;
                case value_t::boolean:
                    value.boolean = other.value.boolean;
                    break;
                case value_t::null:
                    value.object = nullptr;
                    break;
            }
        }
        return *this;
    }
    
    json(json&& other) : json() {
        swap(*this, other);
    }
    
    json& operator=(json&& other) {
        if (this != &other) {
            swap(*this, other);
        }
        return *this;
    }

    ~json() {
        destroy();
    }

    friend void swap(json& a, json& b) {
        std::swap(a.type, b.type);
        std::swap(a.value, b.value);
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
    result<T, json_error> get() const&;

    template <typename T>
    result<T, json_error> get() &&;

    template <>
    result<string_t, json_error> get<std::string>() const& {
        if (is_string()) {
            return *value.string;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<string_t, json_error> get<std::string>() && {
        if (is_string()) {
            return std::move(*value.string);
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<bool, json_error> get<bool>() const& {
        if (is_boolean()) {
            return bool(value.boolean);
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<bool, json_error> get<bool>() && {
        if (is_boolean()) {
            return bool(value.boolean);
        }
        return error(json_error::invalid_type);
    }

    template <>
    result<int8_t, json_error> get<int8_t>() const& {
        if (type == value_t::int_num && within_limits<int8_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int8_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<int8_t, json_error> get<int8_t>() && {
        if (type == value_t::int_num && within_limits<int8_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int8_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }

    template <>
    result<int16_t, json_error> get<int16_t>() const& {
        if (type == value_t::int_num && within_limits<int16_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int16_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }

    template <>
    result<int16_t, json_error> get<int16_t>() && {
        if (type == value_t::int_num && within_limits<int16_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int16_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }

    template <>
    result<int32_t, json_error> get<int32_t>() const& {
        if (type == value_t::int_num && within_limits<int32_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int32_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }

    template <>
    result<int32_t, json_error> get<int32_t>() && {
        if (type == value_t::int_num && within_limits<int32_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int32_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<int64_t, json_error> get<int64_t>() const& {
        if (type == value_t::int_num && within_limits<int64_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int64_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<int64_t, json_error> get<int64_t>() && {
        if (type == value_t::int_num && within_limits<int64_t>(value.int_num)) {
            return value.int_num;
        } else if (type == value_t::uint_num && under_max<int64_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint8_t, json_error> get<uint8_t>() const& {
        if (type == value_t::uint_num && within_limits<uint8_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint8_t, json_error> get<uint8_t>() && {
        if (type == value_t::uint_num && within_limits<uint8_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint16_t, json_error> get<uint16_t>() const& {
        if (type == value_t::uint_num && within_limits<uint16_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint16_t, json_error> get<uint16_t>() && {
        if (type == value_t::uint_num && within_limits<uint16_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint32_t, json_error> get<uint32_t>() const& {
        if (type == value_t::uint_num && within_limits<uint32_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint32_t, json_error> get<uint32_t>() && {
        if (type == value_t::uint_num && within_limits<uint32_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint64_t, json_error> get<uint64_t>() const& {
        if (type == value_t::uint_num && within_limits<uint64_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<uint64_t, json_error> get<uint64_t>() && {
        if (type == value_t::uint_num && within_limits<uint64_t>(value.uint_num)) {
            return value.uint_num;
        }
        return error(json_error::invalid_type);
    }
    
    template <>
    result<float, json_error> get<float>() const& {
        switch (type) {
            case value_t::float_num:
                return value.float_num;
            case value_t::int_num:
                return value.int_num;
            case value_t::uint_num: 
                return value.uint_num;
            default:
                return error(json_error::invalid_type);
        }
    }
    
    template <>
    result<float, json_error> get<float>() && {
        switch (type) {
            case value_t::float_num:
                return value.float_num;
            case value_t::int_num:
                return value.int_num;
            case value_t::uint_num: 
                return value.uint_num;
            default:
                return error(json_error::invalid_type);
        }
    }
    
    template <>
    result<double, json_error> get<double>() const& {
        switch (type) {
            case value_t::float_num:
                return value.float_num;
            case value_t::int_num:
                return value.int_num;
            case value_t::uint_num: 
                return value.uint_num;
            default:
                return error(json_error::invalid_type);
        }
    }
    
    template <>
    result<double, json_error> get<double>() && {
        switch (type) {
            case value_t::float_num:
                return value.float_num;
            case value_t::int_num:
                return value.int_num;
            case value_t::uint_num: 
                return value.uint_num;
            default:
                return error(json_error::invalid_type);
        }
    }

    static std::ostream& pretty_print(std::ostream& os, const json& j, size_t& indent) {
        switch (j.type) {
            case value_t::object: {
                if (j.value.object->empty()) {
                    os << "{}"; 
                    break;
                }
                os << "{\n";
                indent += 2;
                auto b = j.value.object->begin();
                auto n = b;
                n++;
                auto e = j.value.object->end();
                while (n != e) {
                    std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                    os << "\"" << b->first << "\": ";
                    pretty_print(os, b->second, indent) << ",\n";
                    ++b;
                    ++n;
                }
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                os << "\"" << b->first << "\": ";
                pretty_print(os, b->second, indent) << "\n";
                indent -= 2;
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                os << "}";
                break;
            }
            case value_t::array: {
                if (j.value.array->empty()) {
                    os << "[]";
                    break;
                }
                os << "[\n";
                indent += 2;
                auto b = j.value.array->begin();
                auto e = --j.value.array->end();
                while (b != e) {
                    std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                    pretty_print(os, *b++, indent) << ",\n";
                }
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                pretty_print(os, *b, indent) << "\n";
                indent -= 2;
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                os << "]";
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

    friend std::ostream& operator<<(std::ostream& os, const json& j) {
        size_t indent = 0;
        return pretty_print(os, j, indent);
    }

    // Array Operations
    
    void push_back(const json& j) {
        if (is_null()) {
            type = value_t::array;
            value.array = new array_t(1);
        }
        value.array->push_back(j);
    }

    void push_back(json&& j) {
        if (is_null()) {
            type = value_t::array;
            value.array = new array_t(1);
        }
        value.array->push_back(std::move(j));
    }

    json& operator[](int i) {
        if (is_null()) {
            type = value_t::array;
            value.array = new array_t(1);
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
            value.object = new object_t(1);
        }

        for (auto& [name, value] : *value.object) {
            if (name == k) {
                return value;
            }
        }

        value.object->push_back({k, json()});
        return value.object->back().second;
    }

    json& operator[](const std::string& k) {
        if (is_null()) {
            type = value_t::object;
            value.object = new object_t(1);
        }

        for (auto& [name, value] : *value.object) {
            if (name == k) {
                return value;
            }
        }

        value.object->push_back({k, json()});
        return value.object->back().second;
    }
    
    const json& operator[](const std::string& k) const {
        for (auto& [name, value] : *value.object) {
            if (name == k) {
                return value;
            }
        }

        value.object->push_back({k, json()});
        return value.object->back().second;
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
        
        const string_t& name() {
            if (type == value_t::object) {
                return object_it->first;
            }
            std::abort();
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

    static result<json, const char*> parse(const std::string& s) {
        const char* c = s.data();
        const char* cend = c + s.size();

        auto parse_value = [&]() -> result<json, const char*> {
            c = skip_whitespace(c, cend);
            if (c == cend) {
                return error<const char*>("Unexpected end of string while parsing value");
            }
            switch (*c) {
                case '{': // Begin object
                    c = skip_whitespace(c + 1, cend);
                    return json::object();
                case '[': // Begin array
                    c = skip_whitespace(c + 1, cend);
                    return json::array();
                case '"': { // Begin String
                    parsed_string ps = parse_string(c, cend);
                    switch (ps.t) {
                        case parsed_string::type::string:
                            assert(*c == '"');
                            c = skip_whitespace(ps.end + 1, cend);
                            return json(std::string(ps.s.data, ps.s.size));
                        case parsed_string::type::error:
                            c = ps.end; 
                            return error(ps.error);
                    }
                    break;
                }
                case 't': // begin true
                    if (cend - c < 4) {
                        c += cend - c;
                        return error<const char*>("Unexpected value"); 
                    } else if (c[1] == 'r' && c[2] == 'u' && c[3] == 'e') {
                        c = skip_whitespace(c + 4, cend);
                        return json(true);
                    } else {
                        c += 4;
                        return error<const char*>("Unexpected value"); 
                    }
                    break;
                case 'f': // Begin false
                    if (cend - c < 5) {
                        c += cend - c;
                        return error<const char*>("Unexpected value"); 
                    } else if (c[1] == 'a' && c[2] == 'l' && c[3] == 's' && c[4] == 'e') {
                        c = skip_whitespace(c + 5, cend);
                        return json(false);
                    } else {
                        c += 5;
                        return error<const char*>("Unexpected value"); 
                    }
                    break;
                case 'n': // begin null
                    if (cend - c < 4) {
                        c += cend - c;
                        return error<const char*>("Unexpected value"); 
                    } else if (c[1] == 'u' && c[2] == 'l' && c[3] == 'l') {
                        c = skip_whitespace(c + 4, cend);
                        return json();
                    } else {
                        c += 4;
                        return error<const char*>("Unexpected value"); 
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
                    parsed_number n = parse_number(c, cend);
                    switch (n.type) {
                        case number_t::int_num:
                            c = n.end;
                            c = skip_whitespace(c, cend);
                            return json(n.i);
                        case number_t::uint_num:
                            c = n.end;
                            c = skip_whitespace(c, cend);
                            return json(n.u);
                        case number_t::real_num:
                            c = n.end;
                            c = skip_whitespace(c, cend);
                            return json(n.d);
                        case number_t::error:
                            return error(n.what);
                    }
                    break;
                }
                default:
                    break;
            }
            return error<const char*>("Unexpected token");
        };

        // Structures holds in-progress structures (objects or array)
        // and a count of the number of elements parsed so far.
        std::stack<std::pair<json*, size_t>> structures;
        // Holds Name-Value pairs for objects being constructed
        std::deque<std::pair<std::string, json>> object_parts;
        // Holds values for arrays being constructed
        std::deque<json> array_parts;

        json root;
        // JSON docs can be single values all by themselves
        {
            result<json, const char*> value = parse_value();
            if (!value) {
                return value;
            } else if (!(value.value().is_object() || value.value().is_array())) {
                // Expect a single value document
                c = skip_whitespace(c, cend);
                if (c != cend) {
                    return error<const char*>("Unexpected character");
                }
                return value;
            }

            // Array or Object
            root = std::move(value).value();
            structures.emplace(&root, 0);
        }

        auto end_array_or_object = [&structures, &object_parts, &array_parts]() {
            assert(!structures.empty());
            assert(structures.top().first->is_array() || structures.top().first->is_object());
            if (structures.top().second == 0) {
                structures.pop();
                return;
            }
            auto [a_or_o, size] = structures.top();
            structures.pop();

            if (a_or_o->is_object()) {
                auto& obj_vec = *a_or_o->value.object;
                assert(obj_vec.empty());
                obj_vec.insert(obj_vec.end(),
                               std::make_move_iterator(object_parts.end() - size),
                               std::make_move_iterator(object_parts.end()));
                object_parts.erase(object_parts.end() - size, object_parts.end());
            } else {
                assert(a_or_o->is_array());
                auto& arr_vec = *a_or_o->value.array;
                assert(arr_vec.empty());
                arr_vec.insert(arr_vec.end(),
                               std::make_move_iterator(array_parts.end() - size),
                               std::make_move_iterator(array_parts.end()));
                array_parts.erase(array_parts.end() - size, array_parts.end());
            }
        };

        while (!structures.empty()) {
            // Parse Array
            // Arrays are lists of Values.
            // Commas "," separate Values.
            // [ value, value2, ... ]
            //  ^

            assert(structures.top().first->is_array() || structures.top().first->is_object());
            if (structures.top().first->is_array()) {
                if (structures.top().second > 0) {
                    c = skip_whitespace(c, cend);
                    // [ value, value2, ... ]
                    //                      ^
                    if (c != cend && *c == ']') {
                        c++;
                        end_array_or_object();
                        continue;
                    }

                    // [ value, value2, ... ]
                    //        ^
                    if (c == cend || *c != ',') {
                        return error<const char*>("Expected ','");
                    }
                    c++;
                }

                // [ value, value2, ... ]
                //   ^
                result<json, const char*> value = parse_value();
                if (!value) {
                    // [ value, value2, ... ]
                    //                      ^
                    if (c != cend && *c == ']') {
                        c++;
                        end_array_or_object();
                        continue;
                    }
                    return value;
                } else if (value.value().is_object() || value.value().is_array()) {
                    // Structues will be: |new_struct*, 0   | <- top
                    //                    |array*,      n+1 |
                    structures.top().second += 1;
                    array_parts.emplace_back(std::move(value).value());
                    structures.emplace(&array_parts.back(), 0);
                    continue;
                } else {
                    structures.top().second += 1;
                    array_parts.emplace_back(std::move(value).value());
                    continue;
                }
            } else {
                // Parse Object
                // Objects are unordered sets of Name-Value pairs.
                // Names must be Strings.
                // Colons ":" separate Names and Values.
                // Commas "," separate Name-Value pairs.
                // { "name": value, "name2": value2, ... }
                //  ^

                assert(structures.top().first->is_object());

                if (structures.top().second > 0) {
                    // { "name": value, "name2": value2, ... }
                    //                                       ^
                    c = skip_whitespace(c, cend);
                    if (c != cend && *c == '}') {
                        c++;
                        end_array_or_object();
                        continue;
                    }

                    // { "name": value, "name2": value2, ... }
                    //                ^
                    if (c == cend || *c != ',') {
                        return error<const char*>("Expected ','");
                    }
                    c++;
                }

                c = skip_whitespace(c, cend);
                if (c == cend) {
                    return error<const char*>("Unexpected end of string while parsing Key");
                }

                if (*c == '}') {
                    // { "name": value, "name2": value2, ... }
                    //                                       ^
                    c++;
                    end_array_or_object();
                    continue;
                } else if (*c != '"') {
                    return error<const char*>("Expected start of String for Key");
                } 

                // { "name": value, "name2": value2, ... }
                //   ^ 
                string_t key;
                parsed_string ps = parse_string(c, cend);
                switch (ps.t) {
                    case parsed_string::type::string:
                        c = ps.end + 1;
                        key = string_t(ps.s.data, ps.s.size);
                        break;
                    case parsed_string::type::error:
                        return error(ps.error);
                }

                // { "name": value, "name2": value2, ... }
                //         ^
                c = skip_whitespace(c, cend);
                if (c == cend || *c != ':') {
                    return error<const char*>("Expected ':'");
                }
                c++;
                c = skip_whitespace(c, cend);

                // { "name": value, "name2": value2, ... }
                //           ^
                result<json, const char*> value = parse_value();
                if (!value) {
                    return value;
                } else if (value.value().is_object() || value.value().is_array()) {
                    // Structues will be: |new_struct*, 0   | <- top
                    //                    |object*,     n+1 |
                    structures.top().second += 1;
                    object_parts.emplace_back(std::move(key), std::move(value).value());
                    structures.emplace(&object_parts.back().second, 0);
                    continue;
                } else {
                    // We parsed a Name and a non-object, non-array Value
                    structures.top().second += 1;
                    object_parts.emplace_back(std::move(key), std::move(value).value());
                    continue;
                }
            }
        }

        c = skip_whitespace(c, cend);
        if (c != cend) {
            return error<const char*>("Unexpected character");
        }

        return root;
    }

//private:
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

    static parsed_string parse_string(const char* c, const char* cend) {
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

    // https://github.com/simdjson/simdjson/blob/730939f01c7bd4aff103c6523697c3f70484a322/include/simdjson/generic/numberparsing.h#L55
    static bool compute_double(int64_t power, uint64_t i, double* d) {
    #ifndef FLT_EVAL_METHOD
    #error "FLT_EVAL_METHOD should be defined, please include cfloat."
    #endif
    #if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
        // We cannot be certain that x/y is rounded to nearest.
        if (0 <= power && power <= 22 && i <= 9007199254740991) {
    #else
        if (-22 <= power && power <= 22 && i <= 9007199254740991) {
    #endif
            // convert the integer into a double. This is lossless since
            // 0 <= i <= 2^53 - 1.
            *d = double(i);
            //
            // The general idea is as follows.
            // If 0 <= s < 2^53 and if 10^0 <= p <= 10^22 then
            // 1) Both s and p can be represented exactly as 64-bit floating-point
            // values
            // (binary64).
            // 2) Because s and p can be represented exactly as floating-point values,
            // then s * p
            // and s / p will produce correctly rounded values.
            //
            if (power < 0) {
                *d = *d / pow(10.0, -power);
            } else {
                *d = *d * pow(10.0, power);
            }
            return true;
        }
        return false;
    }

    static parsed_number parse_number(const char* str, const char* end) {
        enum class parse_phase {
            begin, // Allows '-' or any digit
            unsigned_digits, // 1-9 or '.' 
            signed_digits_1, // Follows leading '-'. Any digit but 0 promotes to real
            signed_digits_2, // any digit, '.', 'e', or 'E' promotes to real
            real_decimal, // Can only be '0.'
            real_significand_1, // First digit of significand after '.'. Ignores leading 0s
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
                            phase = parse_phase::real_decimal;
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
                            phase = parse_phase::real_significand_1;
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
                            phase = parse_phase::real_decimal;
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
                            phase = parse_phase::real_significand_1;
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
                case parse_phase::real_decimal:
                    // std::cout << "real_decimal\n"; 
                    switch(*c) {
                        case '.':
                            phase = parse_phase::real_significand_1;
                            break;
                        default:
                            return parsed_number(c, static_cast<int64_t>(0));
                    }
                    break;
                case parse_phase::real_significand_1:
                    switch(*c) {
                        case '0':
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
                            implicit_exponent -= 1;
                            u = u * 10 + (*c - '0');
                            phase = parse_phase::real_significand_2; 
                            break;
                        case 'e':
                        case 'E':
                            phase = parse_phase::real_exponent_1; 
                            break;
                        default:
                            int64_t exponent = implicit_exponent + (explicit_exponent * exponent_sign); 
                            double d;
                            if (compute_double(exponent, u, &d)) {
                                return parsed_number(c, d * sign);
                            } else {
                                char* e;
                                d = std::strtod(str, &e);
                                return parsed_number(e, d);
                            }
                    }
                    break;
                case parse_phase::real_significand_2:
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
                            //return parsed_number(c, "Expected 'e', 'E' or digit after '.'");
                            int64_t exponent = implicit_exponent + (explicit_exponent * exponent_sign); 
                            double d;
                            if (compute_double(exponent, u, &d)) {
                                return parsed_number(c, d * sign);
                            } else {
                                char* e;
                                d = std::strtod(str, &e);
                                return parsed_number(e, d);
                            }
                    }
                    break;
                case parse_phase::real_exponent_1:
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
                            double d;
                            if (compute_double(exponent, u, &d)) {
                                return parsed_number(c, d * sign);
                            } else {
                                char* e;
                                d = std::strtod(str, &e);
                                return parsed_number(e, d);
                            }
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
                            double d;
                            if (compute_double(exponent, u, &d)) {
                                return parsed_number(c, d * sign);
                            } else {
                                char* e;
                                d = std::strtod(str, &e);
                                return parsed_number(e, d);
                            }
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
            case parse_phase::real_decimal:
                return parsed_number(c, static_cast<int64_t>(0));
            case parse_phase::real_exponent_1:
                return parsed_number(c, "Expected digits after exponent signifier");
            case parse_phase::real_significand_1:
                // fallthrough
            case parse_phase::real_significand_2:
                // fallthrough
            case parse_phase::real_exponent_2:
                // fallthrough
            case parse_phase::real_exponent_3: {
                // TODO: https://r-libre.teluq.ca/2259/1/floatparsing-11.pdf
                int64_t exponent = implicit_exponent + (explicit_exponent * exponent_sign);
                double d;
                if (compute_double(exponent, u, &d)) {
                    return parsed_number(c, d * sign);
                } else {
                    char* e;
                    d = std::strtod(str, &e);
                    return parsed_number(e, d);
                }
            }
        }
    }

    static inline const char* skip_whitespace(const char* c, const char* cend) {
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
};

} // namespace fe

