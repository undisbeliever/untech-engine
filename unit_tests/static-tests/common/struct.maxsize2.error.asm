
include "../../../src/common/struct.inc"
include "../../../src/common/assert.inc"

scope Base {
    basestruct(Base)
    struct_maxsize(12)
        field(a, 5)
    endstruct()
}

scope Child1 {
    childstruct(Base)
        field(b, 5)
    endstruct()
}

scope Child2 {
    childstruct(Base)
        field(b, 10)
    endstruct()         // fail
}

