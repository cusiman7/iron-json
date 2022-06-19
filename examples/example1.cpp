
#include <iron/json.h>
#include <iostream>
#include <fstream>
#include <vector>

using fe::json;

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
        auto j = json::parse(read_file(argv[1]));
        if (j) {
            std::cout << j.value();
        } else {
            std::cout << j.error();
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
        std::cout << static_cast<int>(j.get<int8_t>().value()) << "\n";
        j = -128;
        std::cout << static_cast<int>(j.get<int8_t>().value()) << "\n";
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

    {
        json j = {
            {"key", true},
            {nullptr, "hi", 123}
        };
        std::cout << j << "\n";
    }

/*
    {
        json j = json::object(std::pair{"key", 123});
        std::cout << j << "\n";
    }
*/

    {
        json j("hello");
        std::cout << j << "\n";
        std::string s = std::move(j).get<std::string>().value();
        std::cout << s << "\n";
        std::cout << j << "\n";
    }
    {
        json j(true);
        std::cout << j << "\n";
        auto s = std::move(j).get<bool>().value();
        std::cout << s << "\n";
        std::cout << j << "\n";
    }
}
