
include "../../../engine/common/struct.inc"

namespace test {
    childstruct(MissingBase) // ERROR

    field(c, 1)

    endstruct()
}

