// Copyright (c) 2015 Caleb Jones
#include "src/tokenizer.h"

#include <cstdlib>
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
    // Check if the identifier is one of the keywords,
    // if so, return the keyword token instead of an identifier
    if (identifier_string == "def") return tokDef;
    if (identifier_string == "let") return tokLet;
    if (identifier_string == "mut") return tokMut;
    if (identifier_string == "if") return tokIf;
    if (identifier_string == "elif") return tokElif;
    if (identifier_string == "else") return tokElse;
    if (identifier_string == "for") return tokFor;
    if (identifier_string == "in") return tokIn;
    if (identifier_string == "struct") return tokStruct;
    if (identifier_string == "return") return tokReturn;
    if (identifier_string == "default") return tokDefault;
    if (identifier_string == "pass") return tokPass;
    if (identifier_string == "none") return tokNone;
    if (identifier_string == "true") return tokTrue;
    if (identifier_string == "false") return tokFalse;
    return tokIdentifier;
}

int Tokenizer::get_num() {
    // The actual text of the number is saved in "number_string"
    number_string = "";
    do {
        number_string += get_char();
    } while (isdigit(next_char));
    // If it has a fractional portion handle it
    if (next_char == '.') {
        do {
            number_string += get_char();
        } while (isdigit(next_char));
    }
    // Numbers aren't allowed to have a letter immediately following them
    // and numbers aren't allowed to have a second decimal point
    if (isalpha(next_char || next_char == '.')) {
        // If either are found, return a "tokInvalid"
        return tokInvalid;
    }
    // Our number is fine, report that it was found
    number_value = std::atof(number_string.c_str());
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
    if (next_char == EOF) {
        get_char();
        return tokEOF;
    }
    int value = next_char;
    get_char();  // Flush the lookahead
    // Just return the ASCII value if it's not recognized
    return value;
}
