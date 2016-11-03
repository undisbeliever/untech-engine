// Test enum.inc

include "../../../src/common/assert.inc"
include "../../../src/common/struct.inc"

scope point {
    struct()
    struct_maxsize(12)

    field(xPos, 2)
    field(yPos, 2)

    endstruct()
}

assert(point.xPos == 0)
assert(point.yPos == 2)
assert(point.size == 4)

scope test {
    struct()

    field(A, 1)
    scope nest {
        field(B, 10)
    }
    field(C, 2)

    endstruct()
}

assert(test.A == 0)
assert(test.nest.B == 1)
assert(test.C == 11)
assert(test.size == 13)


// test struct with a given starting value
scope startTest {
    struct(0x1337)

    field(A, 1)
    scope nest {
        field(B, 10)
    }
    field(C, 2)

    endstruct()
}

assert(startTest.A == 0x1337)
assert(startTest.nest.B == 0x1337 + 1)
assert(startTest.C == 0x1337 + 11)
assert(startTest.size == 13)

// test struct inheritance.
scope TestBase {
    scope Base {
        basestruct(Base)
            field(a, 3)
        endstruct()
    }
    scope BaseWithParent {
        basestruct(BaseWithParent, Base)
            field(b, 3)
        endstruct()
    }

    scope Child1 {
        childstruct(Base)
            field(c, 2)
        endstruct()
    }

    scope Child2 {
        childstruct(BaseWithParent)
            field(d, 2)
        endstruct()
    }

    assert(Base.a == 0)
    assert(Base.size == 3)

    assert(BaseWithParent.a == 0)
    assert(BaseWithParent.b == 3)
    assert(BaseWithParent.size == 6)

    assert(Child1.a == 0)
    assert(Child1.c == 3)
    assert(Child1.size == 5)

    assert(Child2.a == 0)
    assert(Child2.b == 3)
    assert(Child2.d == 6)
    assert(Child2.size == 8)
}

scope TestBaseWithOffset {
    // test struct inheritance.
    scope Base {
        basestruct_offset(OffsetBase, 0x100)
            field(a, 3)
        endstruct()
    }
    scope BaseWithParent {
        basestruct(OffsetBaseWithParent, OffsetBase)
            field(b, 3)
        endstruct()
    }

    scope Child1 {
        childstruct(OffsetBase)
            field(c, 2)
        endstruct()
    }

    scope Child2 {
        childstruct(OffsetBaseWithParent)
            field(d, 2)
        endstruct()
    }

    assert(Base.a == 0x100)
    assert(Base.size == 3)

    assert(BaseWithParent.a == 0x100)
    assert(BaseWithParent.b == 0x103)
    assert(BaseWithParent.size == 6)

    assert(Child1.a == 0x100)
    assert(Child1.c == 0x103)
    assert(Child1.size == 5)

    assert(Child2.a == 0x100)
    assert(Child2.b == 0x103)
    assert(Child2.d == 0x106)
    assert(Child2.size == 8)
}

