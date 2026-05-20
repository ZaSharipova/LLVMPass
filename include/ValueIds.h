#ifndef VALUE_IDS_H_
#define VALUE_IDS_H_

#include "llvm/IR/Value.h"

#include <unordered_map>

namespace defuse {
class ValueIds final {
public:
    ValueIds() = default;
    int GetOrAssign(const llvm::Value *V);
    int Size(void) const {
        return ids_.size();   
    }

    bool HasValue(const llvm::Value *Value) const {
        return ids_.find(Value) != ids_.end();
    }

private:
    std::unordered_map<const llvm::Value *, int> ids_;
};

} // defuse

#endif // VALUE_IDS_H_