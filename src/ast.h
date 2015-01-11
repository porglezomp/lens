// Copyright (c) 2015 Caleb Jones
#ifndef LENS_AST_H_
#define LENS_AST_H_

#include <string>
#include <vector>
#include <iostream>
struct Value;

class ExprAST {
 public:
    virtual void print(std::ostream* str) const = 0;
    virtual ~ExprAST() {}
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
};

class VariableAST : public ExprAST {
    std::string name;
 public:
    virtual void print(std::ostream* out) const;
    explicit VariableAST(std::string str);
};

class BinaryExprAST : public ExprAST {
    int op;
    ExprAST *lhs, *rhs;
 public:
    virtual void print(std::ostream* out) const;
    BinaryExprAST(ExprAST *lhs, int op, ExprAST *rhs);
};

class AssignmentAST : public ExprAST {
    std::string name;
    ExprAST *rhs;
 public:
    virtual void print(std::ostream* out) const;
    AssignmentAST(std::string name, ExprAST *rhs);
};

class PrototypeAST {
    std::string name;
    std::vector<std::string> args;
 public:
    PrototypeAST(std::string name, std::vector<std::string> args);
    friend std::ostream& operator<<(std::ostream& out, PrototypeAST const& ast);
};

class FunctionAST {
    PrototypeAST *proto;
    std::vector<ExprAST*> body;
 public:
    FunctionAST(PrototypeAST *proto, std::vector<ExprAST*> body);
    friend std::ostream& operator<<(std::ostream& out, FunctionAST const& ast);
};

struct StructElement {
    std::string name;
    int type;
    Value *value;
};
class StructAST {
 public:
    std::string name;
    std::vector<StructElement> elements;
};

#endif  // LENS_AST_H_
