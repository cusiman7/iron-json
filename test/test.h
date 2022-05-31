
#pragma once

#include <type_traits>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace test {

struct test_error : public std::exception {};

using test_fn = void(*)();
void register_test(const char* name, test_fn f, int line, const char* file);
void set_check_vars(std::string&& a, std::string&& b, const char* comp);
void on_check_failed(const char* expr, const char* file, int line);
void on_require_failed(const char* expr, const char* file, int line);

template <typename T>
std::string to_string(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template <typename T>
std::string to_string(const std::optional<T>& t) {
    return t ? to_string(t.value()) : "std::nullopt";
}

inline std::string to_string(const bool& b) {
    return (b ? "true" : "false");
}

inline std::string to_string(const double& d) {
    std::stringstream ss;
    ss.precision(std::numeric_limits<double>::max_digits10);
    ss << d;
    return ss.str();
}

inline std::string to_string(const float& f) {
    std::stringstream ss;
    ss.precision(std::numeric_limits<float>::max_digits10);
    ss << f;
    return ss.str();
}

class Test {
public:
    Test(const char* name, test_fn f, int line, const char* file); 
    void operator()() const;
    const char* name() const;

private:
    const char* name_;
    test_fn f_;
    int line_;
    const char* file_;
};

template <class A>
struct Comp {
    const A& a;

    operator bool() const {
        using test::to_string;
        if (a) return true;
        set_check_vars(to_string(a), "", "");
        return false;
    }
    
    template <class B>
    bool operator==(B&& b) const {
        using test::to_string;
        if (a == b) return true;
        set_check_vars(to_string(a), to_string(std::forward<B>(b)), "==");
        return false;
    }
    
    template <class B>
    bool operator!=(B&& b) const {
        using test::to_string;
        if (a != b) return true;
        set_check_vars(to_string(a), to_string(b), "!=");
        return false;
    }
    
    template <class B>
    bool operator<=(B&& b) const {
        using test::to_string;
        if (a <= b) return true;
        set_check_vars(to_string(a), to_string(b), "<=");
        return false;
    }
    
    template <class B>
    bool operator>=(B&& b) const {
        using test::to_string;
        if (a >= b) return true;
        set_check_vars(to_string(a), to_string(b), ">=");
        return false;
    }

    template <class B>
    bool operator<(B&& b) const {
        using test::to_string;
        if (a < b) return true;
        set_check_vars(to_string(a), to_string(std::forward<B>(b)), "<");
        return false;
    }
    
    template <class B>
    bool operator>(B&& b) const {
        using test::to_string;
        if (a > b) return true;
        set_check_vars(to_string(a), to_string(std::forward<B>(b)), ">");
        return false;
    }
};

struct Check {
    template <class A>
    Comp<A> operator<(A&& a) {
        return Comp<A>{std::forward<A>(a)};
    }
};

}

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define TEST(Name)                                                              \
    static_assert(true, Name " must be string literal");                        \
    namespace detail {                                                          \
        static void CONCAT(_registered_test_, __LINE__)();                      \
                                                                                \
        namespace {                                                             \
            struct CONCAT(_register_struct_, __LINE__) {                        \
                CONCAT(_register_struct_, __LINE__)() {                         \
                    test::register_test(                                        \
                        Name,                                                   \
                        CONCAT(_registered_test_, __LINE__),                    \
                        __LINE__,                                               \
                        __FILE__);                                              \
                }                                                               \
            } CONCAT(_registered_struct_instance_, __LINE__);                   \
        }                                                                       \
    }                                                                           \
    void detail::CONCAT(_registered_test_, __LINE__)()

#define CHECK(expr) ((test::Check{} < expr) ? void(0) : test::on_check_failed(#expr, __FILE__, __LINE__))

#define REQUIRE(expr) ((test::Check{} < expr) ? void(0) : test::on_require_failed(#expr, __FILE__, __LINE__))

