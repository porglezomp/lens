// Copyright (c) 2015 Caleb Jones

#include "src/parser.h"

#include <string>
#include <vector>

#include "src/ast.h"

#define ERROR(msg) (Error(msg), nullptr)

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
        return ERROR("expected ')'");
    }
    get_next_token();  // Consume ')'
    return inner;
}

ExprAST *Parser::parse_identifer_expr() {
    std::string identifier_name = tokenizer.identifier_string;
    get_next_token();  // Consume identifier string

    if (next_token == '(') {
        return ERROR("functions unsupported!");
    }
    return new VariableAST(identifier_name);
}

ExprAST *Parser::parse_primary_expr() {
    switch (next_token) {
    default:
        return ERROR("unknown token while expecting expression");
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

ExprAST *Parser::parse_assignment() {
    if (next_token != tokLet) {
        return ERROR("ICE: Expecting 'let' in Parser::parse_assignment");
    }
    get_next_token();  // Consume the 'let'

    std::string name;
    if (next_token != tokIdentifier) {
        return ERROR("expecting variable name after 'let'");
    }
    name = tokenizer.identifier_string;
    get_next_token();  // Consume the LHS

    if (next_token != '=') return ERROR("expecting = after variable name "
                            "(declaration without definition not supported)");
    get_next_token();  // Consume the '='

    ExprAST *rhs = parse_expression();
    if (rhs == NULL) return ERROR("expecting expression after '='");

    return new AssignmentAST(name, rhs);
}

ExprAST *Parser::parse_line() {
    ExprAST *result = NULL;
    if (next_token == tokLet) {
        result = parse_assignment();
    } else {
        return parse_expression();
    }
    if (next_token == tokNewline) {
        get_next_token();  // Consume the newline
    }
    return result;
}

FunctionAST *Parser::parse_function() {
    if (next_token != tokDef) {
        return ERROR("ICE: Expecting 'def' in Parser::parse_function");
    }
    get_next_token();  // Consume 'def'

    if (next_token != tokIdentifier) {
        return ERROR("expecting identifier after def");
    }
    std::string function_name = tokenizer.identifier_string;
    get_next_token();  // Consume identifier string

    if (next_token != '(') {
        return ERROR("expecting '(' after function name");
    }
    get_next_token();  // Consume '('

    std::vector<std::string> args;
    if (next_token != ')') {
        while (true) {
            if (next_token != tokIdentifier) {
                return ERROR("expecting identifier in argument list");
            }
            // Save the argument name
            args.push_back(tokenizer.identifier_string);
            get_next_token();  // Consume the argument name

            if (next_token != ':') {
                return ERROR("expecting ':' after argument name");
            }
            get_next_token();  // Consume ':'

            // Currently we just throw the type away, but we still want
            // to consume it
            if (next_token != tokIdentifier) {
                return ERROR("expecting type after ':'");
            }
            get_next_token();  // Consume the type name

            if (next_token == ')') {
                break;
            } else if (next_token != ',') {
                return ERROR("expecting ','");
            }
            get_next_token();  // Consume ','
        }
    }
    get_next_token();  // Consume ')'

    if (next_token != tokProduces) {
        return ERROR("expecting '->' after argument list");
    }
    get_next_token();  // Consume '->'

    // We currently only support single identifier types
    if (next_token != tokIdentifier) {
        return ERROR("expecting return type after '->'");
    }
    get_next_token();  // Consume return type

    if (next_token != ':') return ERROR("expecting ':'");
    get_next_token();  // Consume ':'

    if (next_token != tokNewline) return ERROR("expecting newline");
    get_next_token();  // Consume the newline

    // Find the indentation of the child block
    int indent = tokenizer.indentation;
    // Not indenting is an error, so fail if there's none
    if (indent == 0) {
        return ERROR("expecting indent in function body");
    }

    std::vector<ExprAST*> body;
    // As long as the code is indented the same amount, read it
    do {
        // Parse a line of the body
        ExprAST *expr = parse_line();
        // If parsing the line fails, bail!
        if (expr == NULL) return ERROR("error in function body?");
        // Otherwise, add it to the body
        body.push_back(expr);
        if (tokenizer.indentation == -1) {
            tokenizer.indentation = tokenizer.get_indentation();
        }
    } while (tokenizer.indentation == indent);
    PrototypeAST *proto = new PrototypeAST(function_name, args);
    return new FunctionAST(proto, body);
}

FunctionAST *Parser::parse_top_level() {
    if (next_token == tokDef) {
        return parse_function();
    } else if (ExprAST *expr = parse_line()) {
        PrototypeAST *proto = new PrototypeAST("", std::vector<std::string>());
        std::vector<ExprAST*> body = {expr};
        return new FunctionAST(proto, body);
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
