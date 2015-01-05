// Copyright (c) 2015 Caleb Jones

#include "src/parser.h"

#include <string>
#include <vector>

#include "src/ast.h"

Parser::Parser(Tokenizer tok) : tokenizer(tok) {
    operator_precedence['+'] = 20;
    operator_precedence['-'] = 20;
    operator_precedence['*'] = 40;
    operator_precedence['/'] = 40;
    get_next_token();  // Prime the token pump!
}

int Parser::get_next_token() {
    auto value = next_token;
    next_token = tokenizer.get_token();
    return value;
}

ExprAST *Parser::parse_number_expr() {
    ExprAST *result = new NumberAST(tokenizer.number_value);
    get_next_token();  // Consume the number
    return result;
}

ExprAST *Parser::parse_paren_expr() {
    get_next_token();  // Consume '('
    ExprAST *inner = parse_expression();
    if (inner == NULL) return NULL;

    if (next_token != ')') {
        Error("expected ')'");
        return NULL;
    }
    get_next_token();  // Consume ')'
    return inner;
}

ExprAST *Parser::parse_identifer_expr() {
    std::string identifier_name = tokenizer.identifier_string;
    get_next_token();  // Consume identifier string

    if (next_token != '(') {
        return new VariableAST(identifier_name);
    } else {
        Error("functions unsupported!");
        return NULL;
    }
}

ExprAST *Parser::parse_primary_expr() {
    switch (next_token) {
    default:
        Error("unknown token while expecting expression");
        return NULL;
    case tokIdentifier:
        return Parser::parse_identifer_expr();
    case tokNumber:
        return Parser::parse_number_expr();
    case '(':
        return Parser::parse_paren_expr();
    }
}

int Parser::get_token_precedence() {
    int precedence = operator_precedence[next_token];
    if (precedence <= 0) return -1;
    return precedence;
}

ExprAST *Parser::parse_expression() {
    ExprAST *lhs = parse_primary_expr();
    if (lhs == NULL) return 0;

    return parse_binop_rhs(0, lhs);
}

FunctionAST *Parser::parse_top_level() {
    if (ExprAST *expr = parse_expression()) {
        PrototypeAST *proto = new PrototypeAST("", std::vector<std::string>());
        return new FunctionAST(proto, expr);
    }
    return NULL;
}

ExprAST *Parser::parse_binop_rhs(int expr_precedence, ExprAST *lhs) {
    while (true) {
        int tok_precedence = get_token_precedence();

        // If the operator is less strong than the current expression,
        // we're done
        if (tok_precedence < expr_precedence) {
            return lhs;
        }

        // We know that it's a binary operator since it has > 0 precedence
        int binop = next_token;
        get_next_token();  // Consume operator

        // Parse the next expression for the RHS
        ExprAST *rhs = parse_primary_expr();
        if (rhs == NULL) return NULL;

        // If this binds more strongly, recurse into it
        int next_precedence = get_token_precedence();
        if (tok_precedence < next_precedence) {
            // Recurse into parsing! Elegance!
            rhs = parse_binop_rhs(tok_precedence+1, rhs);
            if (rhs == NULL) return NULL;
        }

        lhs = new BinaryExprAST(lhs, binop, rhs);
    }  // while loop
}

void Parser::Error(std::string msg) {
    printf("Error at line %d column %d: %s\n",
           tokenizer.line, tokenizer.col, msg.c_str());
}
