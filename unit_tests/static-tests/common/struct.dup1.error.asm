
include "../../../src/common/struct.inc"

// reuse struct should error out
scope point {
    struct()

    field(xPos, 2)
    field(yPos, 2)

    endstruct()
}

scope point {
    struct()

}

