// Try and create two Data blocks that overlap each other

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/memory.inc"

createDataBlock(rom0,  0x808000, 0x80ffaf)
createDataBlock(rom1,  0x81c000, 0x81ffff)
createDataBlock(rom2,  0x818000, 0x81cfff)  // ERROR
createDataBlock(rom3,  0x820000, 0x82ffff)

