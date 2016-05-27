// Test push/pop ROM blocks

arch snes.cpu

define MEMORY_MAP(LOROM)
define ROM_SIZE(2)

include "../../../src/common/assert.inc"
include "../../../src/common/memory.inc"

createCodeBlock(code,  0xc08000, 0xc0ffaf)
createDataBlock(rom1,  0xc18000, 0xc1ffff)
createDataBlock(rom2,  0xc28000, 0xc2ffff)
createDataBlock(rom3,  0xc38000, 0xc3ffff)
createDataBlock(rom4,  0xc48000, 0xc4ffff)

code(code)
assert(pc() == 0xc08000)

pushBlock()
assert({::romBlock.stackSize} == 1)

rodata(rom1)
assert(pc() == 0xc18000)

popBlock()
assert({::romBlock.stackSize} == 0)
assert(pc() == 0xc08000)


// test stack works correctly

pushBlock()
rodata(rom1)
pushBlock()

scope innerTest {
    assert({::romBlock.stackSize} == 2)

    rodata(rom2)

    popBlock()
    assert({::romBlock.stackSize} == 1)
    assert(pc() == 0xc18000)

    code()
    pushBlock()
    assert({::romBlock.stackSize} == 2)
    assert(pc() == 0xc08000)

    popBlock()
    assert({::romBlock.stackSize} == 1)
    assert(pc() == 0xc08000)

    rodata(rom3)
}

popBlock()
assert({::romBlock.stackSize} == 0)
assert(pc() == 0xc08000)

// vim: ft=asm ts=4 sw=4 et:

