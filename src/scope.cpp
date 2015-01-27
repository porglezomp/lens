// Copyright (c) 2015 Caleb Jones
#include "src/scope.h"

#include <string>
#include <map>

Scope::Scope() : outer(NULL), items(std::map<std::string, Value*>()) {}
Scope::Scope(Scope *outer)
    : outer(outer), items(std::map<std::string, Value*>()) {}

Value *Scope::find_variable(std::string name) {
    Value *V = items[name];
    if (V != NULL) return V;
    // Ascend the scope list unless you're the root
    if (outer != NULL) {
        return outer->find_variable(name);
    }
    // If you're the root of the scope list
    return NULL;
}
