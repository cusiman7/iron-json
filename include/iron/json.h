
#pragma once

#include <deque>
#include <initializer_list>
#include <iosfwd>
#include <limits>
#include <stack>
#include <string>
#include <vector>

#include <cassert>
#include <cfloat>
#include <cmath> // std::pow
#include <cstring>
#include <stdint.h>

namespace fe {
namespace {
template <typename T, typename U>
bool within_limits(U n) {
    return n >= std::numeric_limits<T>::min() && n <= std::numeric_limits<T>::max();
}

template <typename T, typename U>
bool under_max(U n) {
    static_assert(std::is_unsigned<U>::value, "Number must be unsigned to use under_max");
    return n <= static_cast<U>(std::numeric_limits<T>::max());
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

struct arena_allocator {
    arena_allocator() {
        head_ = alloc_block(4096, nullptr);
    }

    ~arena_allocator() {
        while (head_) {
            void* to_free = head_;
            head_ = head_->prev;
            ::free(to_free);
        }
    }
    
    void* alloc(size_t size) {
        return alloc(size, alignof(max_align_t));
    }

    void* alloc(size_t size, size_t alignment) {
        size_t alignment_offset = 0;
        size_t pointer_loc = reinterpret_cast<size_t>(head_->data) + head_->used;
        if (pointer_loc & (alignment - 1)) {
            alignment_offset = alignment - (pointer_loc & (alignment - 1)); 
        }

        if (head_->used + size + alignment_offset > head_->size) {
            size_t next_size = std::max(head_->size * 2, size + sizeof(block));
            head_ = alloc_block(next_size, head_);
            return alloc(size, alignment); 
        }
        
        size += alignment_offset;
        pointer_loc += alignment_offset;
        assert(size <= head_->size - head_->used);

        head_->used += size;
        return reinterpret_cast<void*>(pointer_loc);
    }

private:
    struct block {
        void* data;
        size_t size;
        size_t used;
        block* prev;
    };
    block* head_ = nullptr;

    block* alloc_block(size_t size, block* prev) {
        void* mem = malloc(size);
        block* b = static_cast<struct block*>(mem);
        b->data = mem;
        b->size = size;
        b->used = sizeof(struct block);
        b->prev = prev;
        return b;
    }
};

struct string {
    char* data;
    size_t size;

    friend bool operator==(const string& lhs, const char* rhs) {
        if (lhs.data && rhs) {
            size_t rhs_size = std::strlen(rhs);
            if (lhs.size != rhs_size) return false;
            return std::memcmp(lhs.data, rhs, lhs.size) == 0;
        }
        return !lhs.data && !rhs;
    }

    friend bool operator==(const char* lhs, const string& rhs) {
        return (rhs == lhs); 
    }

    friend bool operator!=(const string& lhs, const char* rhs) {
        return !(lhs == rhs);
    }
    
    friend bool operator!=(const char* rhs, const string& lhs) {
        return !(lhs == rhs);
    }
    
    friend bool operator==(const string& lhs, const std::string& rhs) {
        if (lhs.data && !rhs.empty() && lhs.size == rhs.size()) {
            return std::memcmp(lhs.data, rhs.data(), lhs.size) == 0;
        }
        return !lhs.data && rhs.empty();
    }
    
    friend bool operator==(const std::string& lhs, const string& rhs) {
        return (rhs == lhs); 
    }
    
    friend bool operator!=(const string& lhs, const std::string& rhs) {
        return !(lhs == rhs);
    }
    
    friend bool operator!=(const std::string& lhs, const string& rhs) {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const string& rhs);
};

enum class json_error: uint8_t {
    invalid_type,
};

enum class value_t: uint8_t {
    object,
    owned_object,
    array,
    owned_array,
    string,
    owned_string,
    int_num,
    uint_num,
    float_num,
    boolean,
    null,
};

class json;
using string_t = string;
using array_t = std::vector<json>;
using object_t = std::vector<std::pair<string_t, json>>;

class json {
    value_t type;
    bool owns_arena_ = false;
    union json_value {
        object_t* object;
        array_t* array;
        string_t string;
        int64_t int_num;
        uint64_t uint_num;
        double float_num;
        bool boolean;
    } value;
    arena_allocator* arena_ = nullptr;

    inline void destroy() {
        switch (type) {
            case value_t::object:
                (*value.object).~object_t();
                break;
            case value_t::owned_object:
                for (const auto& it : *value.object) {
                    free(it.first.data);
                }
                delete value.object;
                break;
            case value_t::array:
                (*value.array).~array_t();
                break;
            case value_t::owned_array:
                delete value.array;
                break;
            case value_t::owned_string:
                free(value.string.data);
                break;
            default:
                break;
        }
    }
    
    static string_t alloc_string(size_t size) {
        char* data = static_cast<char*>(malloc(size * sizeof(char)));
        return string_t{data, 0};
    }
    
    static string_t alloc_string(size_t size, arena_allocator* arena) {
        assert(arena);
        char* data = static_cast<char*>(arena->alloc(size * sizeof(char)));
        return string_t{data, 0};
    }

    static string_t alloc_string(const char* str) {
        size_t s = std::strlen(str);
        char* data = static_cast<char*>(malloc(s * sizeof(char)));
        memcpy(data, str, s);
        return string_t{data, s};
    }
    
    static string_t alloc_string(const char* str, arena_allocator* arena) {
        assert(arena);
        size_t s = std::strlen(str);
        char* data = static_cast<char*>(arena->alloc(s * sizeof(char)));
        memcpy(data, str, s);
        return string_t{data, s};
    }
    
    static string_t alloc_string(const char* str, size_t size) {
        char* data = static_cast<char*>(malloc(size * sizeof(char)));
        memcpy(data, str, size);
        return string_t{data, size};
    }

    static string_t alloc_string(const char* str, size_t size, arena_allocator* arena) {
        assert(arena);
        char* data = static_cast<char*>(arena->alloc(size * sizeof(char)));
        memcpy(data, str, size);
        return string_t{data, size};
    }

    static string_t alloc_string(const std::string& str) {
        size_t s = str.size();
        char* data = static_cast<char*>(malloc(s * sizeof(char)));
        memcpy(data, str.data(), s);
        return string_t{data, s};
    }

    static string_t alloc_string(const std::string& str, arena_allocator* arena) {
        assert(arena);
        size_t s = str.size();
        char* data = static_cast<char*>(arena->alloc(s * sizeof(char)));
        memcpy(data, str.data(), s);
        return string_t{data, s};
    }
  
    static string_t alloc_string(const string_t& str) {
        char* data = static_cast<char*>(malloc(str.size * sizeof(char)));
        memcpy(data, str.data, str.size);
        return string_t{data, str.size};
    }
  
    static string_t alloc_string(const string_t& str, arena_allocator* arena) {
        assert(arena);
        char* data = static_cast<char*>(arena->alloc(str.size * sizeof(char)));
        memcpy(data, str.data, str.size);
        return string_t{data, str.size};
    }
    
    static object_t* alloc_object(arena_allocator* arena) {
        return new(arena->alloc(sizeof(object_t))) object_t();
    }

    static object_t* alloc_object(const object_t& o, arena_allocator* arena) {
        return new(arena->alloc(sizeof(object_t))) object_t(o);
    }

    static object_t* alloc_object(object_t&& o, arena_allocator* arena) {
        return new(arena->alloc(sizeof(object_t))) object_t(std::move(o));
    }

public:
    json() : type(value_t::null) {
        value.object = nullptr;
    }
    json(nullptr_t) : json() {}
    json(arena_allocator* arena) : type(value_t::null), arena_(arena) {
         value.object = nullptr;
    }
    json(const char* str) : type(value_t::owned_string) {
        value.string = alloc_string(str); 
    }
    json(const char* str, arena_allocator* arena) : type(value_t::string), arena_(arena) {
        value.string = alloc_string(str, arena);
    }
    json(const std::string& str) : type(value_t::owned_string) {
        value.string = alloc_string(str);
    }
    json(const std::string& str, arena_allocator* arena) : type(value_t::string), arena_(arena) {
        value.string = alloc_string(str, arena);
    }
private:
    json(const string& str) : type(value_t::string) { value.string = str; }
public:
    json(int num) : type(value_t::int_num) { value.int_num = num; }
    json(int64_t num) : type(value_t::int_num) { value.int_num = num; }
    json(uint64_t num) : type(value_t::uint_num) { value.uint_num = num; }
    json(double num) : type(value_t::float_num) { value.float_num = num; }
    json(bool b) : type(value_t::boolean) { value.boolean = b; }
    json(const object_t& o) : type(value_t::owned_object) { value.object = new object_t(o); }
    json(object_t&& o) : type(value_t::owned_object) { value.object = new object_t(std::move(o)); }
    json(const object_t& o, arena_allocator* arena) : type(value_t::object), arena_(arena) {
        value.object = alloc_object(o, arena);
    }
    json(object_t&& o, arena_allocator* arena) : type(value_t::object), arena_(arena) {
        value.object = alloc_object(std::move(o), arena);
    }
    json(const array_t& a) : type(value_t::owned_array) {
        value.array = new array_t(a);
    }
    json(array_t&& a) : type(value_t::owned_array) {
        value.array = new array_t(std::move(a));
    }
    json(const array_t& a, arena_allocator* arena) : type(value_t::owned_array), arena_(arena) {
        value.array = new array_t(a);
    }
    json(array_t&& a, arena_allocator* arena) : type(value_t::owned_array), arena_(arena) {
        value.array = new array_t(std::move(a));
    }
public:

    json(std::initializer_list<json> init) : json() {
        bool looks_like_object = true;
        for (const auto& it : init) {
            looks_like_object = it.is_array() && it.size() == 2 && it[0].is_string();
            if (!looks_like_object) {
                break;
            }
        }
        if (looks_like_object) {
            type = value_t::owned_object;
            value.object = new object_t();
            value.object->reserve(init.size());
            for (auto& it : init) {
                string_t name = alloc_string(it[0].value.string);
                value.object->emplace_back(std::move(name), std::move(it[1]));
            }
        } else {
            type = value_t::owned_array;
            value.array = new array_t(init.begin(), init.end());
        }
    }

    static json object() {
        return json(object_t{});
    }

    static json object(const object_t& o) {
        return json(o);
    }

    static json object(object_t&& o) {
        return json(std::move(o));
    }

    static json object(arena_allocator* arena) {
        return json(object_t{}, arena);
    }

/*
    template <typename ...Args>
    static json object(Args&& ...args) {
        return json(object_t{std::forward<Args>(args)...});
    }
*/

    static json array() {
        return json(array_t{});
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

    static json doc() {
        json j;
        j.arena_ = new arena_allocator();
        j.owns_arena_ = true;
        return j;
    }

    json(const json& other) : type(other.type) {
        switch (type) {
            case value_t::object:
            case value_t::owned_object:
                type = value_t::owned_object;
                value.object = new object_t();
                value.object->reserve(other.value.object->size());
                for (const auto& other_it : *other.value.object) {
                    value.object->emplace_back(alloc_string(other_it.first), other_it.second);
                }
                break;
            case value_t::array:
            case value_t::owned_array:
                type = value_t::owned_array;
                value.array = new array_t(*other.value.array);
                break;
            case value_t::string:
            case value_t::owned_string:
                if (arena_) {
                    type = value_t::string;
                    value.string = alloc_string(other.value.string, arena_);
                } else {
                    type = value_t::owned_string;
                    value.string = alloc_string(other.value.string);
                }
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
                case value_t::owned_object:
                    if (arena_) {
                        type = value_t::object;
                        value.object = alloc_object(arena_);
                        value.object->reserve(other.value.object->size());
                        for (const auto& other_it : *other.value.object) {
                            value.object->emplace_back(alloc_string(other_it.first, arena_), other_it.second);
                        }
                    } else {
                        type = value_t::owned_object;
                        value.object = new object_t();
                        value.object->reserve(other.value.object->size());
                        for (const auto& other_it : *other.value.object) {
                            value.object->emplace_back(alloc_string(other_it.first), other_it.second);
                        }
                    }
                    break;
                case value_t::array:
                case value_t::owned_array:
                    type = value_t::owned_array;
                    value.array = new array_t(*other.value.array);
                    break;
                case value_t::string:
                case value_t::owned_string:
                    if (arena_) {
                        type = value_t::string;
                        value.string = alloc_string(other.value.string, arena_);
                    } else {
                        type = value_t::owned_string;
                        value.string = alloc_string(other.value.string);
                    }
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
            if (arena_ != other.arena_ || owns_arena_) {
                if (!arena_) {
                    arena_ = other.arena_;
                }
                type = other.type;
                switch (type) {
                    case value_t::object:
                    case value_t::owned_object:
                        if (arena_) {
                            type = value_t::object;
                            value.object = alloc_object(arena_);
                            value.object->reserve(other.value.object->size());
                            for (const auto& other_it : *other.value.object) {
                                value.object->emplace_back(alloc_string(other_it.first, arena_), other_it.second);
                            }
                        } else {
                            type = value_t::owned_object;
                            value.object = new object_t();
                            value.object->reserve(other.value.object->size());
                            for (const auto& other_it : *other.value.object) {
                                value.object->emplace_back(alloc_string(other_it.first), other_it.second);
                            }
                        }
                        break;
                    case value_t::array:
                    case value_t::owned_array:
                        type = value_t::owned_array;
                        value.array = new array_t(*other.value.array);
                        break;
                    case value_t::string:
                    case value_t::owned_string:
                        if (arena_) {
                            type = value_t::string;
                            value.string = alloc_string(other.value.string, arena_);
                        } else {
                            type = value_t::owned_string;
                            value.string = alloc_string(other.value.string);
                        }
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
            } else {
                swap(*this, other);
            }
        }
        return *this;
    }
    
    friend void swap(json& a, json& b) {
        std::swap(a.type, b.type);
        std::swap(a.value, b.value);
        std::swap(a.arena_, b.arena_);
        std::swap(a.owns_arena_, b.owns_arena_);
    }

    json& operator=(nullptr_t n) {
        destroy();
        type = value_t::null;
        value.object = n;
        return *this;
    }

    json& operator=(const char* str) {
        destroy();
        if (arena_) {
            type = value_t::string;
            value.string = alloc_string(str, arena_);
        } else {
            type = value_t::owned_string;
            value.string = alloc_string(str);
        }
        return *this;
    }

    ~json() {
        destroy();
        if (owns_arena_) {
            delete arena_;
        }
    }

    arena_allocator* arena() const {
        return arena_;
    }

    inline size_t empty() const {
        if (is_object()) return value.object->empty();
        if (is_array()) return value.array->empty();
        if (is_string()) return value.string.size == 0;
        std::abort();
    }

    inline size_t size() const {
        if (is_object()) return value.object->size();
        if (is_array()) return value.array->size();
        if (is_string()) return value.string.size;
        std::abort();
    }

    inline bool is_object() const { return type == value_t::object || type == value_t::owned_object; }
    inline bool is_array() const { return type == value_t::array || type == value_t::owned_array; }
    inline bool is_string() const { return type == value_t::string || type == value_t::owned_string; }
    inline bool is_number() const { return type == value_t::int_num || type == value_t::uint_num || type == value_t::float_num; }
    inline bool is_int() const { return type == value_t::int_num; }
    inline bool is_uint() const { return type == value_t::uint_num; }
    inline bool is_double() const { return type == value_t::float_num; }
    inline bool is_boolean() const { return type == value_t::boolean; }
    inline bool is_null() const { return type == value_t::null; }

    template <typename T>
    result<T, json_error> get() const;

    std::string dump() const;
    static std::ostream& print(std::ostream& os, const json& j);
    static std::ostream& pretty_print(std::ostream& os, const json& j, size_t& indent);
    friend std::ostream& operator<<(std::ostream& os, const json& j);

    // Array Operations

    void push_back(const json& j) {
        if (is_null()) {
            type = value_t::owned_array;
            value.array = new array_t();
        }
        value.array->push_back(j);
    }

    void push_back(json&& j) {
        if (is_null()) {
            type = value_t::owned_array;
            value.array = new array_t();
        }
        value.array->push_back(std::move(j));
    }

    json& operator[](int i) {
        if (is_null()) {
            type = value_t::owned_array;
            value.array = new array_t();
        }
        return (*value.array)[i];
    }

    const json& operator[](int i) const {
        return (*value.array)[i];
    }

    // Object Operations
    
    void become_object() {
        assert(is_null());
        if (arena_) {
            type = value_t::object;
            value.object = alloc_object(arena_);
        } else {
            type = value_t::owned_object;
            value.object = new object_t();
        }
    }
    
    json& operator[](const char* k) {
        if (is_null()) {
            become_object();
        }

        for (auto& it : *value.object) {
            if (it.first == k) {
                return it.second;
            }
        }

        // owned_objects own the names in the object array
        if (type == value_t::object) {
            assert(arena_);
            string_t name = alloc_string(k, arena_);
            value.object->emplace_back(std::move(name), json(arena_));
        } else {
            assert(type == value_t::owned_object);
            string_t name = alloc_string(k);
            value.object->emplace_back(std::move(name), json(arena_));
        }
        return value.object->back().second;
    }

    json& operator[](const std::string& k) {
        if (is_null()) {
            become_object();
        }

        for (auto& it : *value.object) {
            if (it.first == k) {
                return it.second;
            }
        }

        // owned_objects own the names in the object array
        if (type == value_t::object) {
            assert(arena_);
            string_t name = alloc_string(k, arena_);
            value.object->emplace_back(std::move(name), json(arena_));
        } else {
            assert(type == value_t::owned_object);
            string_t name = alloc_string(k);
            value.object->emplace_back(std::move(name), json(arena_));
        }
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
                case value_t::owned_object:
                    return object_it->second;
                case value_t::array:
                case value_t::owned_array:
                    return *array_it;
                default:
                    std::abort();
            }
        }

        reference_type operator->() {
            switch (type) {
                case value_t::object:
                case value_t::owned_object:
                    return object_it->second;
                case value_t::array:
                case value_t::owned_array:
                    return *array_it;
                default:
                    std::abort();
            }
        }

        const string_t& name() {
            if (type == value_t::object || type == value_t::owned_object) {
                return object_it->first;
            }
            std::abort();
        }

        const string_t& key() {
            if (type == value_t::object || type == value_t::owned_object) {
                return object_it->first;
            }
            std::abort();
        }

        const string_t& first() {
            if (type == value_t::object || type == value_t::owned_object) {
                return object_it->first;
            }
            std::abort();
        }

        reference_type value() {
            if (type == value_t::object || type == value_t::owned_object) {
                return object_it->second;
            }
            std::abort();
        }

        reference_type second() {
            if (type == value_t::object || type == value_t::owned_object) {
                return object_it->second;
            }
            std::abort();
        }

        basic_iterator& operator++() {
            switch (type) {
                case value_t::object:
                case value_t::owned_object:
                    ++object_it;
                    break;
                case value_t::array:
                case value_t::owned_array:
                    ++array_it;
                    break;
                default:
                    std::abort();
            }
            return *this;
        }

        basic_iterator& operator++(int) {
            switch (type) {
                case value_t::object:
                case value_t::owned_object:
                    object_it++;
                    break;
                case value_t::array:
                case value_t::owned_array:
                    array_it++;
                    break;
                default:
                    std::abort();
            }
            return *this;
        }

        bool operator==(const basic_iterator& other) const {
            if (type == other.type) {
                switch (type) {
                    case value_t::object:
                    case value_t::owned_object:
                        return object_it == other.object_it;
                    case value_t::array:
                    case value_t::owned_array:
                        return array_it == other.array_it;
                    default:
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
        if (is_object()) return iterator(*this, value.object->begin());
        else if (is_array()) return iterator(*this, value.array->begin());
        else return iterator();
    }

    iterator end() {
        if (is_object()) return iterator(*this, value.object->end());
        else if (is_array()) return iterator(*this, value.array->end());
        else return iterator();
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        if (is_object()) return const_iterator(*this, value.object->cbegin());
        else if (is_array()) return const_iterator(*this, value.array->cbegin());
        else return const_iterator();
    }

    const_iterator cend() const {
        if (is_object()) return const_iterator(*this, value.object->cend());
        else if (is_array()) return const_iterator(*this, value.array->cend());
        else return const_iterator();
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
        if (is_object()) {
            return {*value.object};
        }
        std::abort();
    }

    static result<json, const char*> parse(const std::string& s) {
        json root = json::doc();
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
                    return json::object(root.arena());
                case '[': // Begin array
                    c = skip_whitespace(c + 1, cend);
                    return json::array();
                case '"': { // Begin String
                    auto ps = parse_string(&c, cend, root.arena());
                    if (!ps) {
                        return error<const char*>(ps.error());
                    }
                    assert(*c == '"');
                    c = skip_whitespace(c + 1, cend);
                    return json(std::move(ps).value());
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
                            return error<const char*>(n.what);
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
        std::deque<std::pair<string_t, json>> object_parts;
        // Holds values for arrays being constructed
        std::deque<json> array_parts;

        // JSON docs can be single values all by themselves
        {
            result<json, const char*> value = parse_value();
            if (!value) {
                return value;
            } else if (!(value.value().is_object() || value.value().is_array())) {
                // Expect a single value document
                c = skip_whitespace(c, cend);
                if (c != cend) {
                    return error<const char*>("Unexpected character:");
                }
                return value;
            }

            // Array or Object
            root = std::move(value).value();
            assert(root.arena());
            structures.emplace(&root, 0);
        }

        auto end_array_or_object = [&structures, &object_parts, &array_parts]() {
            assert(!structures.empty());
            assert(structures.top().first->is_array() || structures.top().first->is_object());
            if (structures.top().second == 0) {
                structures.pop();
                return;
            }
            auto a_or_o_size = structures.top();
            structures.pop();
           
            json* a_or_o = a_or_o_size.first;
            size_t size = a_or_o_size.second;

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
                    return error<const char*>(value.error());
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
                auto ps = parse_string(&c, cend, root.arena());
                if (!ps) {
                    return error<const char*>(ps.error());
                }
                assert(*c == '"');
                c++;
                key = std::move(ps).value();

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
                    return error<const char*>(value.error());
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
    using parsed_string = result<string_t, const char*>;

    // Only called by parse_string when escape characters are found
    static parsed_string parse_string_slow(const char* str_start, const char* str_end, arena_allocator* arena) {
        // Reserve enough space for the output.
        // At worst this is 6x larger than it needs to be because the entire string could be
        // 6 char length hex codes which map to 1 byte utf-8 codepoints (\u0041 == 'A')
        size_t size = (str_end - str_start) * sizeof(char);
        string_t ret = alloc_string(size, arena);

        auto append = [&ret](const char* str, size_t count) {
            memcpy(ret.data + ret.size, str, count);
            ret.size += count;
        };

        auto append_char = [&ret](char c) {
            ret.data[ret.size] = c;
            ret.size++;
        };

        const char* start = str_start;
        const char* curr = str_start;

        auto parse_utf16_unit = [&curr]() -> result<uint16_t, const char*> {
            uint16_t codeunit = 0;
            for (int16_t i = 12; i >= 0; i -= 4) {
                curr++;
                if (*curr >= '0' && *curr <= '9') {
                    codeunit |= static_cast<uint16_t>(*curr - '0') << i;
                } else if (*curr >= 'A' && *curr <= 'F') {
                    codeunit |= static_cast<uint16_t>((*curr - 'A') + 10) << i;
                } else if (*curr >= 'a' && *curr <= 'f') {
                    codeunit |= static_cast<uint16_t>((*curr - 'a') + 10) << i;
                } else {
                    return error<const char*>("Invalid UTF-16 codeunit");
                }
            }
            return codeunit;
        };

        while (curr != str_end) {
            if (*curr == '\\') {
                append(start, curr - start);
                curr++;
                switch (*curr) {
                    case '"':
                    case '\\':
                    case '/':
                        append_char(*curr);
                        break;
                    case 'b':
                        append_char('\b');
                        break;
                    case 'f':
                        append_char('\f');
                        break;
                    case 'n':
                        append_char('\n');
                        break;
                    case 'r':
                        append_char('\r');
                        break;
                    case 't':
                        append_char('\t');
                        break;
                    case 'u': {
                        if (curr + 4 >= str_end) {
                            return error<const char*>("Invalid UTF-16 codeuint");
                        }
                        auto maybe_codeunit = parse_utf16_unit();
                        if (!maybe_codeunit) {
                            // We tried to parse \uXXXX but failed
                            return error<const char*>(maybe_codeunit.error());
                        }
                        uint16_t codeunit = maybe_codeunit.value();
                        if (codeunit <= 0x7F) {
                            // 0xxxxxxx
                            append_char(static_cast<char>(codeunit));
                        } else if (codeunit >= 0x80 && codeunit <= 0x7FF) {
                            // 110xxxxx	10xxxxxx
                            char bytes[2];
                            bytes[0] = 0xC0 | (codeunit >> 6);
                            bytes[1] = 0x80 | (codeunit & 0x3F);
                            append(bytes, 2);
                        } else if ((codeunit >= 0x800 && codeunit <= 0xD7FF) || (codeunit >= 0xE000)) {
                            // 1110xxxx	10xxxxxx 10xxxxxx
                            char bytes[3];
                            bytes[0] = 0xE0 | (codeunit >> 12);
                            bytes[1] = 0x80 | ((codeunit >> 6) & 0x3F);
                            bytes[2] = 0x80 | (codeunit & 0x3F);
                            append(bytes, 3);
                        } else if (codeunit >= 0xD800 && codeunit <= 0xDFFF) {
                            // We just parsed part one of 2 surrogates.
                            // If this surrogate is alone, replace it with the replacement char ???
                            if (curr + 5 >= str_end || curr[1] != '\\' || curr[2] != 'u') {
                                append(u8"???", 3);
                                break;
                            }  
                            curr += 2;
                            auto maybe_surrogate = parse_utf16_unit();
                            if (!maybe_surrogate) {
                                // We tried to parse \uXXXX but failed
                                return error<const char*>(maybe_surrogate.error());
                            }
                            uint16_t codeunit_2 = maybe_surrogate.value();
                            uint32_t codepoint = static_cast<uint32_t>(((codeunit - 0xD800) * 0x400) + (codeunit_2 - 0xDC00) + 0x10000);
                            if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
                                append(u8"???", 3);
                                break;
                            }
                            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                            char bytes[4];
                            bytes[0] = 0xF0 | (codepoint >> 18);
                            bytes[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                            bytes[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                            bytes[3] = 0x80 | (codepoint & 0x3F);
                            append(bytes, 4);
                        }
                        break;
                    }
                    default:
                        return error<const char*>("Invalid escape sequence while parsing string");
                }
                curr++;
                start = curr;
                continue;
            }
            // The string has already been UTF-8 validated by parse_string
            unsigned char byte_0 = *curr;
            if (byte_0 <= 0x7F) curr++;
            else if ((byte_0 & 0xE0) == 0xC0) curr += 2;
            else if ((byte_0 & 0xF0) == 0xE0) curr += 3;
            else if ((byte_0 & 0xF8) == 0xF0) curr += 4;
        }
        append(start, curr - start);
        return ret;
    }

    /*
     * We parse strings in a single pass in the common case and two passes worst-case.
     * The first pass validates the string is UTF-8 and identifies the end of the string.
     * If the string has no escape characters '\' the string is memcopied and returned.
     * If any escape characters were identified during validation and length checking
     * a second pass is performed to decode the string.
     */
    static parsed_string parse_string(const char** c, const char* cend, arena_allocator* arena) {
        assert(**c == '"');
        assert(arena);
        (*c)++;
        const char* str_start = *c;
        bool take_slow_path = false;

        while (true) {
            while (*c != cend && **c != '"') {
                take_slow_path |= (**c == '\\');

                // Naive UTF-8 validation
                unsigned char byte_0 = **c;
                int n = 0;
                if (byte_0 <= 0x7F) n = 0;
                else if ((byte_0 & 0xE0) == 0xC0) n = 1;
                else if ((byte_0 & 0xF0) == 0xE0) n = 2;
                else if ((byte_0 & 0xF8) == 0xF0) n = 3;
                else return error<const char*>("Invalid UTF-8 codepoint");

                (*c)++;
                for (int i = 0; i < n; i++) {
                    if ((*c + i) == cend || (((unsigned char)*(*c+i)) & 0xC0) != 0x80) return error<const char*>("Invalid UTF-8 codepoint");
                }
                (*c) += n;
            }
            if (*c == cend) {
                return error<const char*>("Unexpected end of string when parsing string");
            }

            if (!take_slow_path) {
                assert(**c == '"');
                size_t size = static_cast<size_t>(*c - str_start);
                return alloc_string(str_start, size, arena);
            }

            // We may not be at the end of the string yet as it may simply have been escaped
            int escaped = 0;
            const char* peek_back = (*c - 1);
            while (peek_back != str_start - 1 && *peek_back == '\\') {
                escaped ^= 1;
                peek_back--;
            }

            if (!escaped) {
                assert(**c == '"');
                return parse_string_slow(str_start, *c, arena);
            }
            // Found an escaped quote, continue looking for end of string
            (*c)++;
        }
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
                *d = *d / std::pow(10.0, -power);
            } else {
                *d = *d * std::pow(10.0, power);
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
                            break;
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
        return parsed_number(c, "parse_number fell through to end of function");
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
}; // class json

template <>
inline result<std::string, json_error> json::get<std::string>() const {
    if (is_string()) {
        return std::string(value.string.data, value.string.size);
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<bool, json_error> json::get<bool>() const {
    if (is_boolean()) {
        return bool(value.boolean);
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<int8_t, json_error> json::get<int8_t>() const {
    if (type == value_t::int_num && within_limits<int8_t>(value.int_num)) {
        return value.int_num;
    } else if (type == value_t::uint_num && under_max<int8_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<int16_t, json_error> json::get<int16_t>() const {
    if (type == value_t::int_num && within_limits<int16_t>(value.int_num)) {
        return value.int_num;
    } else if (type == value_t::uint_num && under_max<int16_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<int32_t, json_error> json::get<int32_t>() const {
    if (type == value_t::int_num && within_limits<int32_t>(value.int_num)) {
        return value.int_num;
    } else if (type == value_t::uint_num && under_max<int32_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<int64_t, json_error> json::get<int64_t>() const {
    if (type == value_t::int_num && within_limits<int64_t>(value.int_num)) {
        return value.int_num;
    } else if (type == value_t::uint_num && under_max<int64_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<uint8_t, json_error> json::get<uint8_t>() const {
    if (type == value_t::uint_num && within_limits<uint8_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<uint16_t, json_error> json::get<uint16_t>() const {
    if (type == value_t::uint_num && within_limits<uint16_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<uint32_t, json_error> json::get<uint32_t>() const {
    if (type == value_t::uint_num && within_limits<uint32_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<uint64_t, json_error> json::get<uint64_t>() const {
    if (type == value_t::uint_num && within_limits<uint64_t>(value.uint_num)) {
        return value.uint_num;
    }
    return error<json_error>(json_error::invalid_type);
}

template <>
inline result<float, json_error> json::get<float>() const {
    switch (type) {
        case value_t::float_num:
            return value.float_num;
        case value_t::int_num:
            return value.int_num;
        case value_t::uint_num:
            return value.uint_num;
        default:
            return error<json_error>(json_error::invalid_type);
    }
}

template <>
inline result<double, json_error> json::get<double>() const {
    switch (type) {
        case value_t::float_num:
            return value.float_num;
        case value_t::int_num:
            return value.int_num;
        case value_t::uint_num:
            return value.uint_num;
        default:
            return error<json_error>(json_error::invalid_type);
    }
}

} // namespace fe

