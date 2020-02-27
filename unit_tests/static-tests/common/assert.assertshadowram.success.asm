architecture wdc65816-strict

include "../../../src/common/assert.inc"

constant a = 0x0000
constant b = 0x0420
constant c = 0x1fff
constant d = 0x7e0000
constant e = 0x7e0420
constant f = 0x7e1fff

assertShadowRam(a)
assertShadowRam(b)
assertShadowRam(c)
assertShadowRam(d)
assertShadowRam(e)
assertShadowRam(f)

