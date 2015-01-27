// Copyright (c) 2015 Caleb Jones
#ifndef LENS_TOKENIZER_H_
#define LENS_TOKENIZER_H_ 1

#include <string>
#include <vector>

class Reader;
struct Line;

const char *token_name(int tok);

enum Tokens {
    tokInvalid = -1,
    tokEOF = 0,
    // ASCII in tokens 1-255
    // Some of the tokens are ASCII characters, but deserve names
    tokAdd = '+',
    tokSub = '-',
    tokMul = '*',
    tokDiv = '/',
    tokNewline = '\n',
    // Tokens beyond ASCII:
    tokIdentifier = 256,
    tokNumber,
    tokIndent,  // Produced when a line is indented more than the previous
    tokDedent,  // Produced when a line is less indented than the previous
    // Produced when a line is indented to less than the previous,
    // and more than the next indentation level down.
    // Example:
    // if a == b:
    // ....do_something()
    // ..bad_one()  # This line isn't indented correctly
    tokBadDedent,
    tokProduces,
    tokDef,
    tokLet,
    tokMut,
    tokStruct,
    tokFor,
    tokIn,
    tokIf,
    tokElif,
    tokElse,
    tokAnd,
    tokOr,
    tokEq,
    tokNot,
    tokAssign,
    tokNotEq,
    tokBitAnd,
    tokBitOr,
    tokXor,
    tokBitNot,
    tokIs,
    tokReturn,
    tokDefault,
    tokPass,
    tokTrue,
    tokFalse,
    tokNone,
    tokType
};

class Tokenizer {
    Reader &reader;
    std::vector<int> indent_stack;
    bool is_new_line;
    int line_index;
 public:
    explicit Tokenizer(Reader &r);
    std::string identifier_string;
    std::string number_string;
    double number_value;
    std::string token_error;
    const Line *line;
    int col;
    char next_char;

    char get_char();
    int get_token();
 private:
    int get_num();
    int get_ident();
};


#endif  // LENS_TOKENIZER_H_
