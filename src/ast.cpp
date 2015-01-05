// Copyright (c) 2015 Caleb Jones
#include "src/ast.h"

#include <string>
#include <vector>

NumberAST::NumberAST(double number) : value(number) {}

VariableAST::VariableAST(std::string str) : name(str) {}

BinaryExprAST::BinaryExprAST(ExprAST *lhs, int op, ExprAST *rhs)
    : op(op), lhs(lhs), rhs(rhs) {}

PrototypeAST::PrototypeAST(std::string name, std::vector<std::string> args)
    : name(name), args(args) {}

FunctionAST::FunctionAST(PrototypeAST *proto, ExprAST *body)
    : proto(proto), body(body) {}
