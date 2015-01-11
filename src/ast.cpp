// Copyright (c) 2015 Caleb Jones
#include "src/ast.h"

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

NumberAST::NumberAST(double number) : value(number) {}
void NumberAST::print(std::ostream *out) const {
    *out << value;
}

VariableAST::VariableAST(std::string str) : name(str) {}
void VariableAST::print(std::ostream *out) const {
    *out << name;
}

BinaryExprAST::BinaryExprAST(ExprAST *lhs, int op, ExprAST *rhs)
    : op(op), lhs(lhs), rhs(rhs) {}
void BinaryExprAST::print(std::ostream *out) const {
    *out << "(";
    lhs->print(out);
    *out << " " << static_cast<char>(op) << " ";
    rhs->print(out);
    *out << ")";
}

AssignmentAST::AssignmentAST(std::string name, ExprAST *rhs)
    : name(name), rhs(rhs) {}
void AssignmentAST::print(std::ostream *out) const {
    *out << name << " = ";
    rhs->print(out);
}

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

FunctionAST::FunctionAST(PrototypeAST *proto, std::vector<ExprAST*> body)
    : proto(proto), body(body) {}

std::ostream& operator<<(std::ostream& out, FunctionAST const& ast) {
    out << *ast.proto << ":\n";
    for (auto iter = ast.body.begin(); iter != ast.body.end(); iter++) {
        // Dereference twice to go iterator -> ExprAST* -> ExprAST
        out << "    " << **iter << "\n";
    }
    return out;
}
