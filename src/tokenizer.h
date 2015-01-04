// Copyright (c) 2015 Caleb Jones
#ifndef TOKENIZER_H_
#define TOKENIZER_H_ 1

#include <string>

enum Tokens {
    tokInvalid = -1,
    tokEOF = 0,
    // ASCII range in tokens 0-255
    tokIdentifier = 256,
    tokNumber,
    tokNewline,
    tokProduces
};

class Tokenizer {
public:
    Tokenizer();
    std::string token_string;
    std::string number_string;
    std::string token_error;
    int line, col;
    char next_char;

    char get_char();
    int indentation();
    int get_token();
private:
    int get_num();
    int get_ident();
};


#endif  // TOKENIZER_H_
