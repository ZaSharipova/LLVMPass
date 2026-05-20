#ifndef INSTRUMENTER_H_
#define INSTRUMENTER_H_

#include "ValueIds.h"
#include "llvm/IR/Module.h"

namespace defuse {
class Instrumenter final {
public:
    explicit Instrumenter(llvm::Module &Module);
    int Instrument(ValueIds &ids);

private:
    void DeclareLogFunctions(void);
    bool ShouldInstrument(const llvm::Instruction &Instruction) const;
    void InstrumentValue(llvm::Instruction &Instruction, int id);
    void InstrumentEdges(llvm::CallInst *call, ValueIds &ids);
    int  FindCalleeEntryId(llvm::Argument *param, ValueIds &ids);

    llvm::Module &module_;
    llvm::FunctionCallee log_i32_;
    llvm::FunctionCallee log_i64_;
    llvm::FunctionCallee log_edge_;
};

} // defuse

#endif // INSTRUMENTER_H_