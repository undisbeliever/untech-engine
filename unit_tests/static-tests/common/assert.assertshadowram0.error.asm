architecture wdc65816-strict

include "../../../engine/common/assert.inc"

constant a = 0x2000

assertLowRam(a) // ERROR

