// Test cpu.inc

architecture wdc65816-strict

include "../../../src/common/cpu.inc"

sep($30)
assert16i() // ERROR

