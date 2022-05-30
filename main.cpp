
#include "json.h"
#include <iostream>
#include <fstream>
#include <vector>

std::string read_file(const char* path) {
    if(std::ifstream is{path, std::ios::binary | std::ios::ate}) {
        auto size = is.tellg();
        std::string str(size, '\0'); // construct string to stream size
        is.seekg(0);
        if(is.read(&str[0], size)) {
            return str;
        }
    }
    return {};
}

int main(int argc, const char** argv) {
    if (argc > 1) {
        std::optional<json> j = parse(read_file(argv[1]));
        if (j) {
            std::cout << *j << "\n";
        } else {
            std::cout << "nope\n";
        }
        return 0;
    }

    json null;
    for (auto& it : null) {
        std::cout << it << "\n";
    }

    json j = 1234;
    std::cout << sizeof(j) << "\n";
    std::cout << j << "\n";

    {
        json j = 127;
        std::cout << static_cast<int>(*j.get<int8_t>()) << "\n";
        j = -128;
        std::cout << static_cast<int>(*j.get<int8_t>()) << "\n";
    }
    
    for (auto& it : j) {
        std::cout << it << "\n";
    }

    json jd = 4.5;
    std::cout << jd << "\n";

    json j2 = false;
    std::cout << j2 << "\n";
    j2 = true;
    std::cout << j2 << "\n";

    json js("Hello JSON");
    std::cout << js << "\n";

    if (std::optional<std::string> s = js) {
        std::cout << s.value() << "\n";
    }

    json j3;
    j3.push_back(5);
    j3.push_back(7);
    std::cout << j3[0] << ", " << j3[1] << "\n";
   
    std::cout << j3 << "\n";
    for (const auto& v : j3) {
        std::cout << v << "\n";
    }

    json j4;
    j4["key"] = "value";
    std::cout << j4["key"] << "\n";

    for (const auto& v : j4) {
        std::cout << v << "\n";
    }

    json j5({{"hello", "lol"}, {"hrm", 123}, {"float", 3.14}});
    for (const auto& v : j5) {
        std::cout << v << "\n";
    }
    for (auto it = j5.begin(); it != j5.end(); ++it) {
        std::cout << it.key() << ": " << it.value() << "\n";
        std::cout << it.first() << ": " << it.second() << "\n";
    }
    for (const auto& [key, value] : j5.items()) {
        std::cout << key << ": " << value << "\n";
    }
    std::cout << j5 << "\n";

    {
        // create an empty structure (null)
        json j;

        // add a number that is stored as double (note the implicit conversion of j to an object)
        j["pi"] = 3.141;

        // add a Boolean that is stored as bool
        j["happy"] = true;

        // add a string that is stored as std::string
        j["name"] = "Niels";

        // add another null object by passing nullptr
        j["nothing"] = nullptr;

        // add an object inside the object
        j["answer"]["everything"] = 42;

        // add an array that is stored as std::vector (using an initializer list)
        j["list"] = { 1, 0, 2 };

        // add another object (using an initializer list of pairs)
        j["object"] = { {"currency", "USD"}, {"value", 42.99} };

        std::cout << j << "\n";
    }

    {
        json j2 = {
          {"pi", 3.141},
          {"happy", true},
          {"name", "Niels"},
          {"nothing", nullptr},
          {"answer", {
            {"everything", 42}
          }},
          {"list", {1, 0, 2}},
          {"object", {
            {"currency", "USD"},
            {"value", 42.99}
          }}
        };
        std::cout << j2 << "\n";
    }

    {
        json empty_object_implicit = json({});
        json empty_object_explicit = json::object();
    }

    json array_not_object = json::array({ {"currency", "USD"}, {"value", 42.99} });
    std::cout << array_not_object << "\n";

    {
        json j = {1, 2, 3, 4, 5};
        for (auto& it : j) {
            it = it.get<int64_t>().value() + 1;
        }
        std::cout << j << "\n";
    }

    std::cout << "---parsing---\n";

    std::vector<std::string> strings = {
        "0",
        "0,",
        "-0",
        "1",
        "18446744073709551615", // largest uint64_t
        "18446744073709551616", // largest uint64_t + 1
        "-9223372036854775808", // smallest int64_t
        "-9223372036854775809", // smallest int64_t - 1
        "5.972E+24", // mass of earth
        "-5.972E+24", // mass of earth
        "9.109e-31", // mass of electron
        "-9.109e-31", // mass of electron
        "-1e-1", // -0.1
        "-0.0e0", // -0
        "-0.0e0,", // -0
        "-0.0e+00001,", // -0
        "-0.0e", // error
        "", // error
        "-", // error
        "-0.0ee", // error
        "1.2,", // 1.2 
        "1.00000000000000000000,", // 1 
    };

    for (const auto& s : strings) {
        parsed_number n = parse_number(s.data(), s.data() + s.size());
        std::cout << "input: '" << s << "' == ";
        switch (n.type) {
            case number_t::int_num:
                std::cout << "int: " << n.i << "\n";
                break;
            case number_t::uint_num:
                std::cout << "uint: " << n.u << "\n";
                break;
            case number_t::real_num:
                std::cout << "real: " << n.d << "\n";
                break;
            case number_t::error:
                std::cout << "error: " << n.what << "\n";
                break;
        }
    }

    std::string ws = " \n\r\tHello";
    const char* c = skip_whitespace(ws.c_str(), ws.c_str() + ws.size());
    std::cout << *c << "\n";

    {
        std::vector<std::string> strings = {
            R"("")",
            R"("\n")",
            R"("\\n")",
            R"("HelloWorld")",
            R"("HelloWorld\n")",
            R"("Hello  World\n")",
            R"("Hello  World\n  \u1234")"
        };
        for (const auto& s : strings) {
            parsed_string ps = parse_string(s.data(), s.data() + s.size());
            switch (ps.t) {
                case parsed_string::type::string:
                    std::cout << "string: \"" << std::string_view(ps.s.data, ps.s.size) << "\"\n";
                    break;
                case parsed_string::type::error:
                    std::cout << "error: " << ps.error << "\n";
                    break;
            }
        }
    }

    {
        std::optional<json> j = parse("1234");
        std::cout << *j << "\n";
    }

    {
        std::cout << *parse("null") << "\n";
        parse("nulf");
        parse("nil");
        std::cout << *parse("true") << "\n";
        parse("tru");
        parse("truf");
        std::cout << *parse("false") << "\n";
        parse("fal");
        parse("falsf");
    
    }
    
    {
        std::cout << *parse("\"Hello World\"") << "\n";
    }
    
    {
        std::cout << *parse("[]") << "\n";
        parse("[");
        std::string s;
        s.reserve(10000);
        for (int i = 0; i < 10000; i++) {
            s.append("[");
        }
        parse(s);
        std::cout << *parse("[1, true, false]") << "\n";
        std::cout << *parse("[\"hi\", true, false]") << "\n";
        std::cout << *parse("[1, true, false, [1.2, false, []]]") << "\n";
        std::cout << *parse("[{\"hi\": true}, false]") << "\n";
    }
    
    {
        std::cout << *parse("{}") << "\n";
        parse("{{");
        std::cout << *parse(R"({"key": true, "key2": false, "key3": null, "key4": 123})") << "\n";
        std::cout << *parse(R"({"key": true, "key2": {"key3": null, "key4": 123}})") << "\n";
    }

    {
        std::cout << *parse(R"({"key": true, "key2": [null, "key4", 123]})") << "\n";
        std::cout << *parse(R"([{"key": true}, {"key2": [null, "str4", 123]}])") << "\n";
    }
}
