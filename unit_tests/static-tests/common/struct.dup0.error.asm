
include "../../../engine/common/assert.inc"
include "../../../engine/common/struct.inc"

// duplicate field
namespace test {
    struct()

    field(A, 2)
    field(A, 1) // ERROR

    endstruct()
}

