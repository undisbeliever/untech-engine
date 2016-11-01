
include "../../../src/common/struct.inc"
include "../../../src/common/assert.inc"

scope toolarge {
    struct()
    struct_maxsize(12)

    field(a, 5)
    field(b, 5)
    field(c, 5)

    endstruct() // ERROR
}
