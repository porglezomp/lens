// Copyright (c) 2015 Caleb Jones
#include "src/ast.h"

#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "src/tokenizer.h"

// #include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Analysis/Verifier.h"

#define ERROR(msg, ...) (printf("Code generation error: " msg "\n", \
##__VA_ARGS__), nullptr)
#define ERRORB(msg, ...) (printf("Code generation error: " msg "\n", \
##__VA_ARGS__), false)

using namespace llvm;

void generate_prelude(Module *mod);

Module *_TheModule;
Module *TheModule() {
    if (_TheModule == NULL) {
        _TheModule = new Module("Whatever", getGlobalContext());
        generate_prelude(_TheModule);
    }
    return _TheModule;
}
IRBuilder<> Builder(getGlobalContext());
std::map<std::string, AllocaInst*> NamedValues;

void generate_prelude(Module *mod) {
    // "C" printf(fmt: ^str, args...) -> void
    std::vector<Type*> printf_args = {Builder.getInt8Ty()->getPointerTo()};

    FunctionType *printf_type =
        FunctionType::get(Builder.getInt32Ty(), printf_args, true);
    Constant *printf_func = mod->getOrInsertFunction("printf", printf_type);

    // printi64(n: i64) -> void
    std::vector<Type*> argtypes;
    argtypes.push_back(Builder.getInt64Ty());
    std::vector<Type*> v = {Type::getInt64Ty(getGlobalContext())};
    FunctionType *printi64_type = FunctionType::get(
        Type::getVoidTy(getGlobalContext()),
        v,
        false);
    Function *printi64 = Function::Create(printi64_type,
                                          Function::ExternalLinkage,
                                          "printi64",
                                          mod);
    printi64->arg_begin()->setName("n");
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", printi64);
    Builder.SetInsertPoint(bb);
    Value *fmtstr = Builder.CreateGlobalStringPtr("%li\n");
    Builder.CreateCall2(printf_func, fmtstr, printi64->arg_begin());
    Builder.CreateRetVoid();
}

// ========================================================================= //
// Numbers
// ========================================================================= //
NumberAST::NumberAST(double number) : value(number) {}

void NumberAST::print(std::ostream *out) const {
    *out << value;
}

Value *NumberAST::expr_codegen() {
    return ConstantInt::get(getGlobalContext(), APInt(64, value));
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
    auto ptr = NamedValues.find(name);
    if (ptr == NamedValues.end()) {
        return ERROR("Unknown variable name '%s'", name.c_str());
    }
    return Builder.CreateLoad(NamedValues[name], name);
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
    case '+': return Builder.CreateAdd(L, R, "addtmp");
    case '-': return Builder.CreateSub(L, R, "subtmp");
    case '*': return Builder.CreateMul(L, R, "multmp");
    case '/': return Builder.CreateSDiv(L, R, "divtmp");
    case '<': return Builder.CreateICmpSLT(L, R, "lttmp");
    case '>': return Builder.CreateICmpSGT(L, R, "gttmp");
    case tokEq: return Builder.CreateICmpEQ(L, R, "eqtmp");
    case tokNotEq: return Builder.CreateICmpNE(L, R, "neqtmp");
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
        return ERROR("unknown function '%s' referenced", name.c_str());
    }

    if (callee_function->arg_size() != args.size()) {
        return ERROR("incorrect number of arguments (%s expected %li, %li given)",
                     name.c_str(), callee_function->arg_size(), args.size());
    }

    std::vector<Value*> argv;
    for (unsigned i = 0, e = args.size(); i != e; i++) {
        argv.push_back(args[i]->expr_codegen());
        if (argv.back() == NULL) return NULL;
    }

    return Builder.CreateCall(callee_function, argv);
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
    *out << "let " << name << " = ";
    rhs->print(out);
}

bool AssignmentAST::codegen() {
    // TODO(Caleb Jones): Implement
    // Is this a correct implementation?
    auto ptr = Builder.CreateAlloca(Type::getInt64Ty(getGlobalContext()),
                                    nullptr, name);
    auto value = rhs->expr_codegen();
    if (value == NULL) return false;

    Builder.CreateStore(value, ptr);
    // value->setName(name);
    NamedValues[name] = ptr;
    return true;
}

// ========================================================================= //
// Reassignment
// ========================================================================= //
ReassignAST::ReassignAST(std::string name, ExprAST *rhs)
    : name(name), rhs(rhs) {}

void ReassignAST::print(std::ostream *out) const {
    *out << name << " = ";
    rhs->print(out);
}

bool ReassignAST::codegen() {
    auto value = rhs->expr_codegen();
    if (value == NULL) return false;
    if (NamedValues.find(name) == NamedValues.end()) return false;
    Builder.CreateStore(value, NamedValues[name]);
    return true;
}

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
// Conditional
// ========================================================================= //
IfElseAST::IfElseAST(ExprAST *cond,
                     std::vector<StatementAST*> ifbody,
                     std::vector<StatementAST*> elsebody)
    : condition(cond), ifbody(ifbody), elsebody(elsebody) {}

void IfElseAST::print(std::ostream* out) const {
    *out << "IF: ";
    *out << *condition;
    *out << "\n";
    for (auto iter = ifbody.begin(); iter != ifbody.end(); iter++) {
        *out << "    "  << **iter << "\n";
    }
    *out << "    ELSE:\n";
    for (auto iter = elsebody.begin(); iter != elsebody.end(); iter++) {
        *out << "    " << **iter << "\n";
    }
}

bool IfElseAST::codegen() {
    Function *fn = Builder.GetInsertBlock()->getParent();

    // Generate the basic blocks of the conditional
    // if <cond>:
    //     <ifbb>
    // else:
    //     <elsebb>
    // <mergebb>
    BasicBlock *ifbb = BasicBlock::Create(getGlobalContext(), "if", fn);
    BasicBlock *elsebb = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock *mergebb = BasicBlock::Create(getGlobalContext(), "ifcont");

    // Generate the code for <cond>
    Value *condval = condition->expr_codegen();
    if (condval == NULL) return ERRORB("failed generating condition for if");

    Builder.CreateCondBr(condval, ifbb, elsebb);

    Builder.SetInsertPoint(ifbb);
    for (auto iter = ifbody.begin(); iter != ifbody.end(); iter++) {
        bool success = (*iter)->codegen();
        if (!success) return ERRORB("failed generating statement in if");
    }
    // Branch back to the merge block only if the code doesn't return directly
    // out of the block. (LLVM IR doesn't like to have a branch after a return
    // because then that's a return in middle of a block)
    if (ifbody.back()->type() != RETURN_AST) Builder.CreateBr(mergebb);

    // Codegen can change the current block (nested if, for example)
    // Here we move ifbb back to the right block.
    ifbb = Builder.GetInsertBlock();

    // Add the else to the back of the function
    fn->getBasicBlockList().push_back(elsebb);

    Builder.SetInsertPoint(elsebb);
    for (auto iter = elsebody.begin(); iter != elsebody.end(); iter++) {
        bool success = (*iter)->codegen();
        if (!success) return ERRORB("failed generating statement in if");
    }
    // Branch back to the merge block only if the code doesn't return directly
    // out of the block. (LLVM IR doesn't like to have a branch after a return
    // because then that's a return in middle of a block)
    if (elsebody.back()->type() != RETURN_AST) Builder.CreateBr(mergebb);

    // Codegen can change the current block (nested if, for example)
    elsebb = Builder.GetInsertBlock();

    // Add our merge block to the back of the function
    fn->getBasicBlockList().push_back(mergebb);
    Builder.SetInsertPoint(mergebb);

    return true;
}

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
    std::vector<Type*> ints(args.size(),
                            Type::getInt64Ty(getGlobalContext()));
    auto ret_type = Type::getInt64Ty(getGlobalContext());
    if (name == "main") {
        ret_type = Type::getInt32Ty(getGlobalContext());
    }
    FunctionType *ftype = FunctionType::get(
        ret_type,
        ints,
        false);

    Function *f = Function::Create(ftype,
                                   Function::ExternalLinkage,
                                   name,
                                   TheModule());

    // Check for name conflicts
    if (f->getName() != name) {
        return ERROR("redifinition of a function");
    }

    return f;
}

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
    std::cout << *this << std::endl;
    NamedValues.clear();

    Function *function = proto->codegen();
    if (function == NULL) {
        return ERROR("Error generating function prototype");
    }

    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", function);
    Builder.SetInsertPoint(bb);

    // Set the names of all the arguments
    unsigned idx = 0;
    for (auto iter = function->arg_begin(); idx != proto->args.size(); idx++, iter++) {
        iter->setName(proto->args[idx]);
        auto ptr = Builder.CreateAlloca(Type::getInt64Ty(getGlobalContext()),
                                        nullptr, proto->args[idx]);
        Builder.CreateStore(iter, ptr);
        NamedValues[proto->args[idx]] = ptr;
    }

    for (auto iter = body.begin(); iter != body.end(); iter++) {
        bool success = (*iter)->codegen();
        if (!success) {
            return ERROR("Error generating function code");
        }
    }
    if (proto->name == "main") {
        Builder.CreateRet(
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0));
    } else if (body.back()->type() != RETURN_AST) {
        Builder.CreateRet(
            ConstantInt::get(Type::getInt64Ty(getGlobalContext()), 0));
    }

    // function->dump();
    verifyFunction(*function);

    return function;
}
