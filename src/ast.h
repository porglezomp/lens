// Copyright (c) 2015 Caleb Jones
#ifndef LENS_AST_H_
#define LENS_AST_H_

#include <string>
#include <vector>
struct Value;

class ExprAST {
 public:
    virtual ~ExprAST() {}
};

class NumberAST : public ExprAST {
    double value;
 public:
    explicit NumberAST(double number);
};

class VariableAST : public ExprAST {
    std::string name;
 public:
    explicit VariableAST(std::string str);
};

class BinaryExprAST : public ExprAST {
    int op;
    ExprAST *lhs, *rhs;
 public:
    BinaryExprAST(ExprAST *lhs, int op, ExprAST *rhs);
};

class PrototypeAST {
    std::string name;
    std::vector<std::string> args;
 public:
    PrototypeAST(std::string name, std::vector<std::string> args);
};

class FunctionAST {
    PrototypeAST *proto;
    ExprAST *body;
 public:
    FunctionAST(PrototypeAST *proto, ExprAST *body);
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
