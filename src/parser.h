// Copyright (c) 2015 Caleb Jones
#ifndef LENS_PARSER_H_
#define LENS_PARSER_H_

#include <string>
#include <map>

class ExprAST;
class FunctionAST;
class StatementAST;

class Tokenizer;

class Parser {
    Tokenizer &tokenizer;
    std::map<int, int> operator_precedence;

 public:
    explicit Parser(Tokenizer &tok);
    int next_token;
    int get_next_token();
    StatementAST *parse_line();
    ExprAST *parse_number_expr();
    ExprAST *parse_identifer_expr();
    ExprAST *parse_paren_expr();
    ExprAST *parse_primary_expr();
    StatementAST *parse_assignment();
    ExprAST *parse_binop_rhs(int precedence, ExprAST *lhs);
    StatementAST *parse_return();

    FunctionAST *parse_function();
    FunctionAST *parse_top_level();

    ExprAST *parse_expression();
    int get_token_precedence();

    void Error(std::string msg);
};

#endif  // LENS_PARSER_H_
