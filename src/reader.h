// Copyright (c) 2015 Caleb Jones
#ifndef LENS_READER_H_
#define LENS_READER_H_

#include <fstream>
#include <string>
#include <vector>

struct Line {
    bool is_eof;
    int line_number;
    int indentation;
    std::string text;
    Line(int linenum, int indent, std::string text, bool iseof);
    friend std::ostream &operator<<(std::ostream& out, const Line &l);
};

class Reader {
    std::ifstream input_stream;
    int line_no;
    bool done;
    std::vector<Line> lines;
    Line read_line();

 public:
    explicit Reader(std::string filename);
    ~Reader();
    const Line &next_line();
};

#endif  // LENS_READER_H_
