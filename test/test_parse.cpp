
#include "test.h"

#include <iron/json.h>

#include <sstream>

using fe::json;

TEST("json::parse numbers") {
    {
        CHECK(!json::parse(""));
    }
    {
        auto j = json::parse("  0  ");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_int());
        CHECK(j.value().get<int64_t>().value() == 0);
    }
    {
        auto j = json::parse("-0");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_int());
        CHECK(j.value().get<int64_t>().value() == 0);
    }
    {
        auto j = json::parse("1");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_uint());
        CHECK(j.value().get<uint64_t>().value() == 1u);
    }
    {
        // largest uint64_t
        auto j = json::parse("18446744073709551615");
        REQUIRE(j);
        REQUIRE(j.value().is_number());
        REQUIRE(j.value().is_uint());
        CHECK(j.value().get<uint64_t>().value() == 18446744073709551615u);
    }
    {
        // largest uint64_t + 1
        CHECK(!json::parse("18446744073709551616"));
    }
    {
        // smallest int64_t
        auto j = json::parse("-9223372036854775808");
        REQUIRE(j);
        REQUIRE(j.value().is_number());
        REQUIRE(j.value().is_int());
        CHECK(j.value().get<int64_t>().value() == -9223372036854775807LL - 1LL);
    }
    {
        // smallest int64_t - 1
        CHECK(!json::parse("-9223372036854775809"));
    }
    {
        // mass of earth
        auto j = json::parse("5.972E+24");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == 5.972e24);
    }
    {
        // -mass of earth
        auto j = json::parse("-5.972E+24");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == -5.972e24);
    }
    {
        // mass of electron
        auto j = json::parse("9.109e-31");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == 9.109e-31);
    }
    {
        // -mass of electron
        auto j = json::parse("-9.109e-31");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == -9.109e-31);
    }
    {
        auto j = json::parse("-1e1");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == -10);
    }
    {
        auto j = json::parse("-0.0e0");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == -0);
    }
    {
        auto j = json::parse("-0.0E0");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == -0);
    }
    {
        auto j = json::parse("-0.0E+000001");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == -0);
    }
    {
        CHECK(!json::parse("-0.0e"));
    }
    {
        CHECK(!json::parse("-"));
    }
    {
        CHECK(!json::parse("-0.0ee"));
    }
    {
        auto j = json::parse("1.2");
        REQUIRE(j);
        CHECK(j.value().is_number());
        REQUIRE(j.value().is_double());
        CHECK(j.value().get<double>().value() == 1.2);
    }
    {
        CHECK(!json::parse("1.2,"));
    }
}

TEST("json::parse whitespace") {
    CHECK(!json::parse("               "));
    CHECK(json::parse(" \n\r\t1 \n\r\t"));
}

TEST("json::parse strings") {
    auto round_trip_string = [](const std::string& in, const std::string& raw, const std::string& out) {
        auto j = json::parse(in);
        if (!j) std::cout << j.error() << "\n";
        REQUIRE(j);
        CHECK(j.value().get<std::string>().value() == raw);
        std::stringstream ss;
        ss << j.value();
        CHECK(ss.str() == out);
    };
    round_trip_string(R"("")", "", R"("")");
    round_trip_string(R"("\n")", "\n", R"("\n")");
    round_trip_string(R"("\\n")", "\\n", R"("\\n")");
    round_trip_string(R"("\\n")", "\\n", R"("\\n")");
    round_trip_string(R"("HelloWorld")", "HelloWorld", R"("HelloWorld")");
    round_trip_string(R"("HelloWorld\n")", "HelloWorld\n", R"("HelloWorld\n")");
    round_trip_string(R"("Hello\"World\n")", "Hello\"World\n", R"("Hello\"World\n")");
    round_trip_string(R"("\\\\\\\\")", "\\\\\\\\", R"("\\\\\\\\")");
    round_trip_string(R"("\\\\\\\"")", "\\\\\\\"", R"("\\\\\\\"")");
    round_trip_string(R"("\"\"\"\"")", "\"\"\"\"", R"("\"\"\"\"")");
    round_trip_string(R"("\"Name rue")", "\"Name rue", R"("\"Name rue")");
    round_trip_string(R"("- SSH Channel data now initialized in base class (TriggerSSHChannelBase)\n- New doc w/ checklist for adding new vendor support to Trigger.")",
        "- SSH Channel data now initialized in base class (TriggerSSHChannelBase)\n- New doc w/ checklist for adding new vendor support to Trigger.",
        R"("- SSH Channel data now initialized in base class (TriggerSSHChannelBase)\n- New doc w/ checklist for adding new vendor support to Trigger.")");
    round_trip_string(R"("\"\\\/\b\f\n\r\t")", "\"\\/\b\f\n\r\t", R"("\"\\/\b\f\n\r\t")");
    round_trip_string(R"("\u0060\u012a\u12AB")", "\u0060\u012a\u12AB", "\"\u0060\u012a\u12AB\""); 
    round_trip_string(R"("\u0000")", std::string("\0", 1), R"("\u0000")"); 
    round_trip_string(R"("\uD801\udc37")", u8"𐐷", u8"\"𐐷\""); 
    round_trip_string(R"("\ud800")", u8"�", u8"\"�\""); 
}

TEST("json::parse null false true") {
    CHECK(json::parse(" null "));
    CHECK(!json::parse("n"));
    CHECK(!json::parse("nu"));
    CHECK(!json::parse("nul"));
    CHECK(!json::parse("xull"));
    CHECK(!json::parse("nxll"));
    CHECK(!json::parse("nuxl"));
    CHECK(!json::parse("nulx"));
    CHECK(!json::parse("nullx"));

    CHECK(json::parse(" false "));
    CHECK(!json::parse("f"));
    CHECK(!json::parse("fa"));
    CHECK(!json::parse("fal"));
    CHECK(!json::parse("xalse"));
    CHECK(!json::parse("fxlse"));
    CHECK(!json::parse("faxse"));
    CHECK(!json::parse("falxe"));
    CHECK(!json::parse("falsx"));
    CHECK(!json::parse("falsex"));

    CHECK(json::parse(" true "));
    CHECK(!json::parse("t"));
    CHECK(!json::parse("tr"));
    CHECK(!json::parse("tru"));
    CHECK(!json::parse("xrue"));
    CHECK(!json::parse("txue"));
    CHECK(!json::parse("trxe"));
    CHECK(!json::parse("trux"));
    CHECK(!json::parse("truex"));
}

TEST("json::parse array") {
    auto j = json::parse("[]");
    REQUIRE(j);
    CHECK(j.value().is_array());
    CHECK(!json::parse("["));
    CHECK(!json::parse("]"));
    CHECK(json::parse("[1, true, false]"));
    CHECK(json::parse("[\"hi\", true, false]"));
    CHECK(json::parse("[1, true, false, [1.2, false, []]]"));
    CHECK(json::parse("[{\"hi\": true}, false]"));
}

TEST("json::parse object") {
    auto j = json::parse("{}");
    REQUIRE(j);
    CHECK(j.value().is_object());
    CHECK(!json::parse("{"));
    CHECK(!json::parse("}"));
    CHECK(json::parse(R"({"key": true, "key2": false, "key3": null, "key4": 123})"));
    CHECK(json::parse(R"({"key": true, "key2": {"key3": null, "key4": 123}})"));
    CHECK(json::parse(R"({"key": true, "key2": [null, "key4", 123]})"));
    CHECK(json::parse(R"([{"key": true}, {"key2": [null, "str4", 123]}])"));
}

TEST("json::parse demo.json") {
    const char* data =
R"({
    "Image": {
        "Width":  800,
        "Height": 600,
        "Title":  "View from 15th Floor",
        "Thumbnail": {
            "Url":    "http://www.example.com/image/481989943",
            "Height": 125,
            "Width":  100
        },
        "Animated" : false,
        "IDs": [116, 943, 234, 38793]
      }
})";
    auto j = json::parse(data);
    CHECK(j);
    REQUIRE(j.value().is_object());
    auto image = j.value()["Image"];
    REQUIRE(image.is_object());

    REQUIRE(image["Width"].is_number());
    CHECK(image["Width"].get<int32_t>().value() == 800);

    REQUIRE(image["Height"].is_number());
    CHECK(image["Height"].get<int32_t>().value() == 600);

    REQUIRE(image["Title"].is_string());
    CHECK(image["Title"].get<std::string>().value() == "View from 15th Floor");

    auto th = image["Thumbnail"];
    REQUIRE(th.is_object());
    REQUIRE(th["Url"].is_string());
    CHECK(th["Url"].get<std::string>().value() == "http://www.example.com/image/481989943");

    REQUIRE(th["Height"].is_number());
    CHECK(th["Height"].get<int32_t>().value() == 125);

    REQUIRE(th["Width"].is_number());
    CHECK(th["Width"].get<int32_t>().value() == 100);

    REQUIRE(image["Animated"].is_boolean());
    CHECK(!image["Animated"].get<bool>().value());

    REQUIRE(image["IDs"].is_array());
    auto ids = image["IDs"];
    REQUIRE(ids.size() == 4u);
    CHECK(ids[0].get<int32_t>().value() == 116);
    CHECK(ids[1].get<int32_t>().value() == 943);
    CHECK(ids[2].get<int32_t>().value() == 234);
    CHECK(ids[3].get<int32_t>().value() == 38793);
}

TEST("parse UTF-8") {
    {
        auto j = json::parse(u8R"("olá mundo")");
        CHECK(j);
    }
    {
        auto j = json::parse(u8R"("你好世界")");
        CHECK(j);
    }
    {
        auto j = json::parse("\"\xa0\xa1\"");
        CHECK(!j);
    }
}
