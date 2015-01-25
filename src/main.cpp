// Copyright (c) 2015 Caleb Jones
#include <string>
#include <iostream>

#include "src/parser.h"
#include "src/tokenizer.h"
#include "src/ast.h"

int main() {
    auto tokenizer = Tokenizer();
    auto parser = Parser(tokenizer);
    FunctionAST *result = parser.parse_top_level();
    if (result != NULL) {
        auto code = result->codegen();
        if (code != NULL) {
            code->dump();
        }
    }
    if (result) std::cout << (*result) << std::endl;
    return 0;
}
