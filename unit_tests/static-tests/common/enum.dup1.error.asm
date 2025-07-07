
include "../../../engine/common/enum.inc"

namespace test {
    createEnum()
        enum(A)
        enum(B)
        enum(C)
// endEnum() is optional
}

// Try to re-declare an enum.
namespace test {
    createEnum() // ERROR
        enum(D)
        enum(E)
        enum(F)
    endEnum()
}

