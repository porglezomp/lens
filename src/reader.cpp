// Copyright (c) 2015 Caleb Jones
#include "src/reader.h"

#include <string>
#include <vector>
#include <iostream>

Line::Line(int linenum, int indent, std::string text, bool is_eof)
    : is_eof(is_eof), line_number(linenum), indentation(indent), text(text) {}

std::ostream &operator<<(std::ostream& out, const Line &l) {
    out << l.line_number << ":" << l.indentation << " " << l.text;
    return out;
}

Reader::Reader(std::string filename) {
    input_stream = std::ifstream(filename);
    lines = std::vector<Line>();
    line_no = 0;
}

Reader::~Reader() {
    input_stream.close();
}

Line Reader::read_line() {
    if (input_stream.eof()) {
        done = true;
        return Line(line_no + 1, 0, "\n", true);
    }
    int indent = 0;
    char next_char;
    while (isblank(next_char = input_stream.get())) {
        if (next_char == '\t') {
            // Tabs are worth 8 spaces
            indent += 8;
        } else {
            indent++;
        }
    }
    std::string line = "";
    while (next_char != '\n' && !input_stream.eof()) {
        line += next_char;
        next_char = input_stream.get();
    }
    line += '\n';
    // Use line_no + 1 because this is a new line
    return Line(line_no + 1, indent, line, false);
}

const Line &Reader::next_line() {
    if (line_no >= lines.size()) {
        lines.push_back(read_line());
    }
    // Return the current line and advance the counter
    Line &l = lines[line_no++];
    return l;
}
