architecture wdc65816-strict

include "../../../src/common/assert.inc"

constant a = 0x100

assertZeroPage(a) // ERROR

