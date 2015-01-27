// Copyright (c) 2015 Caleb Jones
#include "src/ast.h"

#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

// #include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/Verifier.h"

#define ERROR(msg, ...) (printf("Code generation error: " msg "\n", \
##__VA_ARGS__), nullptr)

using namespace llvm;

Module *_TheModule;
Module *TheModule() {
    if (_TheModule == NULL) {
        _TheModule = new Module("Whatever", getGlobalContext());
    }
    return _TheModule;
}
IRBuilder<> Builder(getGlobalContext());
std::map<std::string, Value*> NamedValues;

// ========================================================================= //
// Numbers
// ========================================================================= //
NumberAST::NumberAST(double number) : value(number) {}

void NumberAST::print(std::ostream *out) const {
    *out << value;
}

Value *NumberAST::expr_codegen() {
    return ConstantFP::get(getGlobalContext(), APFloat(value));
}

// int NumberAST::type() {
//     return NUMBER_AST;
// }

// ========================================================================= //
// Variables
// ========================================================================= //
VariableAST::VariableAST(std::string str) : name(str) {}

void VariableAST::print(std::ostream *out) const {
    *out << name;
}

Value *VariableAST::expr_codegen() {
    Value *V = NamedValues[name];
    return V ? V : ERROR("Unknown variable name '%s'", name.c_str());
}

// int VariableAST::type() {
//     return VARIABLE_AST;
// }

// ========================================================================= //
// Binary Expressions
// ========================================================================= //
BinaryExprAST::BinaryExprAST(ExprAST *lhs, int op, ExprAST *rhs)
    : op(op), lhs(lhs), rhs(rhs) {}

void BinaryExprAST::print(std::ostream *out) const {
    *out << "(";
    lhs->print(out);
    *out << " " << static_cast<char>(op) << " ";
    rhs->print(out);
    *out << ")";
}

Value *BinaryExprAST::expr_codegen() {
    Value *L = lhs->expr_codegen();
    Value *R = rhs->expr_codegen();
    if (L == NULL || R == NULL) return NULL;

    switch (op) {
    case '+': return Builder.CreateFAdd(L, R, "addtmp");
    case '-': return Builder.CreateFSub(L, R, "subtmp");
    case '*': return Builder.CreateFMul(L, R, "multmp");
    case '/': return Builder.CreateFDiv(L, R, "divtmp");
    default: return ERROR("invalid binary operator");
    }
}

// int BinaryExprAST::type() {
//     return BINARY_EXPR_AST;
// }

// ========================================================================= //
// Function Calls
// ========================================================================= //
CallAST::CallAST(std::string name, std::vector<ExprAST*> args)
    : name(name), args(args) {}

void CallAST::print(std::ostream *out) const {
    *out << name << "(";
    for (auto iter = args.begin(); iter != args.end(); iter++) {
        (*iter)->print(out);
        *out << ", ";
    }
    *out << ")";
}

Value *CallAST::expr_codegen() {
    Function *callee_function = TheModule()->getFunction(name);
    if (callee_function == NULL) {
        return ERROR("unknown function referenced");
    }

    if (callee_function->arg_size() != args.size()) {
        return ERROR("incorrect number of arguments");
    }

    std::vector<Value*> argv;
    for (unsigned i = 0, e = args.size(); i != e; i++) {
        argv.push_back(args[i]->expr_codegen());
        if (argv.back() == NULL) return NULL;
    }

    return Builder.CreateCall(callee_function, argv, "calltmp");
}

// int CallAST::type() {
//     return CALL_AST;
// }

// ========================================================================= //
// Assignment
// ========================================================================= //
AssignmentAST::AssignmentAST(std::string name, ExprAST *rhs)
    : name(name), rhs(rhs) {}

void AssignmentAST::print(std::ostream *out) const {
    *out << name << " = ";
    rhs->print(out);
}

bool AssignmentAST::codegen() {
    // TODO(Caleb Jones): Implement
    // Is this a correct implementation?
    auto value = rhs->expr_codegen();
    if (value == NULL) return false;
    value->setName(name);
    NamedValues[name] = value;
    return true;
}

// int AssignmentAST::type() {
//     return ASSIGNMENT_AST;
// }

// ========================================================================= //
// Return
// ========================================================================= //
ReturnAST::ReturnAST(ExprAST *rvalue) : rvalue(rvalue) {}

void ReturnAST::print(std::ostream *out) const {
    *out << "return ";
    rvalue->print(out);
}

bool ReturnAST::codegen() {
    // TODO(Caleb Jones): Implement
    // Is this the proper implementation? Who knows!
    auto result = rvalue->expr_codegen();
    if (result == NULL) return false;
    Builder.CreateRet(result);
    return true;
}

// int ReturnAST::type() {
//     return RETURN_AST;
// }

// ========================================================================= //
// Function Prototypes
// ========================================================================= //
PrototypeAST::PrototypeAST(std::string name, std::vector<std::string> args)
    : name(name), args(args) {}

std::ostream& operator<<(std::ostream& out, PrototypeAST const& ast) {
    out << ast.name << "(";
    for (auto iter = ast.args.begin(); iter != ast.args.end(); iter++) {
        out << *iter << ", ";
    }
    out << ")";
    return out;
}

Function *PrototypeAST::codegen() {
    // Make the function type: (Currently just (double, double, ...) -> double)
    std::vector<Type*> doubles(args.size(),
                               Type::getDoubleTy(getGlobalContext()));
    auto ret_type = Type::getDoubleTy(getGlobalContext());
    if (name == "main") {
        ret_type = Type::getInt32Ty(getGlobalContext());
    }
    FunctionType *ftype = FunctionType::get(
        ret_type,
        doubles,
        false);

    Function *f = Function::Create(ftype,
                                   Function::ExternalLinkage,
                                   name,
                                   TheModule());

    // Check for name conflicts
    if (f->getName() != name) {
        return ERROR("redifinition of a function");
    }

    // Set the names of all the arguments
    unsigned idx = 0;
    for (auto iter = f->arg_begin(); idx != args.size(); idx++, iter++) {
        iter->setName(args[idx]);
        NamedValues[args[idx]] = iter;
    }

    return f;
}

// ========================================================================= //
// Conditions
// ========================================================================= //
IfElseAST::IfElseAST() {}

// ========================================================================= //
// Functions
// ========================================================================= //
FunctionAST::FunctionAST(PrototypeAST *proto, std::vector<StatementAST*> body)
    : proto(proto), body(body) {}

std::ostream& operator<<(std::ostream& out, FunctionAST const& ast) {
    out << *ast.proto << ":\n";
    for (auto iter = ast.body.begin(); iter != ast.body.end(); iter++) {
        // Dereference twice to go iterator -> StatementAST* -> StatementAST
        out << "    " << **iter << "\n";
    }
    return out;
}

Function *FunctionAST::codegen() {
    NamedValues.clear();

    Function *function = proto->codegen();
    if (function == NULL) {
        return ERROR("Error generating function prototype");
    }

    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", function);
    Builder.SetInsertPoint(bb);

    bool success;
    for (auto iter = body.begin(); iter != body.end(); iter++) {
        success = (*iter)->codegen();
        if (!success) {
            return ERROR("Error generating function code");
        }
    }
    if (proto->name == "main") {
        Builder.CreateRet(
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0));
    }

    verifyFunction(*function);

    return function;
}
