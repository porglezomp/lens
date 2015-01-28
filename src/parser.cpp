// Copyright (c) 2015 Caleb Jones

#include "src/parser.h"

#include <string>
#include <vector>

#include "src/ast.h"
#include "src/tokenizer.h"
#include "src/reader.h"

#define ERROR(msg, ...) (printf("Error at line %d column %d: " msg "\n", \
tokenizer.line->line_number, tokenizer.col, ##__VA_ARGS__), nullptr)

Parser::Parser(Tokenizer &tok) : tokenizer(tok) {
    operator_precedence['+'] = 20;
    operator_precedence['-'] = 20;
    operator_precedence['*'] = 40;
    operator_precedence['/'] = 40;
    operator_precedence['<'] = 10;
    operator_precedence['>'] = 10;
    operator_precedence[tokEq] = 10;
    operator_precedence[tokIneq] = 10;
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

    if (next_token != '(') return new VariableAST(identifier_name);
    get_next_token();  // Consume '('

    std::vector<ExprAST*> args;
    if (next_token != ')') {
        while (true) {
            ExprAST *next_expr = parse_expression();
            if (next_expr == NULL) return NULL;
            args.push_back(next_expr);

            // We're done with the argument list
            if (next_token == ')') break;

            if (next_token != ',') return ERROR("expecting ',' in arguments");
            get_next_token();  // Consume ','
        }
    }
    get_next_token();  // Consume ')'

    return new CallAST(identifier_name, args);
}

ExprAST *Parser::parse_primary_expr() {
    switch (next_token) {
    default:
        return ERROR("unknown token '%s' while expecting expression",
                     token_name(next_token));
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

StatementAST *Parser::parse_assignment() {
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

StatementAST *Parser::parse_return() {
    if (next_token != tokReturn) {
        ERROR("ICE: expecting return in Parser::parse_return");
    }
    get_next_token();  // Consume 'return'

    // Get the value to be returned
    ExprAST *rvalue = parse_expression();
    if (rvalue == NULL) {
        ERROR("expecting expression after 'return' keyword");
    }
    return new ReturnAST(rvalue);
}

StatementAST *Parser::parse_line() {
    StatementAST *result = NULL;
    if (next_token == tokLet) {
        result = parse_assignment();
    } else if (next_token == tokReturn) {
        result = parse_return();
    } else if (next_token == tokIf) {
        result = parse_ifelse();
    } else {
        return parse_expression();
    }
    if (next_token == tokNewline) {
        get_next_token();  // Consume the newline at the end of the line
    }
    return result;
}

StatementAST *Parser::parse_ifelse() {
    if (next_token != tokIf) {
        return ERROR("ICE: Expecting 'if' in Parser::parse_ifelse");
    }
    get_next_token();  // Consume 'if'
    ExprAST *condition = parse_expression();
    if (condition == NULL) return ERROR("expecting condition after 'if'");
    if (next_token != ':') return ERROR("expecting ':' after condition");
    get_next_token();  // Consume ':'
    if (next_token != tokNewline) {
        return ERROR("expecting newline after if statement");
    }
    get_next_token();  // Consume newline
    if (next_token != tokIndent) {
        return ERROR("expecing indent after if statement");
    }
    get_next_token();  // Consume the indent token

    std::vector<StatementAST*> ifbody;
    while (next_token != tokDedent) {
        auto line = parse_line();
        if (line == NULL) return ERROR("Error parsing body of if");
        ifbody.push_back(line);
    }

    get_next_token();  // Consume the unindent token
    if (next_token != tokElse) return ERROR("if without else unsupported");
    get_next_token();  // Consume the 'else'
    if (next_token != ':') return ERROR("expecting ':' after 'else'");
    get_next_token();  // Consume the ':'
    if (next_token != tokNewline) return ERROR("expecting newline after ':'");
    get_next_token();  // Consume the newline
    if (next_token != tokIndent) {
        return ERROR("expecting indent after else statement");
    }
    get_next_token();  // Consume the indentation token

    std::vector<StatementAST*> elsebody;
    while (next_token != tokDedent) {
        auto line = parse_line();
        if (line == NULL) return ERROR("Error parsing body of else");
        elsebody.push_back(line);
    }

    get_next_token();  // Consume the unindent token

    return new IfElseAST(condition, ifbody, elsebody);
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
            if (next_token != tokType) {
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
    if (next_token != tokType) {
        return ERROR("expecting return type after '->'");
    }
    get_next_token();  // Consume return type

    if (next_token != ':') return ERROR("expecting ':'");
    get_next_token();  // Consume ':'

    if (next_token != tokNewline) return ERROR("expecting newline");
    get_next_token();  // Consume the newline

    // Not indenting is an error, so fail if there's none
    if (next_token != tokIndent) {
        return ERROR("expecting indent in function body");
    }
    get_next_token();  // Consume the indentation token

    std::vector<StatementAST*> body;
    // As long as the code is indented the same amount, read it
    do {
        // Parse a line of the body
        StatementAST *expr = parse_line();
        // If parsing the line fails, bail!
        if (expr == NULL) return ERROR("error in function body?");
        // Otherwise, add it to the body
        body.push_back(expr);
    } while (next_token != tokDedent);
    get_next_token();  // Consume unindent token
    PrototypeAST *proto = new PrototypeAST(function_name, args);
    return new FunctionAST(proto, body);
}

FunctionAST *Parser::parse_top_level() {
    // TODO(Caleb Jones) Is it safe to just skip blank lines like this?
    while (next_token == tokNewline) {
        // std::cerr << "Skipping toplevel newline" << std::endl;
        get_next_token();
    }
    if (next_token == tokEOF) {
        // std::cerr << "EOF" << std::endl;
        return NULL;
    }
    if (next_token == tokDef) {
        return parse_function();
    } else if (StatementAST *expr = parse_line()) {
        PrototypeAST *proto = new PrototypeAST("main",
                                               std::vector<std::string>());
        std::vector<StatementAST*> body = {expr};
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
