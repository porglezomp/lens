// Copyright (c) 2015 Caleb Jones
#include <string>
#include <iostream>

#include "src/tokenizer.h"

int main() {
    auto tokenizer = Tokenizer();
    int token;
    std::cout << tokenizer.indentation() << std::endl;
    while ((token = tokenizer.get_token()) != tokEOF) {
        std::cout << token << std::endl;
        if (token == tokInvalid) {
            std::cout << "Invalid token at line " << tokenizer.line
                      << " column " << tokenizer.col << std::endl;
            return 1;
        }
    }
    return 0;
}
