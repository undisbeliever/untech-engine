
include "../../../src/common/assert.inc"
include "../../../src/common/struct.inc"

// reuse struct should error out
namespace point {
    struct()

    field(xPos, 2)
    field(yPos, 2)

    endstruct()
}

namespace point {
    struct() // ERROR

}

