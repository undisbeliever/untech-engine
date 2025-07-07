
include "../../../engine/common/struct.inc"

// should error on nested fields
namespace test {
    struct()

    namespace nest {
        struct() // ERROR

        endstruct()
    }

    endstruct()
}

