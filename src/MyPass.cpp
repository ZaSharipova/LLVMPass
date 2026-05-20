#include "DotWriter.h"
#include "Instrumenter.h"
#include "ValueIds.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"

#if __has_include("llvm/Plugins/PassPlugin.h")
    #include "llvm/Plugins/PassPlugin.h"
#else
    #include "llvm/Passes/PassPlugin.h"
#endif

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct DefUsePass : public PassInfoMixin<DefUsePass> {
    PreservedAnalyses run(Module &Module, ModuleAnalysisManager &) {
        defuse::ValueIds ids{};

        const char *dot_path = std::getenv("MYPASS_DOT_FILE");
        const char *map_path = std::getenv("MYPASS_MAP_FILE");
        defuse::DotWriter writer(dot_path ? dot_path : "dots/graph.dot",
            map_path ? map_path : "dots/mapping.txt");

        writer.Write(Module, ids);
        errs() << "Written " << (dot_path ? dot_path : "dots/graph.dot") << " and " << (map_path ? map_path : "dots/mapping.txt")  << "\n";

        defuse::Instrumenter instrumenter(Module);
        int count = instrumenter.Instrument(ids);
        errs() << "Instrumented " << count << " instructions\n";

        if (count == 0) {
            errs() << "Warning: no instructions were instrumented\n";
            return PreservedAnalyses::all();
        }

        return PreservedAnalyses::none();
    }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo
llvmGetPassPluginInfo(void) {
    return {LLVM_PLUGIN_API_VERSION, "DefUsePass", "v0.1",
        [](PassBuilder &PassBuilder) {
            PassBuilder.registerPipelineParsingCallback( // left it in case of a simple call
                [](StringRef Name, ModulePassManager &ModulePassManager, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "defuse-pass") {
                        ModulePassManager.addPass(DefUsePass());
                        return true;
                    }

                    return false;
                });

            PassBuilder.registerPipelineStartEPCallback(
                [](ModulePassManager &ModulePassManager, OptimizationLevel) {
                ModulePassManager.addPass(DefUsePass());
                });
        }};
}
