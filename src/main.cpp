// Copyright (c) 2015 Caleb Jones
#include <string>
#include <iostream>

#include "src/parser.h"
#include "src/tokenizer.h"

int main() {
    auto tokenizer = Tokenizer();
    auto parser = Parser(tokenizer);
    std::cout << parser.parse_top_level() << std::endl;
    return 0;
}
