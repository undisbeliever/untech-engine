// Test enum.inc

include "../../../engine/common/assert.inc"
include "../../../engine/common/struct.inc"

namespace point {
    struct()
    struct_maxsize(12)

    field(xPos, 2)
    field(yPos, 2)

    endstruct()
}

assert(point.xPos == 0)
assert(point.yPos == 2)
assert(point.size == 4)

namespace test {
    struct()

    field(A, 1)
    namespace nest {
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
namespace startTest {
    struct(0x1337)

    field(A, 1)
    namespace nest {
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
namespace TestBase {
    namespace Base {
        basestruct()
            field(a, 3)
        endstruct()
    }
    namespace BaseWithParent {
        basestruct(Base)
            field(b, 3)
        endstruct()
    }

    namespace Child1 {
        childstruct(Base)
            field(c, 2)
        endstruct()
    }

    namespace Child2 {
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

namespace TestBaseWithOffset {
    // test struct inheritance.
    namespace Base {
        basestruct_offset(0x100)
            field(a, 3)
        endstruct()
    }
    namespace BaseWithParent {
        basestruct(Base)
            field(b, 3)
        endstruct()
    }

    namespace Child1 {
        childstruct(Base)
            field(c, 2)
        endstruct()
    }

    namespace Child2 {
        childstruct(BaseWithParent)
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

