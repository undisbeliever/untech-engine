
include "../../../engine/common/assert.inc"
include "../../../engine/common/struct.inc"

// generate an error if endstruct() has no matching struct()
namespace point {
    endstruct() // ERROR
}

