
include "../../../engine/common/struct.inc"
include "../../../engine/common/assert.inc"

namespace toolarge {
    struct()
    struct_maxsize(12)

    field(a, 5)
    field(b, 5)
    field(c, 5) // ERROR
}
