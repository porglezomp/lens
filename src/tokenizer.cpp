// Copyright (c) 2015 Caleb Jones
#include "src/tokenizer.h"

#include <cstdio>

Tokenizer::Tokenizer() : line(1), col(1) {
    this->get_char();
}

char Tokenizer::get_char() {
    char value = next_char;
    if (value == '\n') {
        line += 1;
        col = 1;
    } else {
        col += 1;
    }
    next_char = std::getchar();
    return value;
}

int Tokenizer::indentation() {
    int indent = 0;
    while (isblank(this->next_char)) {
        if (next_char == '\t') {
            indent += 8;
            col += 7;
        } else {
            indent++;
        }
        this->get_char();
    }
    return indent;
}

int Tokenizer::get_ident() {
    return tokInvalid;
}

int Tokenizer::get_num() {
    return tokInvalid;
}

int Tokenizer::get_token() {
    if (isalpha(this->next_char)) {
        return get_ident();
    }
    if (isdigit(this->next_char)) {
        return get_num();
    }
    return tokEOF;
}
