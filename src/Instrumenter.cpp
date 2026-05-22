#include "Instrumenter.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include <vector>

static constexpr const char *kRuntimePrefix = "mypass_";

namespace defuse {
Instrumenter::Instrumenter(llvm::Module &Module) : module_(Module), log_i32_(), log_i64_(), log_edge_() {
    DeclareLogFunctions();
}

void Instrumenter::DeclareLogFunctions(void) {
    llvm::LLVMContext &context = module_.getContext();
    llvm::Type *void_ty = llvm::Type::getVoidTy(context);
    llvm::Type *i32_ty = llvm::Type::getInt32Ty(context);
    llvm::Type *i64_ty = llvm::Type::getInt64Ty(context);

    log_i32_ = module_.getOrInsertFunction("mypass_log_i32__",
        llvm::FunctionType::get(void_ty, {i32_ty, i32_ty}, false));

    log_i64_ = module_.getOrInsertFunction("mypass_log_i64__",
        llvm::FunctionType::get(void_ty, {i32_ty, i64_ty}, false));

    log_edge_ = module_.getOrInsertFunction("mypass_log_edge__",
        llvm::FunctionType::get(void_ty, {i32_ty, i32_ty}, false));
}

bool Instrumenter::ShouldInstrument(const llvm::Instruction &Instruction) const {
    if (Instruction.isTerminator()) return false;
    if (llvm::isa<llvm::PHINode>(&Instruction)) return false;
    if (Instruction.getType()->isVoidTy()) return false;
    if (Instruction.getType()->isIntegerTy()) return true;
    return false;
}

void Instrumenter::InstrumentValue(llvm::Instruction &Instruction, int id) {
    llvm::Instruction *next = Instruction.getNextNode();
    if (next == nullptr) return;

    llvm::IRBuilder<> builder(next);
    llvm::Value *id_const = builder.getInt32(id);

    if (Instruction.getType()->isIntegerTy(32)) {
        builder.CreateCall(log_i32_, {id_const, &Instruction});
    } else if (Instruction.getType()->isIntegerTy(64)) {
        builder.CreateCall(log_i64_, {id_const, &Instruction});
    } else {
        llvm::Type *i64_ty = llvm::Type::getInt64Ty(module_.getContext());
        llvm::Value *extended = builder.CreateZExt(&Instruction, i64_ty, Instruction.getName() + ".zext");
        builder.CreateCall(log_i64_, {id_const, extended});
    }
}

std::optional<int> Instrumenter::FindCalleeEntryId(llvm::Argument *param, ValueIds &ids) {
    for (llvm::User *user : param->users()) {
        auto *user_instruction = llvm::dyn_cast<llvm::Instruction>(user);
        if (!user_instruction || !ids.HasValue(user_instruction)) continue;
        return ids.GetOrAssign(user_instruction);
    }

    llvm::Function *callee = param->getParent();
    for (llvm::BasicBlock &BasicBlock : *callee) {
        for (llvm::Instruction &Instruction : BasicBlock) {
            if (ids.HasValue(&Instruction)) {
                return ids.GetOrAssign(&Instruction);
            }
        }
    }

    return std::nullopt;
}

void Instrumenter::InstrumentEdges(llvm::CallInst *call, ValueIds &ids) {
    llvm::Function *callee = call->getCalledFunction();
    if (!callee || callee->isDeclaration()) return;

    for (unsigned i = 0; i < call->arg_size(); i++) {
        auto *arg_instruction = llvm::dyn_cast<llvm::Instruction>(call->getArgOperand(i));
        if (!arg_instruction || !ids.HasValue(arg_instruction)) continue;
        int from_id = ids.GetOrAssign(arg_instruction);

        auto param_it = callee->arg_begin();
        std::advance(param_it, i);
        auto to_id = FindCalleeEntryId(&*param_it, ids);
        if (!to_id) continue;

        llvm::IRBuilder<> builder(call);
        builder.CreateCall(log_edge_, {builder.getInt32(from_id),builder.getInt32(*to_id)});
    }
}

int Instrumenter::Instrument(ValueIds &ids) {
    int count = 0;

    for (llvm::Function &Function : module_) { // there we are giving ids to all the instructions
        if (Function.isDeclaration()) continue;

        for (llvm::BasicBlock &BasicBlock : Function) {
            for (llvm::Instruction &Instruction : BasicBlock) {
                if (!ShouldInstrument(Instruction)) continue;
                int id = ids.GetOrAssign(&Instruction);
                InstrumentValue(Instruction, id);
                count++;
            }
        }
    }

    for (llvm::Function &Function : module_) { // and there's another iteration to connect calls
        if (Function.isDeclaration()) continue;

        for (llvm::BasicBlock &BasicBlock : Function) {
            for (llvm::Instruction &Instruction : BasicBlock) {
                if (auto *call = llvm::dyn_cast<llvm::CallInst>(&Instruction)) {
                    InstrumentEdges(call, ids);
                }
            }
        }
    }

    return count;
}

} // defuse