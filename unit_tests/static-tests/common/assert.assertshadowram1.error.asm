architecture wdc65816-strict

include "../../../src/common/assert.inc"

constant a = 0x7e2000

assertShadowRam(a) // ERROR

