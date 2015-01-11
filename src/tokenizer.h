// Copyright (c) 2015 Caleb Jones
#ifndef LENS_TOKENIZER_H_
#define LENS_TOKENIZER_H_ 1

#include <string>

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
    tokNone
};

class Tokenizer {
 public:
    Tokenizer();
    std::string identifier_string;
    std::string number_string;
    int indentation;
    double number_value;
    std::string token_error;
    int line, col;
    char next_char;

    char get_char();
    int get_indentation();
    int get_token();
 private:
    int get_num();
    int get_ident();
};


#endif  // LENS_TOKENIZER_H_
