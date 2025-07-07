architecture wdc65816-strict

include "../../../engine/common/assert.inc"

constant a = 0x7f0000

assertLowRam(a) // ERROR

