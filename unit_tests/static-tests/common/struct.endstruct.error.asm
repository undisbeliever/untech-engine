
include "../../../src/common/assert.inc"
include "../../../src/common/struct.inc"

// generate an error if endstruct() has no matching struct()
namespace point {
    endstruct() // ERROR
}

