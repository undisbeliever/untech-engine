// assert.inc

architecture wdc65816-strict

include "../../../engine/common/assert.inc"

assert(1)
assert(1 + 1 == 2)

assertPowerOfTwo(0)
assertPowerOfTwo(1)
assertPowerOfTwo(2)
assertPowerOfTwo(4)
assertPowerOfTwo(8)
assertPowerOfTwo(16)
assertPowerOfTwo(32)
assertPowerOfTwo(64)
assertPowerOfTwo(128)
assertPowerOfTwo(0x80000000)

