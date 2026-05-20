#ifndef DOT_WRITER_H_
#define DOT_WRITER_H_

#include "ValueIds.h"
#include "llvm/IR/Module.h"

#include <fstream>
#include <string>

namespace defuse {
class DotWriter final {
public:
    DotWriter(const std::string_view &dot_path, const std::string_view &mapping_path);

    void Write(llvm::Module &Module, ValueIds &ids);

private:
    void WriteHeader(void);
    void WriteFunctionNodes(llvm::Function &Function, ValueIds &ids);
    void WriteEdges(llvm::Module &Module, ValueIds &ids);
    void WriteFooter(void);

    std::ofstream dot_;
    std::ofstream mapping_;
};

} // defuse

#endif // DOT_WRITER_H_
