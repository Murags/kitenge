#include "symmetry/Symmetry.h"

namespace kitenge {

const char* toString(SymmetryRule rule) {
    switch (rule) {
        case SymmetryRule::Translate:
            return "Translate";
        case SymmetryRule::FourFold:
            return "4-fold";
        case SymmetryRule::Mirror:
            return "Mirror";
    }
    return "Unknown";
}

}  // namespace kitenge
