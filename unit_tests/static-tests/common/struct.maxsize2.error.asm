
include "../../../engine/common/struct.inc"
include "../../../engine/common/assert.inc"

namespace Base {
    basestruct()
    struct_maxsize(12)
        field(a, 5)
    endstruct()
}

namespace Child1 {
    childstruct(Base)
        field(b, 5)
    endstruct()
}

namespace Child2 {
    childstruct(Base)
        field(b, 10) // ERROR
    endstruct()
}

