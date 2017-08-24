// Test enum.inc

include "../../../src/common/assert.inc"
include "../../../src/common/enum.inc"

namespace test {
    createEnum()

    enum(ZERO)
    enum(ONE)
    enum(TWO)
    enum(THREE)
}

assert(test.ZERO == 0)
assert(test.ONE == 1)
assert(test.TWO == 2)
assert(test.THREE == 3)

namespace functionTable {
    createEnum(0, 2)

    enum(initPtr)
    enum(deletePtr)
    enum(processPtr)
}

assert(functionTable.initPtr == 0)
assert(functionTable.deletePtr == 2)
assert(functionTable.processPtr == 4)

