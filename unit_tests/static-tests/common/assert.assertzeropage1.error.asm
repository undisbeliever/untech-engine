architecture wdc65816-strict

include "../../../engine/common/assert.inc"

constant a = 0x7e0100

assertZeroPage(a) // ERROR

