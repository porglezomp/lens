// Copyright (c) 2015 Caleb Jones
#include <string>
#include <iostream>

#include "src/tokenizer.h"

int main() {
    auto tokenizer = Tokenizer();
    int token;
    std::cout << "Whitespace: " << tokenizer.indentation() << std::endl;
    while ((token = tokenizer.get_token()) != tokEOF) {
        // std::cout << token << std::endl;
        switch (token) {
        case tokInvalid:
            std::cout << "Invalid token at line " << tokenizer.line
                      << " column " << tokenizer.col << std::endl;
            return 1;
        case tokNumber:
            std::cout << tokenizer.number_string << std::endl;
            break;
        case tokIdentifier:
            std::cout << tokenizer.identifier_string << std::endl;
            break;
        case tokNewline:
            std::cout << "Whitespace: " << tokenizer.indentation() << std::endl;
            break;
        default:
            std::cout << (char) token << std::endl;
            break;
        }
    }
    return 0;
}
