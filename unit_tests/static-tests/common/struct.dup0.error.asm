
include "../../../src/common/struct.inc"

// duplicate field
scope test {
    struct()

    field(A, 2)
    field(A, 1)

    endstruct()
}

