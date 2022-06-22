
#include <iron/json.h>
#include <iostream>
#include <fstream>

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
    if (argc < 2) {
        std::cout << "usage: round_trip FILE\n";
        return 1;
    }
    
    auto j = json::parse(read_file(argv[1]));
    if (j) {
        std::cout << j.value();
    } else {
        std::cout << j.error();
    }
    return 0;
}
