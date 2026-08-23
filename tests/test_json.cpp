# Tests for std::json module
#include <testing.h>
#include <json.h>

TEST(JsonParseNull) {
    eng::json::Value val = eng::json::Parser::parse("null");
    ASSERT_EQ((int)eng::json::JsonType::Null, (int)val.type);
}

TEST(JsonParseBool) {
    eng::json::Value val = eng::json::Parser::parse("true");
    ASSERT_TRUE(val.asBool(false));
}

TEST(JsonParseNumber) {
    eng::json::Value val = eng::json::Parser::parse("42");
    ASSERT_EQ(42, (int)val.asNumber(0));
}

TEST(JsonParseString) {
    eng::json::Value val = eng::json::Parser::parse("\"hello\"");
    ASSERT_EQ(std::string("hello"), val.asString(""));
}

TEST(JsonParseArray) {
    eng::json::Value val = eng::json::Parser::parse("[1, 2, 3]");
    ASSERT_EQ(3, (int)val.asArray().size());
}

TEST(JsonParseObject) {
    eng::json::Value val = eng::json::Parser::parse("{\"key\": \"value\"}");
    ASSERT_EQ(std::string("value"), val.asObject()["key"].asString(""));
}

TEST(JsonStringify) {
    eng::json::Value val;
    val.type = eng::json::JsonType::Number;
    val.numVal = 42.0;
    ASSERT_EQ(std::string("42"), eng::json::Parser::stringify(val));
}