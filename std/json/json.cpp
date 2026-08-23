#include "json.h"
#include <sstream>
#include <cctype>

namespace eng {
namespace json {

bool Value::asBool(bool def) const {
    return type == JsonType::Bool ? boolVal : def;
}

double Value::asNumber(double def) const {
    return type == JsonType::Number ? numVal : def;
}

std::string Value::asString(const std::string& def) const {
    return type == JsonType::String ? strVal : def;
}

std::vector<Value>& Value::asArray() { return arrVal; }
const std::vector<Value>& Value::asArray() const { return arrVal; }
std::map<std::string, Value>& Value::asObject() { return objVal; }
const std::map<std::string, Value>& Value::asObject() const { return objVal; }

void Parser::skipWhitespace(const std::string& input, size_t& pos) {
    while (pos < input.size() && std::isspace((unsigned char)input[pos])) ++pos;
}

std::string Parser::parseString(const std::string& input, size_t& pos) {
    std::string result;
    ++pos; // skip opening quote
    while (pos < input.size() && input[pos] != '"') {
        if (input[pos] == '\\') {
            ++pos;
            switch (input[pos]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                default: result += input[pos]; break;
            }
        } else {
            result += input[pos];
        }
        ++pos;
    }
    ++pos; // skip closing quote
    return result;
}

Value Parser::parseValue(const std::string& input, size_t& pos) {
    skipWhitespace(input, pos);
    if (pos >= input.size()) return Value{};
    
    Value val;
    char c = input[pos];
    
    if (c == '"') {
        val.type = JsonType::String;
        val.strVal = parseString(input, pos);
    } else if (c == '{') {
        val.type = JsonType::Object;
        ++pos;
        skipWhitespace(input, pos);
        while (pos < input.size() && input[pos] != '}') {
            std::string key = parseString(input, pos);
            skipWhitespace(input, pos);
            if (input[pos] == ':') ++pos;
            val.objVal[key] = parseValue(input, pos);
            skipWhitespace(input, pos);
            if (input[pos] == ',') ++pos;
        }
        ++pos; // skip }
    } else if (c == '[') {
        val.type = JsonType::Array;
        ++pos;
        skipWhitespace(input, pos);
        while (pos < input.size() && input[pos] != ']') {
            val.arrVal.push_back(parseValue(input, pos));
            skipWhitespace(input, pos);
            if (input[pos] == ',') ++pos;
        }
        ++pos; // skip ]
    } else if (c == 't') {
        val.type = JsonType::Bool;
        val.boolVal = true;
        pos += 4; // "true"
    } else if (c == 'f') {
        val.type = JsonType::Bool;
        val.boolVal = false;
        pos += 5; // "false"
    } else if (c == 'n') {
        val.type = JsonType::Null;
        pos += 4; // "null"
    } else if (std::isdigit(c) || c == '-') {
        val.type = JsonType::Number;
        std::stringstream ss;
        while (pos < input.size() && (std::isdigit(input[pos]) || input[pos] == '.' || input[pos] == 'e' || input[pos] == 'E' || input[pos] == '+' || input[pos] == '-')) {
            ss << input[pos++];
        }
        val.numVal = std::stod(ss.str());
    }
    
    return val;
}

Value Parser::parse(const std::string& input) {
    size_t pos = 0;
    return parseValue(input, pos);
}

std::string Parser::stringify(const Value& val, bool pretty) {
    std::stringstream ss;
    switch (val.type) {
        case JsonType::Null: ss << "null"; break;
        case JsonType::Bool: ss << (val.boolVal ? "true" : "false"); break;
        case JsonType::Number: ss << val.numVal; break;
        case JsonType::String: ss << "\"" << val.strVal << "\""; break;
        case JsonType::Array: {
            ss << "[";
            for (size_t i = 0; i < val.arrVal.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << stringify(val.arrVal[i], pretty);
            }
            ss << "]";
            break;
        }
        case JsonType::Object: {
            ss << "{";
            bool first = true;
            for (const auto& [k, v] : val.objVal) {
                if (!first) ss << ", ";
                ss << "\"" << k << "\": " << stringify(v, pretty);
                first = false;
            }
            ss << "}";
            break;
        }
    }
    return ss.str();
}

} // namespace json
} // namespace eng
