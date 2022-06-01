
#include "test.h"

#include <iron/json.h>

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
        CHECK(j.value().get<uint64_t>().value() == 1);
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
    {
        auto j = json::parse(R"("")"); 
        REQUIRE(j);
        CHECK(j.value().get<std::string>().value() == "");
    }
    {
        auto j = json::parse(R"("\n")"); 
        REQUIRE(j);
        CHECK(j.value().get<std::string>().value() == R"(\n)");
    }
    {
        auto j = json::parse(R"("\\n")"); 
        REQUIRE(j);
        CHECK(j.value().get<std::string>().value() == R"(\\n)");
    }
    {
        auto j = json::parse(R"("HelloWorld")"); 
        REQUIRE(j);
        CHECK(j.value().get<std::string>().value() == "HelloWorld");
    }
    {
        auto j = json::parse(R"("HelloWorld\n")"); 
        REQUIRE(j);
        CHECK(j.value().get<std::string>().value() == R"(HelloWorld\n)");
    }
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
