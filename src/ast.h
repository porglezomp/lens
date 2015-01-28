// Copyright (c) 2015 Caleb Jones
#ifndef LENS_AST_H_
#define LENS_AST_H_

#include <string>
#include <vector>
#include <iostream>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

enum AST_TYPES {
    EXPR_AST,
    NUMBER_AST,
    VARIABLE_AST,
    BINARY_EXPR_AST,
    CALL_AST,
    REASSIGN_AST,
    ASSIGNMENT_AST,
    RETURN_AST,
    STATEMENT_AST,
    IF_ELSE_AST
};

llvm::Module *TheModule();

class StatementAST {
    static const int idtype = STATEMENT_AST;
 public:
    virtual ~StatementAST() {}
    virtual void print(std::ostream* str) const = 0;
    friend std::ostream& operator<<(std::ostream& out, StatementAST const& ast) {
        ast.print(&out);
        return out;
    }
    virtual bool codegen() = 0;
    virtual int type() { return StatementAST::idtype; }
};

class ExprAST : public StatementAST {
    static const int idtype = EXPR_AST;
 public:
    virtual ~ExprAST() {}
    virtual bool codegen() {
        llvm::Value *res = expr_codegen();
        return (res != NULL);
    }
    virtual llvm::Value *expr_codegen() = 0;
    virtual int type() { return ExprAST::idtype; }
};

class NumberAST : public ExprAST {
    static const int idtype = NUMBER_AST;
    double value;
 public:
    virtual void print(std::ostream* out) const;
    explicit NumberAST(double number);
    virtual llvm::Value *expr_codegen();
    virtual int type() { return NumberAST::idtype; }
};

class VariableAST : public ExprAST {
    static const int idtype = VARIABLE_AST;
    std::string name;
 public:
    virtual void print(std::ostream* out) const;
    explicit VariableAST(std::string str);
    virtual llvm::Value *expr_codegen();
    virtual int type() { return VariableAST::idtype; }
};

// <expr> <op> <expr>
class BinaryExprAST : public ExprAST {
    static const int idtype = BINARY_EXPR_AST;
    int op;
    ExprAST *lhs, *rhs;
 public:
    virtual void print(std::ostream* out) const;
    BinaryExprAST(ExprAST *lhs, int op, ExprAST *rhs);
    virtual llvm::Value *expr_codegen();
    virtual int type() { return BinaryExprAST::idtype; }
};

// <ident>(<ident>, ...)
class CallAST : public ExprAST {
    static const int idtype = CALL_AST;
    std::string name;
    std::vector<ExprAST *> args;
 public:
    CallAST(std::string name, std::vector<ExprAST*> args);
    virtual void print(std::ostream* out) const;
    virtual llvm::Value *expr_codegen();
    virtual int type() { return CallAST::idtype; }
};

// let <ident> = <expr>
class AssignmentAST : public StatementAST {
    static const int idtype = ASSIGNMENT_AST;
    std::string name;
    ExprAST *rhs;
 public:
    virtual void print(std::ostream* out) const;
    AssignmentAST(std::string name, ExprAST *rhs);
    virtual bool codegen();
    virtual int type() { return AssignmentAST::idtype; }
};

// <ident> = <expr>
class ReassignAST : public StatementAST {
    static const int idtype = REASSIGN_AST;
    std::string name;
    ExprAST *rhs;
 public:
    virtual void print(std::ostream* out) const;
    ReassignAST(std::string name, ExprAST *rhs);
    virtual bool codegen();
    virtual int type() { return ReassignAST::idtype; }
};

// return <expr>
class ReturnAST : public StatementAST {
    static const int idtype = RETURN_AST;
    ExprAST *rvalue;
 public:
    virtual void print(std::ostream* out) const;
    explicit ReturnAST(ExprAST *rhs);
    virtual bool codegen();
    virtual int type() { return ReturnAST::idtype; }
};

class IfElseAST : public StatementAST {
    static const int idtype = IF_ELSE_AST;
    ExprAST *condition;
    std::vector<StatementAST*> ifbody;
    std::vector<StatementAST*> elsebody;
 public:
    IfElseAST(ExprAST *cond,
              std::vector<StatementAST*> ifbody,
              std::vector<StatementAST*> elsebody);
    virtual void print(std::ostream* out) const;
    virtual bool codegen();
    virtual int type() { return IfElseAST::idtype; }
};

class PrototypeAST {
 public:
    std::string name;
    std::vector<std::string> args;
    PrototypeAST(std::string name, std::vector<std::string> args);
    friend std::ostream& operator<<(std::ostream& out, PrototypeAST const& ast);
    llvm::Function *codegen();
};

class FunctionAST {
    PrototypeAST *proto;
    std::vector<StatementAST*> body;
 public:
    FunctionAST(PrototypeAST *proto, std::vector<StatementAST*> body);
    friend std::ostream& operator<<(std::ostream& out, FunctionAST const& ast);
    llvm::Function *codegen();
};

#endif  // LENS_AST_H_
