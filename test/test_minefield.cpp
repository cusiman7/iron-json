
#include "test.h"

#include <iron/json.h>

using fe::json;

// https://seriot.ch/projects/parsing_json.html
// https://github.com/nst/JSONTestSuite/tree/master/test_parsing

TEST("json::parse y_string") {
    REQUIRE(json::parse(R"("\u0060\u012a\u12AB")"));
    CHECK(json::parse(R"("\u0060\u012a\u12AB")").value().get<std::string>().value() == "`Īካ");
    REQUIRE(json::parse(R"("\uD801\udc37")"));
    CHECK(json::parse(R"("\uD801\udc37")").value().get<std::string>().value() == "𐐷");
    REQUIRE(json::parse(R"("\ud83d\ude39\ud83d\udc8d")"));
    CHECK(json::parse(R"("\ud83d\ude39\ud83d\udc8d")").value().get<std::string>().value() == "😹💍");
    REQUIRE(json::parse(R"("\"\\\/\b\f\n\r\t")"));
    CHECK(json::parse(R"("\"\\\/\b\f\n\r\t")").value().get<std::string>().value() == "\"\\/\b\f\n\r\t");
    REQUIRE(json::parse(R"("\\u0000")"));
    CHECK(json::parse(R"("\\u0000")").value().get<std::string>().value() == "\\u0000");
    REQUIRE(json::parse(R"("\"")"));
    CHECK(json::parse(R"("\"")").value().get<std::string>().value() == "\"");
    REQUIRE(json::parse(R"("a/*b*/c/*d//e")"));
    CHECK(json::parse(R"("a/*b*/c/*d//e")").value().get<std::string>().value() == "a/*b*/c/*d//e");
    REQUIRE(json::parse(R"("\\a")"));
    CHECK(json::parse(R"("\\a")").value().get<std::string>().value() == "\\a");
    REQUIRE(json::parse(R"("\\n")"));
    CHECK(json::parse(R"("\\n")").value().get<std::string>().value() == "\\n");
    REQUIRE(json::parse(R"("\u0012")"));
    CHECK(json::parse(R"("\u0012")").value().get<std::string>().value() == "\u0012");
    REQUIRE(json::parse(R"("\uFFFF")"));
    CHECK(json::parse(R"("\uFFFF")").value().get<std::string>().value() == "\uFFFF");
    REQUIRE(json::parse(R"("asd")"));
    CHECK(json::parse(R"("asd")").value().get<std::string>().value() == "asd");
    REQUIRE(json::parse(R"(["asd"])"));
    CHECK(json::parse(R"(["asd"])").value()[0].get<std::string>().value() == "asd");
    REQUIRE(json::parse(R"([ "asd"])"));
    CHECK(json::parse(R"([ "asd"])").value()[0].get<std::string>().value() == "asd");
    REQUIRE(json::parse(R"("\uDBFF\uDFFF")"));
    CHECK(json::parse(R"("\uDBFF\uDFFF")").value().get<std::string>().value() == "􏿿");
    REQUIRE(json::parse(R"("new\u00A0line")"));
    CHECK(json::parse(R"("new\u00A0line")").value().get<std::string>().value() == "new line");
    REQUIRE(json::parse(R"("􏿿")"));
    CHECK(json::parse(R"("􏿿")").value().get<std::string>().value() == "􏿿");
    REQUIRE(json::parse(R"("￿")"));
    CHECK(json::parse(R"("￿")").value().get<std::string>().value() == "￿");
    REQUIRE(json::parse(R"("\u0000")"));
    CHECK(json::parse(R"("\u0000")").value().get<std::string>().value() == std::string("\0", 1));
    REQUIRE(json::parse(R"("\u002c")"));
    CHECK(json::parse(R"("\u002c")").value().get<std::string>().value() == ",");
    REQUIRE(json::parse(R"("π")"));
    CHECK(json::parse(R"("π")").value().get<std::string>().value() == "π");
    REQUIRE(json::parse(R"("𛿿")"));
    CHECK(json::parse(R"("𛿿")").value().get<std::string>().value() == "𛿿");
    REQUIRE(json::parse(R"("asd ")"));
    CHECK(json::parse(R"("asd ")").value().get<std::string>().value() == "asd ");
    REQUIRE(json::parse(R"(" ")"));
    CHECK(json::parse(R"(" ")").value().get<std::string>().value() == " ");
    REQUIRE(json::parse(R"("\uD834\uDd1e")"));
    CHECK(json::parse(R"("\uD834\uDd1e")").value().get<std::string>().value() == "𝄞");
    REQUIRE(json::parse(R"("\u0821")"));
    CHECK(json::parse(R"("\u0821")").value().get<std::string>().value() == "\u0821");
    REQUIRE(json::parse(R"("\u0123")"));
    CHECK(json::parse(R"("\u0123")").value().get<std::string>().value() == "ģ");
    REQUIRE(json::parse(R"(" ")"));
    CHECK(json::parse(R"(" ")").value().get<std::string>().value() == " ");
    REQUIRE(json::parse(R"(" ")"));
    CHECK(json::parse(R"(" ")").value().get<std::string>().value() == " ");
    REQUIRE(json::parse(R"("\u0061\u30af\u30EA\u30b9")"));
    CHECK(json::parse(R"("\u0061\u30af\u30EA\u30b9")").value().get<std::string>().value() == "aクリス");
    REQUIRE(json::parse(R"("new\u000Aline")"));
    CHECK(json::parse(R"("new\u000Aline")").value().get<std::string>().value() == "new\nline");
    REQUIRE(json::parse(R"("\u005C")"));
    CHECK(json::parse(R"("\u005C")").value().get<std::string>().value() == "\\");
    REQUIRE(json::parse(R"("⍂㈴⍂")"));
    CHECK(json::parse(R"("⍂㈴⍂")").value().get<std::string>().value() == "⍂㈴⍂");
    REQUIRE(json::parse(R"("\uDBFF\uDFFE")"));
    CHECK(json::parse(R"("\uDBFF\uDFFE")").value().get<std::string>().value() == "􏿾");
    REQUIRE(json::parse(R"("\uD83F\uDFFE")"));
    CHECK(json::parse(R"("\uD83F\uDFFE")").value().get<std::string>().value() == "🿾");
    REQUIRE(json::parse(R"("\u200B")"));
    CHECK(json::parse(R"("\u200B")").value().get<std::string>().value() == "\u200B");
    REQUIRE(json::parse(R"("\u2064")"));
    CHECK(json::parse(R"("\u2064")").value().get<std::string>().value() == "\u2064");
    REQUIRE(json::parse(R"("\uFDD0")"));
    CHECK(json::parse(R"("\uFDD0")").value().get<std::string>().value() == "\uFDD0");
    REQUIRE(json::parse(R"("\uFFFE")"));
    CHECK(json::parse(R"("\uFFFE")").value().get<std::string>().value() == "\uFFFE");
    REQUIRE(json::parse(R"("\u0022")"));
    CHECK(json::parse(R"("\u0022")").value().get<std::string>().value() == "\u0022");
    REQUIRE(json::parse(R"("€𝄞")"));
    CHECK(json::parse(R"("€𝄞")").value().get<std::string>().value() == "€𝄞");
    REQUIRE(json::parse(R"("aa")"));
    CHECK(json::parse(R"("aa")").value().get<std::string>().value() == "aa");
}

TEST("json::parse n_string") {
    CHECK(!json::parse(" "));
    CHECK(!json::parse(R"("\uD800\")"));
    CHECK(!json::parse(R"("\uD800\u")"));
    CHECK(!json::parse(R"("\uD800\u1")"));
    CHECK(!json::parse(R"(é)"));
    CHECK(!json::parse(R"("\")"));
    CHECK(!json::parse(R"("\x00")"));
    CHECK(!json::parse(R"("\\\")"));
    CHECK(!json::parse(R"("\	")"));
    CHECK(!json::parse(R"("\🌀")"));
    CHECK(!json::parse(R"("\u�"")"));
    CHECK(!json::parse(R"("\a")"));
    CHECK(!json::parse(R"("\uqqqq")"));
    CHECK(!json::parse(R"("\�")"));
    CHECK(!json::parse(R"(\u0020"asd")"));
    CHECK(!json::parse(R"(\n)"));
    CHECK(!json::parse(R"(")"));
    CHECK(!json::parse(R"('single quote')"));
    CHECK(!json::parse(R"(abc)"));
    CHECK(!json::parse(R"("\)"));
    CHECK(!json::parse("a\u0001a"));
    CHECK(!json::parse("new\nline"));
    CHECK(!json::parse("\t"));
    CHECK(!json::parse(R"("\UA66D")"));
    CHECK(!json::parse(R"(""x)"));
}

TEST("json::parse i_string") {
    REQUIRE(json::parse(R"("\uDADA")"));
    CHECK(json::parse(R"("\uDADA")").value().get<std::string>().value() == "�");
    REQUIRE(json::parse(R"("\uD888\u1234")"));
    CHECK(json::parse(R"("\uD888\u1234")").value().get<std::string>().value() == "𥘴");
    REQUIRE(json::parse(R"("日ш�")"));
    CHECK(json::parse(R"("日ш�")").value().get<std::string>().value() == "日ш�");
    REQUIRE(json::parse(R"("���")"));
    CHECK(json::parse(R"("���")").value().get<std::string>().value() == "���");
    REQUIRE(json::parse(R"("\uD800\n")"));
    CHECK(json::parse(R"("\uD800\n")").value().get<std::string>().value() == "�\n");
    REQUIRE(json::parse(R"("\uDd1ea")"));
    CHECK(json::parse(R"("\uDd1ea")").value().get<std::string>().value() == "�a");
    REQUIRE(json::parse(R"("\uD800\uD800\n")"));
    CHECK(json::parse(R"("\uD800\uD800\n")").value().get<std::string>().value() == "�\n");
    REQUIRE(json::parse(R"("\ud800")"));
    CHECK(json::parse(R"("\ud800")").value().get<std::string>().value() == "�");
    REQUIRE(json::parse(R"("\ud800abc")"));
    CHECK(json::parse(R"("\ud800abc")").value().get<std::string>().value() == "�abc");
    REQUIRE(json::parse(R"("�")"));
    CHECK(json::parse(R"("�")").value().get<std::string>().value() == "�");
    REQUIRE(json::parse(R"("\uDd1e\uD834")"));
    CHECK(json::parse(R"("\uDd1e\uD834")").value().get<std::string>().value() == "�");
    REQUIRE(json::parse(R"("�")"));
    CHECK(json::parse(R"("�")").value().get<std::string>().value() == "�");
    REQUIRE(json::parse(R"("\uDFAA")"));
    CHECK(json::parse(R"("\uDFAA")").value().get<std::string>().value() == "�");
    REQUIRE(json::parse(R"("�")"));
    CHECK(json::parse(R"("�")").value().get<std::string>().value() == "�");
    REQUIRE(json::parse(R"("����")"));
    CHECK(json::parse(R"("����")").value().get<std::string>().value() == "����");
    REQUIRE(json::parse(R"("��")"));
    CHECK(json::parse(R"("��")").value().get<std::string>().value() == "��");
    REQUIRE(json::parse(R"("������")"));
    CHECK(json::parse(R"("������")").value().get<std::string>().value() == "������");
    REQUIRE(json::parse(R"("������")"));
    CHECK(json::parse(R"("������")").value().get<std::string>().value() == "������");
    REQUIRE(json::parse(R"("��")"));
    CHECK(json::parse(R"("��")").value().get<std::string>().value() == "��");
}
