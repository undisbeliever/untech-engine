
include "../../../src/common/struct.inc"
include "../../../src/common/assert.inc"

namespace Base {
    basestruct()
    struct_maxsize(12)
        field(a, 5)
    endstruct()
}

namespace BaseWithParent {
    basestruct(Base)
        field(b, 5)
    endstruct()
}

namespace Child {
    childstruct(BaseWithParent)
        field(c, 5) // ERROR
    endstruct()
}

