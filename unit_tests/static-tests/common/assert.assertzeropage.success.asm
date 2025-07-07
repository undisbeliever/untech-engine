architecture wdc65816-strict

include "../../../engine/common/assert.inc"

constant a = 0x00
constant b = 0x10
constant c = 0xff
constant d = 0x7e0000
constant e = 0x7e0010
constant f = 0x7e00ff

assertZeroPage(a)
assertZeroPage(b)
assertZeroPage(c)
assertZeroPage(d)
assertZeroPage(e)
assertZeroPage(f)

