// Copyright (c) 2015 Caleb Jones
#ifndef LENS_SCOPE_H_
#define LENS_SCOPE_H_

#include <string>
#include <map>

class Value;

class Scope {
    Scope *outer;
    std::map<std::string, Value*> items;
 public:
    Scope();
    explicit Scope(Scope *outer);
    Value *find_variable(std::string);
    // Function *findFunction;
};

#endif  // LENS_SCOPE_H_
