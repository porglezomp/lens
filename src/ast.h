// Copyright (c) 2015 Caleb Jones
#ifndef LENS_AST_H_
#define LENS_AST_H_

#include <string>
#include <vector>
#include <iostream>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

enum AST_TYPES {
    EXPR_AST,
    NUMBER_AST,
    VARIABLE_AST,
    BINARY_EXPR_AST,
    CALL_AST,
    ASSIGNMENT_AST,
    RETURN_AST
};

llvm::Module *TheModule();

class ExprAST {
 public:
    virtual ~ExprAST() {}
    virtual llvm::Value *codegen() = 0;
    // virtual int type();

    virtual void print(std::ostream* str) const = 0;
    friend std::ostream& operator<<(std::ostream& out, ExprAST const& ast) {
        ast.print(&out);
        return out;
    }
};

class NumberAST : public ExprAST {
    double value;
 public:
    virtual void print(std::ostream* out) const;
    explicit NumberAST(double number);
    virtual llvm::Value *codegen();
    // virtual int type();
};

class VariableAST : public ExprAST {
    std::string name;
 public:
    virtual void print(std::ostream* out) const;
    explicit VariableAST(std::string str);
    virtual llvm::Value *codegen();
    // virtual int type();
};

// <expr> <op> <expr>
class BinaryExprAST : public ExprAST {
    int op;
    ExprAST *lhs, *rhs;
 public:
    virtual void print(std::ostream* out) const;
    BinaryExprAST(ExprAST *lhs, int op, ExprAST *rhs);
    virtual llvm::Value *codegen();
    // virtual int type();
};

// <ident>(<ident>, ...)
class CallAST : public ExprAST {
    std::string name;
    std::vector<ExprAST *> args;
 public:
    CallAST(std::string name, std::vector<ExprAST*> args);
    virtual void print(std::ostream* out) const;
    virtual llvm::Value *codegen();
    // virtual int type();
};

// let <ident> = <expr>
class AssignmentAST : public ExprAST {
    std::string name;
    ExprAST *rhs;
 public:
    virtual void print(std::ostream* out) const;
    AssignmentAST(std::string name, ExprAST *rhs);
    virtual llvm::Value *codegen();
    // virtual int type();
};

// return <expr>
class ReturnAST : public ExprAST {
    ExprAST *rvalue;
 public:
    virtual void print(std::ostream* out) const;
    explicit ReturnAST(ExprAST *rhs);
    virtual llvm::Value *codegen();
    // virtual int type();
};

class PrototypeAST {
 public:
    std::string name;
 private:
    std::vector<std::string> args;
 public:
    PrototypeAST(std::string name, std::vector<std::string> args);
    friend std::ostream& operator<<(std::ostream& out, PrototypeAST const& ast);
    llvm::Function *codegen();
};

class FunctionAST {
    PrototypeAST *proto;
    std::vector<ExprAST*> body;
 public:
    FunctionAST(PrototypeAST *proto, std::vector<ExprAST*> body);
    friend std::ostream& operator<<(std::ostream& out, FunctionAST const& ast);
    llvm::Function *codegen();
};

#endif  // LENS_AST_H_
