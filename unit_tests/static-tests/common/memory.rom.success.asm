// Test ROM blocks

architecture wdc65816-strict

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/assert.inc"
include "../../../src/common/memory.inc"

createCodeBlock(code,  0xc08000, 0xc0ffaf)
createDataBlock(rom1,  0xc18000, 0xc1ffff)

createDataBlock(addrTable, 0xc28000, 0xc2efff)
createDataBlock(fixedsize, 0xc2f000, 0xc2ffff)

code(code)
assert(pc() == 0xc08000)

rodata(rom1)
assert(pc() == 0xc18000)

rodata(addrTable)
assert(pc() == 0xc28000)

namespace testInScope {
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


namespace AddrTableData {
    rodata(addrTable)

    constant N_ITEMS = 15
    constant ELEMENT_SIZE = 5

    variable n = 0
    while n < N_ITEMS {
        fill ELEMENT_SIZE, n
        n = n + 1
    }
}

rodata(fixedsize)
fill 0x1000, 0xff
code()


storeBlockStart(rom1Start, rom1)
assert(rom1Start == 0xc18000)

storeBlockStart(addrTableStart, addrTable)
assert(addrTableStart == 0xc28000)


finalizeMemory()


storeBlockUsed(codeUsed, code)
assert(codeUsed == 2)

storeBlockUsed(rom1Used, rom1)
assert(rom1Used == 4)

storeBlockUsed(addrTableUsed, addrTable)
assert(addrTableUsed == AddrTableData.N_ITEMS * AddrTableData.ELEMENT_SIZE)

storeBlockUsed(fixedsizeUsed, fixedsize)
assert(fixedsizeUsed == 0x1000)


storeBlockCount(addrTableCount, addrTable, AddrTableData.ELEMENT_SIZE)
assert(addrTableCount == AddrTableData.N_ITEMS)


// vim: ft=bass-65816 ts=4 sw=4 et:

