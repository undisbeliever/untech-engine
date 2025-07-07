
include "../../../engine/common/enum.inc"

// Create two enums of the same name
namespace test {
    createEnum()
        enum(A)
        enum(B)
        enum(A) // ERROR
    endEnum()
}

