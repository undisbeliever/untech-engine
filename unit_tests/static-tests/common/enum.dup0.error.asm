
include "../../../src/common/enum.inc"

// Create two enums of the same name
scope test {
    createEnum()

    enum(A)
    enum(B)
    enum(A)
}

