architecture wdc65816-strict

include "../../../src/common/assert.inc"

constant a = 0x7f0000

assertShadowRam(a) // ERROR

