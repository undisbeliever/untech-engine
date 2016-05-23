// Test ROM blocks

arch snes.cpu

define MEMORY_MAP(LOROM)
define ROM_SIZE(1)

include "../../../src/common/assert.inc"
include "../../../src/common/memory.inc"

createCodeBlock(code,  0xc08000, 0xc0ffaf)
createDataBlock(rom1,  0xc18000, 0xc1ffff)
createDataBlock(rom2,  0xc28000, 0xc2efff)

createDataBlock(fixedsize, 0xc2f000, 0xc2ffff)

code(code)
assert(pc() == 0xc08000)

rodata(rom1)
assert(pc() == 0xc18000)

rodata(rom2)
assert(pc() == 0xc28000)

scope testInScope {
    code(code)
    assert(pc() == 0xc08000)
        nop

    rodata(rom1)
    assert(pc() == 0xc18000)
        dw  0x1337
        dw  0x1337
    assert(pc() == 0xc18004)
}

code(code)
assert(pc() == 0xc08001)
    nop

code()
assert(pc() == 0xc08002)

rodata(rom1)
assert(pc() == 0xc18004)


rodata(fixedsize)
fill 0x1000, 0xff
code()

// vim: ft=asm ts=4 sw=4 et:

