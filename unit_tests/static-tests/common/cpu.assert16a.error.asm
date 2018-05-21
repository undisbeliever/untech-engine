// Test cpu.inc

architecture wdc65816-strict

include "../../../src/common/cpu.inc"

a16()
au()
assert16a() // ERROR

