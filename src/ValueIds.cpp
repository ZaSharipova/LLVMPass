#include "ValueIds.h"

namespace defuse {
int ValueIds::GetOrAssign(const llvm::Value *Value) {
    auto it = ids_.find(Value);
    if (it != ids_.end()) {
        return it->second;
    }

    int id = Size();
    ids_[Value] = id;

    return id;
}

} // defuse