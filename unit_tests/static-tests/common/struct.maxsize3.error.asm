
include "../../../src/common/struct.inc"
include "../../../src/common/assert.inc"

scope Base {
    basestruct(Base)
    struct_maxsize(12)
        field(a, 5)
    endstruct()
}

scope BaseWithParent {
    basestruct(BaseWithParent, Base)
        field(b, 5)
    endstruct()
}

scope Child {
    childstruct(BaseWithParent)
        field(c, 5)
    endstruct() // ERROR
}

