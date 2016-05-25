
include "../../../src/common/struct.inc"

// should error on nested fields
scope test {
    struct()

    scope nest {
        struct()

        endstruct()
    }

    endstruct()
}

