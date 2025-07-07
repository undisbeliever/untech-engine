// Test cpu.inc

architecture wdc65816-strict

include "../../../engine/common/assert.inc"
include "../../../engine/common/cpu.inc"

a8()
assert8a()

a16()
assert16a()

i16()
assert16i()

i8()
assert8i()

punknown()

evaluate oldPc = pc()

rep($20)
assert16a()

sep($20)
assert8a()

rep($31)
assert16i()

sep($10)
assert8i()

assert(pc() == {oldPc} + 4 * 2)

