// Test enum.inc

include "../../../src/common/assert.inc"
include "../../../src/common/struct.inc"

scope point {
    struct()

    field(xPos, 2)
    field(yPos, 2)

    endstruct()
}

assert(point.xPos == 0)
assert(point.yPos == 2)
assert(point.size == 4)

scope test {
    struct()

    field(A, 1)
    scope nest {
        field(B, 10)
    }
    field(C, 2)

    endstruct()
}

assert(test.A == 0)
assert(test.nest.B == 1)
assert(test.C == 11)
assert(test.size == 13)

