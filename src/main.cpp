// Copyright (c) 2015 Caleb Jones
#include <string>
#include <iostream>

#include "src/parser.h"
#include "src/tokenizer.h"
#include "src/reader.h"
#include "src/ast.h"

#include "llvm/PassManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/Jit.h"

using namespace llvm;

ExecutionEngine *TheExecutionEngine;

int main() {
    // InitializeNativeTarget();

    auto module = TheModule();
    if (module == NULL) {
        std::cerr << "NO MODULE!" << std::endl;
    }
    // TheExecutionEngine = EngineBuilder(module).create();
    // if (TheExecutionEngine == NULL) {
    //     std::cerr << "NO EXECUTION ENGINE!" << std::endl;
    //     return 1;
    // }

    FunctionPassManager OurFPM(module);

    // Set up the optimizer pipeline.  Start with registering info about how the
    // target lays out data structures.
    // OurFPM.add(new DataLayout(*TheExecutionEngine->getDataLayout()));
    // Provide basic AliasAnalysis support for GVN.
    OurFPM.add(createBasicAliasAnalysisPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    OurFPM.add(createInstructionCombiningPass());
    // Reassociate expressions.
    OurFPM.add(createReassociatePass());
    // Eliminate Common SubExpressions.
    OurFPM.add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    OurFPM.add(createCFGSimplificationPass());

    OurFPM.doInitialization();

    Reader reader("test.ls");
    Tokenizer tokenizer(reader);
    Parser parser(tokenizer);

    while (true) {
        FunctionAST *result = parser.parse_top_level();
        if (result == NULL) break;

        auto code = result->codegen();
        if (code != NULL) {
            OurFPM.run(*code);
        }
    }

    TheModule()->dump();
    return 0;
}
