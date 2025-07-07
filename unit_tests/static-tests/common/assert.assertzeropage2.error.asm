architecture wdc65816-strict

include "../../../engine/common/assert.inc"

constant a = 0x802000

assertZeroPage(a) // ERROR

