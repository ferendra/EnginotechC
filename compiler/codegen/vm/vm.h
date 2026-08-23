#ifndef ENGVEM_VM_H
#define ENGVEM_VM_H

#include "../../ast/nodes.h"
#include "../../diagnostics/diagnostic.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <variant>
#include <memory>

namespace eng {
namespace vm {

// VM Value types
using Value = std::variant<int64_t, double, std::string, bool, std::nullptr_t>;

// Bytecode instructions
enum class Opcode : uint8_t {
    PUSH_INT = 0x01,
    PUSH_FLOAT,
    PUSH_STR,
    PUSH_BOOL,
    PUSH_LOCAL,
    SET_LOCAL,
    GET_LOCAL,
    ADD, SUB, MUL, DIV, MOD,
    EQ, NEQ, LT, GT, LTE, GTE,
    AND, OR, NOT,
    CALL, RETURN,
    JUMP, JUMP_IF,
    PRINT,
    NOP = 0xFF
};

struct Instruction {
    Opcode op;
    int16_t operand = 0;
    std::string strOperand;
};

class VM {
public:
    VM();
    
    // Execute bytecode
    int execute(const std::vector<Instruction>& code, 
                const std::unordered_map<std::string, Value>& globals = {});
    
    // Get result
    Value getResult() const { return result_; }
    
    // Stack operations for debugging
    size_t stackSize() const { return stack_.size(); }
    
private:
    std::stack<Value> stack_;
    std::unordered_map<std::string, Value> locals_;
    std::unordered_map<std::string, Value> globals_;
    Value result_;
    
    Value resolveOperand(int16_t operand);
    void push(Value v);
    Value pop();
    Value binOp(Opcode op, const Value& a, const Value& b);
};

} // namespace vm
} // namespace eng

#endif // ENGVEM_VM_H
