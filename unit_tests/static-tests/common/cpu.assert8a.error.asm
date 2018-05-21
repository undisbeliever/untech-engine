// Test cpu.inc

architecture wdc65816-strict

include "../../../src/common/cpu.inc"

a8()
rep($20)
assert8a() // ERROR

