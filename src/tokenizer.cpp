// Copyright (c) 2015 Caleb Jones
#include "src/tokenizer.h"

#include <cstdio>

Tokenizer::Tokenizer() : line(1), col(1) {
    this->get_char();
}

char Tokenizer::get_char() {
    char value = next_char;
    // Do bookkeeping on line/column numbers
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
    // The actual text of the identifier is saved in "identifier_string"
    this->identifier_string = "";
    do {
        this->identifier_string += this->get_char();
        // Read all the characters [a-zA-Z0-9]*
    } while (isalnum(this->next_char) || this->next_char == '_');
    return tokIdentifier;
}

int Tokenizer::get_num() {
    // The actual text of the number is saved in "number_string"
    this->number_string = "";
    do {
        this->number_string += this->get_char();
    } while (isdigit(this->next_char));
    // If it has a fractional portion handle it
    if (this->next_char == '.') {
        do {
            this->number_string += this->get_char();
        } while (isdigit(this->next_char));
    }
    // Numbers aren't allowed to have a letter immediately following them
    // and numbers aren't allowed to have a second decimal point
    if (isalpha(this->next_char || this->next_char == '.')) {
        // If either are found, return a "tokInvalid"
        return tokInvalid;
    }
    // Our number is fine, report that it was found
    return tokNumber;
}

int Tokenizer::get_token() {
    // Clean out any whitespace between tokens
    while (isblank(this->next_char)) {
        this->get_char();
    }
    // Identifiers: [a-zA-Z_][a-zA-Z0-9_]*
    if (isalpha(this->next_char) || this->next_char == '_') {
        return get_ident();
    }
    // Numbers: [0-9]+(.[0-9]*)?
    if (isdigit(this->next_char)) {
        return get_num();
    }
    // Recognize EOF
    if (this->next_char == EOF) {
        this->get_char();
        return tokEOF;
    }
    // Recognize newlines
    if (this->next_char == '\n') {
        this->get_char();  // Flush the lookahead
        return tokNewline;
    }
    int value = this->next_char;
    this->get_char();  // Flush the lookahead
    // Just return the ASCII value if it's not recognized
    return value;
}
