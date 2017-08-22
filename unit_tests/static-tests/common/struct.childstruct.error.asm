
include "../../../src/common/struct.inc"

scope test {
    childstruct(MissingBase) // ERROR

    field(c, 1)

    endstruct()
}

