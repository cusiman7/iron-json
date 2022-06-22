
#include "test.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <regex>

namespace {
struct CheckVars {
    std::string a;
    std::string b;
    const char* comp = nullptr;
};
thread_local CheckVars check_vars_;
thread_local bool test_failure_ = false;
}

namespace test {

Test::Test(const char* name, test_fn f, int line, const char* file)
: name_(name), f_(f), line_(line), file_(file) {
}

void Test::operator()() const {
    test_failure_ = false;
    try {
        f_();
    } catch (test::test_error&) {
        test_failure_ = true;
    } catch (std::exception& e) {
        test_failure_ = true;
        std::cout << file_ << ":" << line_ << " (\"" << name_ << "\") FAILED:\n  " << e.what() << "\n";
    } catch (...) {
        test_failure_ = true;
        std::cout << file_ << ":" << line_ << " (\"" << name_ << "\") FAILED:\n  Unknown Exception\n";
    }
}

const char* Test::name() const {
    return name_;
}

static std::vector<Test>& registered_tests() {
    static std::vector<Test> tests;
    return tests;
}

void register_test(const char* name, test_fn f, int line, const char* file) {
    registered_tests().emplace_back(name, f, line, file);
}

void set_check_vars(std::string&& a, std::string&& b, const char* comp) {
    check_vars_.a = std::move(a);
    check_vars_.b = std::move(b);
    check_vars_.comp  = comp;
}

void on_check_failed(const char* expr, const char* file, int line) {
    test_failure_ = true;
    std::cout << file << ":" << line << " FAILED:\n"
              << "  CHECK( " << expr <<  " )\n"
              << "with expansion:\n  "
              << check_vars_.a << " " << check_vars_.comp << " " << check_vars_.b << "\n";
    check_vars_ = {};
}

void on_require_failed(const char* expr, const char* file, int line) {
    test_failure_ = true;
    std::cout << file << ":" << line << " FAILED:\n"
              << "  REQUIRE( " << expr <<  " )\n"
              << "with expansion:\n  "
              << check_vars_.a << " " << check_vars_.comp << " " << check_vars_.b << "\n";
    check_vars_ = {};
    throw test::test_error();
}

}

int usage(char* name) {
    std::cout << name << " [-hl] [-p pattern]\n\n"
        << "  -h: Print this help\n"
        << "  -l: List all tests\n"
        << "  -p <pattern>: Run tests that match pattern\n";
    return 0;
}

int main(int argc, char** argv) {
    struct {
        bool list = false;
        const char* pattern = nullptr;
    } args;

    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'l':
                    args.list = true;
                    break;
                case 'p':
                    i++;
                    if (i >= argc) return usage(argv[0]); 
                    args.pattern = argv[i];
                    break;
                case 'h':
                    // fallthrough
                default:
                    return usage(argv[0]);
            }
        }
    } 

    if (args.list) {
        for (const auto& t : test::registered_tests()) {
            std::cout << t.name() << "\n";
        }
        return 0;
    }

    std::regex pattern;
    if (args.pattern) {
        try {
            pattern = std::regex(args.pattern, std::regex::basic);
        } catch (...) {
            std::cout << args.pattern << " is not a valid pattern\n\n";
            return 1;
        }
    } 

    int passing_tests = 0;
    int failing_tests = 0;
    bool test_failure = false;
    for (const auto& t : test::registered_tests()) {
        if (!args.pattern) {
            t();
        } else if (std::regex_search(t.name(), pattern)) {
            t();
        } else {
            continue;
        } 
        passing_tests += test_failure_ ? 0 : 1;
        failing_tests += test_failure_ ? 1 : 0;
        test_failure |= test_failure_;
    }
    if (test_failure) {
        std::cout << "\n" << failing_tests << " of " << passing_tests + failing_tests << " TESTS FAILED\n";
    } else {
        std::cout << "\n" << passing_tests << " TESTS PASSED\n";
    }
    return test_failure ? 1 : 0;
}
