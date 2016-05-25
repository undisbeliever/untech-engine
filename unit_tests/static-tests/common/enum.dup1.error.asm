
include "../../../src/common/enum.inc"

scope test {
    createEnum()

    enum(A)
    enum(B)
    enum(C)
}

// Try to re-declare an enum.
scope test {
    createEnum()

    enum(D)
    enum(E)
    enum(F)
}

