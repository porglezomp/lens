// Copyright (c) 2015 Caleb Jones
#include "src/tokenizer.h"

#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>

#include "src/reader.h"

Tokenizer::Tokenizer(Reader &r) : reader(r), line(&reader.next_line()), col(0) {
    is_new_line = true;
    indent_stack.push_back(0);
    get_char();
}

void Tokenizer::advance_line() {
    line = &reader.next_line();
    col = line->indentation + 1;
    line_index = 0;
    is_new_line = true;
        if (line->is_eof) {
        next_char = EOF;
    } else {
        next_char = line->text[line_index++];
    }
}

char Tokenizer::get_char() {
    char value = next_char;
    // Do bookkeeping on line/column numbers
    if (value == '\n') {
        advance_line();
    } else {
        col += 1;
        if (line->is_eof) {
            next_char = EOF;
        } else {
            next_char = line->text[line_index++];
        }
    }
    return value;
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
    if (identifier_string == "re") return tokRe;
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
    if (identifier_string == "i64") return tokType;
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
    // We do this before new line handling, even though all new lines
    // should start with a character (the reader strips the indentation)
    // because we need it before comments
    while (isblank(next_char)) {
        get_char();
    }
    // Strip comments before handling new lines, because comments don't have
    // to follow indentation rules
    if (next_char == '#') {
        advance_line();
        return tokNewline;
    }
    // Handle any potential new lines
    if (is_new_line) {
        // If the current line is indented more than the previous one,
        // then send an indentation token and update the indent stack
        if (line->indentation > indent_stack.back()) {
            indent_stack.push_back(line->indentation);
            is_new_line = false;
            return tokIndent;
        }
        // If the current line is indented less than the previous one
        if (line->indentation < indent_stack.back()) {
            // Pop the current indentation level off of the stack
            indent_stack.pop_back();
            // If the current level of indentation is more than the next level
            // of indentation lower, then this is a bad indentation level.
            // See the header file for more information
            if (line->indentation > indent_stack.back()) return tokBadDedent;
            // If the indentation level is more than one level down, multiple
            // `tokDedent`s will be submitted
            return tokDedent;
        }
        // The current line has the same indentation as the previous one,
        // so we don't need to submit any tokens
        is_new_line = false;
    }
    // Identifiers: [a-zA-Z_][a-zA-Z0-9_]*
    if (isalpha(next_char) || next_char == '_') {
        return get_ident();
    }
    // Numbers: [0-9]+(.[0-9]*)?
    if (isdigit(next_char)) {
        return get_num();
    }
    // Handle multi-character tokens
    // ->
    if (next_char == '-') {
        get_char();  // Consume '-'
        if (next_char == '>') {
            // We've found a "produces" token (->)
            get_char();  // Consume '>'
            return tokProduces;
        }
        return '-';
    }

    // ==
    if (next_char == '=') {
        get_char();  // Consume '='
        if (next_char == '=') {
            // We've found an "equality" token (==)
            get_char();  // Consume '='
            return tokEq;
        }
        return '=';
    }

    // !=
    if (next_char == '!') {
        get_char();  // Consume '!'
        if (next_char == '=') {
            // We've found an "inequality" token (!=)
            get_char();  // Consume '='
            return tokIneq;
        }
        return '!';
    }

    // Recognize EOF
    if (next_char == EOF) {
        // Drop out to global scope at the end of the file
        return tokEOF;
    }
    int value = next_char;
    get_char();  // Flush the lookahead
    // Just return the ASCII value if it's not recognized
    return value;
}

char token_strings[256][2];
std::string undefined_string;

const char *token_name(int tok) {
    switch (tok) {
    case tokEOF: return "EOF";
    case tokIdentifier: return "identifier";
    case tokNumber: return "number";
    case tokProduces: return "->";
    case tokDef: return "def";
    case tokLet: return "let";
    case tokMut: return "mut";
    case tokStruct: return "struct";
    case tokFor: return "for";
    case tokIn: return "in";
    case '\n': return "\\n";
    case tokIf: return "if";
    case tokIndent: return "INDENT_TOKEN";
    case tokDedent: return "UNINDENT_TOKEN";
    case tokBadDedent: return "INVALID_UNINDENT_TOKEN";
    // tokElif,
    // tokElse,
    // tokAnd,
    // tokOr,
    // tokEq,
    // tokNot,
    // case tokAssign: return "=";
    // tokNotEq,
    // tokBitAnd,
    // tokBitOr,
    // tokXor,
    // tokBitNot,
    // tokIs,
    // tokReturn,
    // tokDefault,
    // tokPass,
    // tokTrue,
    // tokFalse,
    // tokNone,
    // tokType
    }
    if (tok > 0 && tok < 256) {
        token_strings[tok][0] = static_cast<char>(tok);
        token_strings[tok][1] = '\0';
        return static_cast<const char*>(token_strings[tok]);
    }
    undefined_string = "UNDEFINED_TOKEN(";
    undefined_string += tok;
    undefined_string += ")";
    std::cout << static_cast<int>(tok) << std::endl;
    return undefined_string.c_str();
}
