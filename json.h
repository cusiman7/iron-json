
#pragma once

#include <stdint.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <initializer_list>
#include <iostream>

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
    
    template <>
    std::optional<bool> get<bool>() const {
        return is_boolean() ? std::optional<bool>(value.boolean) : std::nullopt;
    }
    
    template <>
    std::optional<int64_t> get<int64_t>() const {
        switch (type) {
            case value_t::int_num: 
                return value.int_num;
            case value_t::uint_num:
                return value.uint_num;
            case value_t::float_num:
                return value.float_num;
            default:
                return std::nullopt;
        }
    }
    
    template <>
    std::optional<uint64_t> get<uint64_t>() const {
        switch (type) {
            case value_t::uint_num: 
                return value.uint_num;
            case value_t::int_num:
                return value.int_num;
            case value_t::float_num:
                return value.float_num;
            default:
                return std::nullopt;
        }
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

    struct iterator {
        value_t type;
        union {
            void* null;
            object_t::iterator object_it;
            array_t::iterator array_it;
        };
        iterator() : type(value_t::null), null(nullptr) {}
        iterator(json& o, object_t::iterator it) : type(o.type), object_it(it) {}
        iterator(json& o, array_t::iterator it) : type(o.type), array_it(it) {}
    
        json& operator*() {
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
        
        json& operator->() {
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

        iterator& operator++() {
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

        iterator& operator++(int) {
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

        bool operator==(const iterator& other) const {
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

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

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
};
