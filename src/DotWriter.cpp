#include "DotWriter.h"

#include "Labels.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Use.h"

namespace defuse {
DotWriter::DotWriter(const std::string_view &dot_path, const std::string_view &mapping_path) {
    dot_.open(std::string(dot_path));
    mapping_.open(std::string(mapping_path));
    if (!dot_.is_open()) {
        llvm::report_fatal_error("Error: failed to open .dot file.\n");
    }

    if (!mapping_.is_open()) {
        llvm::report_fatal_error("Error: failed to open mapping file.\n");
    }
}

void DotWriter::Write(llvm::Module &Module, ValueIds &ids) {
    WriteHeader();
    for (llvm::Function &Function : Module) {
        if (Function.isDeclaration()) continue;
        WriteFunctionNodes(Function, ids);
    }

    WriteEdges(Module, ids);
    WriteFooter();
}

void DotWriter::WriteHeader(void) {
    dot_ << "digraph DefUse {\n";
    dot_ << "\trankdir=TB;\n";
    dot_ << "\tgraph [compound=true, ranksep=0.7, nodesep=0.3];\n";
    dot_ << "\tnode [shape=box, fontname=\"Courier\"];\n";
}

void DotWriter::WriteFunctionNodes(llvm::Function &Function, ValueIds &ids) {
    std::string function_name = Function.getName().str();

    dot_ << "\tsubgraph cluster_" << function_name << " {\n";
    dot_ << "\t\tlabel=\"" << EscapeForDot(function_name) << "\";\n";
    dot_ << "\t\tstyle=dashed;\n";

    for (llvm::Argument &Argument : Function.args()) {
        int id = ids.GetOrAssign(&Argument);
        dot_ << "\t\tn" << id << " [label=\""
             << "id=" << id << "\\n" << EscapeForDot(ShortLabel(&Argument))
             << "\", style=filled, fillcolor=lightyellow];\n";

        mapping_ << id << "\targ\t" << function_name << "\t"
                 << Argument.getName().str() << "\n";
    }

    for (llvm::BasicBlock &BasicBlock : Function) {
        for (llvm::Instruction &Instruction : BasicBlock) {
            int id = ids.GetOrAssign(&Instruction);
            dot_ << "\t\tn" << id << " [label=\""
                 << "id=" << id << "\\n" << EscapeForDot(ShortLabel(&Instruction))
                 << "\"];\n";

            mapping_ << id << "\tinst\t" << function_name << "\t"
                     << Instruction.getOpcodeName() << "\n";
        }
    }

    dot_ << "\t}\n";
}

void DotWriter::WriteEdges(llvm::Module &Module, ValueIds &ids) {
    for (llvm::Function &Function : Module) {
        if (Function.isDeclaration()) continue;

        for (llvm::BasicBlock &BasicBlock : Function) {
            for (llvm::Instruction &Instruction : BasicBlock) {
                int dst_id = ids.GetOrAssign(&Instruction);

                for (llvm::Use &Use : Instruction.operands()) {
                    llvm::Value *Value = Use.get();
                    if (Value == nullptr) continue;

                    if (llvm::isa<llvm::Instruction>(Value) || llvm::isa<llvm::Argument>(Value)) {
                        int src_id = ids.GetOrAssign(Value);
                        dot_ << "\tn" << src_id << " -> n" << dst_id << ";\n";
                    }
                }
            }
        }
    }

    llvm::Function *prev = nullptr; // This part was written so that the graph would be located TB (only invisible edges help make it)
    for (llvm::Function &Function : Module) {
        if (Function.isDeclaration()) continue;
        if (prev) {
            auto prev_first = (prev->arg_begin() != prev->arg_end()) ? ids.GetOrAssign(&*prev->arg_begin())
                : ids.GetOrAssign(&*prev->begin()->begin());
            auto curr_first = (Function.arg_begin() != Function.arg_end()) ? ids.GetOrAssign(&*Function.arg_begin())
                : ids.GetOrAssign(&*Function.begin()->begin());

            dot_ << "\tn" << prev_first << " -> n" << curr_first << " [style=invis, weight=1000, minlen=5];\n";
        }

        prev = &Function;
    }
}

void DotWriter::WriteFooter(void) {
    dot_ << "}\n";
    dot_.close();
    mapping_.close();
}

} // defuse
