
#include "test.h"

#include "../json.h"

TEST("parse numbers") {
    {
        CHECK(!parse(""));
    }
    {
        auto j = parse("  0  ");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_int());
        CHECK(*j->get<int64_t>() == 0);
    }
    {
        auto j = parse("-0");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_int());
        CHECK(*j->get<int64_t>() == 0);
    }
    {
        auto j = parse("1");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_uint());
        CHECK(*j->get<uint64_t>() == 1);
    }
    {
        // largest uint64_t
        auto j = parse("18446744073709551615");
        REQUIRE(j);
        REQUIRE(j->is_number());
        REQUIRE(j->is_uint());
        CHECK(*j->get<uint64_t>() == 18446744073709551615u);
    }
    {
        // largest uint64_t + 1
        CHECK(!parse("18446744073709551616"));
    }
    {
        // smallest int64_t
        auto j = parse("-9223372036854775808");
        REQUIRE(j);
        REQUIRE(j->is_number());
        REQUIRE(j->is_int());
        CHECK(*j->get<int64_t>() == -9223372036854775807LL - 1LL);
    }
    {
        // smallest int64_t - 1
        CHECK(!parse("-9223372036854775809"));
    }
    {
        // mass of earth
        auto j = parse("5.972E+24");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == 5.972e24);
    }
    {
        // -mass of earth
        auto j = parse("-5.972E+24");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == -5.972e24);
    }
    {
        // mass of electron
        auto j = parse("9.109e-31");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == 9.109e-31);
    }
    {
        // -mass of electron
        auto j = parse("-9.109e-31");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == -9.109e-31);
    }
    {
        auto j = parse("-1e1");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == -10);
    }
    {
        auto j = parse("-0.0e0");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == -0);
    }
    {
        auto j = parse("-0.0E0");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == -0);
    }
    {
        auto j = parse("-0.0E+000001");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == -0);
    }
    {
        CHECK(!parse("-0.0e"));
    }
    {
        CHECK(!parse("-"));
    }
    {
        CHECK(!parse("-0.0ee"));
    }
    {
        auto j = parse("1.2");
        REQUIRE(j);
        CHECK(j->is_number());
        CHECK(j->is_double());
        CHECK(*j->get<double>() == 1.2);
    }
    {
        CHECK(!parse("1.2,"));
    }
}

TEST("parse whitespace") {
    CHECK(!parse("               "));
    CHECK(parse(" \n\r\t1 \n\r\t"));
}

TEST("parse strings") {
    {
        auto j = parse(R"("")"); 
        REQUIRE(j);
        CHECK(j->get<std::string>().value() == "");
    }
    {
        auto j = parse(R"("\n")"); 
        REQUIRE(j);
        CHECK(j->get<std::string>().value() == R"(\n)");
    }
    {
        auto j = parse(R"("\\n")"); 
        REQUIRE(j);
        CHECK(j->get<std::string>().value() == R"(\\n)");
    }
    {
        auto j = parse(R"("HelloWorld")"); 
        REQUIRE(j);
        CHECK(j->get<std::string>().value() == "HelloWorld");
    }
    {
        auto j = parse(R"("HelloWorld\n")"); 
        REQUIRE(j);
        CHECK(j->get<std::string>().value() == R"(HelloWorld\n)");
    }
}

TEST("parse null false true") {
    CHECK(parse(" null "));
    CHECK(!parse("n"));
    CHECK(!parse("nu"));
    CHECK(!parse("nul"));
    CHECK(!parse("xull"));
    CHECK(!parse("nxll"));
    CHECK(!parse("nuxl"));
    CHECK(!parse("nulx"));
    CHECK(!parse("nullx"));
    
    CHECK(parse(" false "));
    CHECK(!parse("f"));
    CHECK(!parse("fa"));
    CHECK(!parse("fal"));
    CHECK(!parse("xalse"));
    CHECK(!parse("fxlse"));
    CHECK(!parse("faxse"));
    CHECK(!parse("falxe"));
    CHECK(!parse("falsx"));
    CHECK(!parse("falsex"));
    
    CHECK(parse(" true "));
    CHECK(!parse("t"));
    CHECK(!parse("tr"));
    CHECK(!parse("tru"));
    CHECK(!parse("xrue"));
    CHECK(!parse("txue"));
    CHECK(!parse("trxe"));
    CHECK(!parse("trux"));
    CHECK(!parse("truex"));
}

TEST("parse array") {
    auto j = parse("[]");
    REQUIRE(j);
    CHECK(j->is_array());
    CHECK(!parse("["));
    CHECK(!parse("]"));
    CHECK(parse("[1, true, false]"));
    CHECK(parse("[\"hi\", true, false]"));
    CHECK(parse("[1, true, false, [1.2, false, []]]"));
    CHECK(parse("[{\"hi\": true}, false]"));
}

TEST("parse object") {
    auto j = parse("{}");
    REQUIRE(j);
    CHECK(j->is_object());
    CHECK(!parse("{"));
    CHECK(!parse("}"));
    CHECK(parse(R"({"key": true, "key2": false, "key3": null, "key4": 123})"));
    CHECK(parse(R"({"key": true, "key2": {"key3": null, "key4": 123}})"));
    CHECK(parse(R"({"key": true, "key2": [null, "key4", 123]})"));
    CHECK(parse(R"([{"key": true}, {"key2": [null, "str4", 123]}])"));
}
