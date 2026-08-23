// JSON Library for EnginotechC++
#ifndef ENG_STD_JSON_H
#define ENG_STD_JSON_H

#include <string>
#include <map>
#include <vector>
#include <variant>

namespace eng {
namespace json {

enum class JsonType {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

struct Value {
    JsonType type = JsonType::Null;
    bool boolVal = false;
    double numVal = 0.0;
    std::string strVal;
    std::vector<Value> arrVal;
    std::map<std::string, Value> objVal;
    
    // Type-safe getters with defaults
    bool asBool(bool def = false) const;
    double asNumber(double def = 0.0) const;
    std::string asString(const std::string& def = "") const;
    std::vector<Value>& asArray();
    const std::vector<Value>& asArray() const;
    std::map<std::string, Value>& asObject();
    const std::map<std::string, Value>& asObject() const;
};

class Parser {
public:
    static Value parse(const std::string& input);
    static std::string stringify(const Value& val, bool pretty = false);
private:
    static Value parseValue(const std::string& input, size_t& pos);
    static std::string parseString(const std::string& input, size_t& pos);
    static void skipWhitespace(const std::string& input, size_t& pos);
};

} // namespace json
} // namespace eng

#endif // ENG_STD_JSON_H
