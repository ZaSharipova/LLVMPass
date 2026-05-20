#ifndef LABELS_H_
#define LABELS_H_

#include "llvm/IR/Value.h"
#include <string>

namespace defuse {
std::string ShortLabel(const llvm::Value *V);
std::string EscapeForDot(const std::string_view &string);
} // defuse

#endif // LABELS_H_