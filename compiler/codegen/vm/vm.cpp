#include "vm.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace eng {
namespace vm {

VM::VM() : result_(nullptr) {}

void VM::push(Value v) {
    stack_.push(v);
}

Value VM::pop() {
    if (stack_.empty()) {
        return nullptr;
    }
    Value v = stack_.top();
    stack_.pop();
    return v;
}

Value VM::resolveOperand(int16_t operand) {
    if (operand >= 0 && operand < 256) {
        // Try to find in locals
        auto it = locals_.find("arg_" + std::to_string(operand));
        if (it != locals_.end()) return it->second;
    }
    return static_cast<int64_t>(operand);
}

Value VM::binOp(Opcode op, const Value& a, const Value& b) {
    if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b)) {
        int64_t av = std::get<int64_t>(a);
        int64_t bv = std::get<int64_t>(b);
        switch (op) {
            case Opcode::ADD: return av + bv;
            case Opcode::SUB: return av - bv;
            case Opcode::MUL: return av * bv;
            case Opcode::DIV: return bv != 0 ? av / bv : 0;
            case Opcode::MOD: return bv != 0 ? av % bv : 0;
            case Opcode::EQ: return av == bv;
            case Opcode::NEQ: return av != bv;
            case Opcode::LT: return av < bv;
            case Opcode::GT: return av > bv;
            case Opcode::LTE: return av <= bv;
            case Opcode::GTE: return av >= bv;
            default: return 0;
        }
    }
    return 0;
}

int VM::execute(const std::vector<Instruction>& code, 
                const std::unordered_map<std::string, Value>& globals) {
    globals_ = globals;
    locals_.clear();
    // Clear stack by creating new empty stack
    std::stack<Value> empty;
    std::swap(stack_, empty);
    
    size_t pc = 0;
    
    while (pc < code.size()) {
        const auto& instr = code[pc];
        
        switch (instr.op) {
            case Opcode::PUSH_INT:
                push(static_cast<int64_t>(instr.operand));
                pc++;
                break;
                
            case Opcode::PUSH_FLOAT:
                push(static_cast<double>(instr.operand));
                pc++;
                break;
                
            case Opcode::PUSH_STR:
                push(instr.strOperand);
                pc++;
                break;
                
            case Opcode::PUSH_BOOL:
                push(instr.operand != 0);
                pc++;
                break;
                
            case Opcode::PUSH_LOCAL: {
                auto it = locals_.find("local_" + std::to_string(instr.operand));
                if (it != locals_.end()) {
                    push(it->second);
                } else {
                    push(static_cast<int64_t>(0));
                }
                pc++;
                break;
            }
                
            case Opcode::SET_LOCAL: {
                locals_["local_" + std::to_string(instr.operand)] = pop();
                pc++;
                break;
            }
                
            case Opcode::GET_LOCAL: {
                auto it = locals_.find("local_" + std::to_string(instr.operand));
                if (it != locals_.end()) {
                    push(it->second);
                } else {
                    push(static_cast<int64_t>(0));
                }
                pc++;
                break;
            }
                
            case Opcode::ADD:
            case Opcode::SUB:
            case Opcode::MUL:
            case Opcode::DIV:
            case Opcode::MOD: {
                auto b = pop();
                auto a = pop();
                push(binOp(instr.op, a, b));
                pc++;
                break;
            }
                
            case Opcode::EQ:
            case Opcode::NEQ:
            case Opcode::LT:
            case Opcode::GT:
            case Opcode::LTE:
            case Opcode::GTE: {
                auto b = pop();
                auto a = pop();
                push(binOp(instr.op, a, b));
                pc++;
                break;
            }
                
            case Opcode::AND: {
                auto b = pop();
                auto a = pop();
                push(std::get<bool>(a) && std::get<bool>(b));
                pc++;
                break;
            }
                
            case Opcode::OR: {
                auto b = pop();
                auto a = pop();
                push(std::get<bool>(a) || std::get<bool>(b));
                pc++;
                break;
            }
                
            case Opcode::NOT: {
                auto a = pop();
                push(!std::get<bool>(a));
                pc++;
                break;
            }
                
            case Opcode::PRINT: {
                auto val = pop();
                if (std::holds_alternative<int64_t>(val)) {
                    std::cout << std::get<int64_t>(val);
                } else if (std::holds_alternative<double>(val)) {
                    std::cout << std::get<double>(val);
                } else if (std::holds_alternative<std::string>(val)) {
                    std::cout << std::get<std::string>(val);
                } else if (std::holds_alternative<bool>(val)) {
                    std::cout << (std::get<bool>(val) ? "true" : "false");
                }
                std::cout << "\n";
                pc++;
                break;
            }
                
            case Opcode::RETURN:
                result_ = stack_.empty() ? nullptr : pop();
                return 0;
                
            case Opcode::JUMP:
                pc = instr.operand;
                break;
                
            case Opcode::JUMP_IF: {
                auto cond = pop();
                if (std::get<bool>(cond)) {
                    pc = instr.operand;
                } else {
                    pc++;
                }
                break;
            }
                
            case Opcode::NOP:
                pc++;
                break;
                
            default:
                pc++;
                break;
        }
    }
    
    result_ = stack_.empty() ? nullptr : pop();
    return 0;
}

} // namespace vm
} // namespace eng
