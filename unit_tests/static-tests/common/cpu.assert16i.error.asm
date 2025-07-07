// Test cpu.inc

architecture wdc65816-strict

include "../../../engine/common/cpu.inc"

sep($30)
assert16i() // ERROR

