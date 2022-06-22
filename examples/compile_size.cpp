
#include <iron/json.h>

using fe::json;

int main(int argc, char** argv) {
    auto result = json::parse(argv[1]);
    return !result;
}
