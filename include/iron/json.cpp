
#include "json.h"

#include <iostream>
#include <sstream>
#include <iterator> // ostream_iterator

namespace {
static const char* json_control_char_codes[32] = {"\\u0000", "\\u0001", "\\u0002", "\\u0003",
    "\\u0004", "\\u0005", "\\u0006", "\\u0007", "\\b", "\\t", "\\n",
    "\\u000B", "\\f", "\\r", "\\u000E", "\\u000F", "\\u0010", "\\u0011",
    "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017", "\\u0018",
    "\\u0019", "\\u001A", "\\u001B", "\\u001C", "\\u001D", "\\u001E", "\\u001F"};

std::ostream& write_string(std::ostream& os, const fe::string_t& str) {
    os.put('"');
    size_t start = 0;
    size_t i = 0;
    for (; i < str.size; i++) {
        unsigned char c = str.data[i];
        if (c <= 0x1F || c == '"' || c == '\\') {
            os.write(str.data + start, i - start);
            start = i + 1; 
            switch (c) {
                case '"':
                    os.write("\\\"", 2);
                    break;
                case '\\':
                    os.write("\\\\", 2);
                    break;
                case '\b':
                case '\f':
                case '\n':
                case '\r':
                case '\t':
                    os.write(json_control_char_codes[c], 2);
                    break;
                 default:
                    os.write(json_control_char_codes[c], 6);
                    break;
            }
        }
    }
    os.write(str.data + start, i - start);
    os.put('"');
    return os;
}
} // namespace

namespace fe {
std::ostream& operator<<(std::ostream& os, const string& rhs) {
    os.write(rhs.data, rhs.size);
    return os;
}
    
std::string json::dump() const {
    std::stringstream ss;
    json::print(ss, *this);
    return ss.str();
}

std::ostream& json::print(std::ostream& os, const json& j) {
    switch (j.type) {
        case value_t::object:
        case value_t::owned_object: {
            if (j.value.object->empty()) {
                os.write("{}", 2);
                break;
            }
            os.put('{');
            auto b = j.value.object->cbegin();
            auto n = b;
            n++;
            auto e = j.value.object->cend();
            while (n != e) {
                write_string(os, b->first).put(':');
                print(os, b->second).put(',');
                ++b;
                ++n;
            }
            write_string(os, b->first).put(':');
            print(os, b->second);
            os.put('}');
            break;
        }
        case value_t::array:
        case value_t::owned_array: {
            if (j.value.array->empty()) {
                os.write("[]", 2);
                break;
            }
            os.put('[');
            auto b = j.value.array->begin();
            auto e = --j.value.array->end();
            while (b != e) {
                print(os, *b++).put(',');
            }
            print(os, *b);
            os.put(']');
            break;
        }
        case value_t::string:
        case value_t::owned_string:
            write_string(os, j.value.string);
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

std::ostream& json::pretty_print(std::ostream& os, const json& j, size_t& indent) {
    switch (j.type) {
        case value_t::object:
        case value_t::owned_object: {
            if (j.value.object->empty()) {
                os.write("{\n", 2);
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                os.put('}');
                break;
            }
            os.write("{\n", 2);
            indent += 2;
            auto b = j.value.object->cbegin();
            auto n = b;
            n++;
            auto e = j.value.object->cend();
            while (n != e) {
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                write_string(os, b->first).write(": ", 2);
                pretty_print(os, b->second, indent).write(",\n", 2);
                ++b;
                ++n;
            }
            std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
            write_string(os, b->first).write(": ", 2);
            pretty_print(os, b->second, indent).put('\n');
            indent -= 2;
            std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
            os.put('}');
            break;
        }
        case value_t::array:
        case value_t::owned_array: {
            if (j.value.array->empty()) {
                os.write("[\n", 2);
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                os.put(']');
                break;
            }
            os.write("[\n", 2);
            indent += 2;
            auto b = j.value.array->begin();
            auto e = --j.value.array->end();
            while (b != e) {
                std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
                pretty_print(os, *b++, indent).write(",\n", 2);
            }
            std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
            pretty_print(os, *b, indent).put('\n');
            indent -= 2;
            std::fill_n(std::ostream_iterator<char>(os), indent, ' ');
            os.put(']');
            break;
        }
        case value_t::string:
        case value_t::owned_string:
            write_string(os, j.value.string);
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

std::ostream& operator<<(std::ostream& os, const json& j) {
    size_t indent = 0;
    return json::pretty_print(os, j, indent);
}
    

} // namespace fe
