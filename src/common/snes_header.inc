// common/snes_header.inc
// ======================
//
// Builds the SNES ROM header.
//
// The following need to be defined before including this file:
//
//  * MEMORY_MAP - set to one of the values in the MEMORY_MAP scope
//  * ROM_SIZE   - the ROM size in Mbits
//  * REGION     - the region of the ROM (must match the REGION scope)
//
// The following defines are optional, but must be defined before
// including this file.
//
//  * CART_TYPE  - the type of cart (see CART_TYPE scope, defaults to romOnly)
//  * ROM_SPEED  - the speed of the ROM (slow (default), fast)


// ::TODO Handle SRAM::

scope REGION {
    constant Japan(0x00)
    constant US(0x01)
    constant Europe(0x02)
    constant Scandinavia(0x03)
    constant Europe_french(0x06)
    constant Dutch(0x07)
    constant Spanish(0x08)
    constant German(0x09)
    constant Italian(0x0a)
    constant Chinese(0x0b)
    constant Korean(0x0d)
    constant Common(0x0e)
    constant Canada(0x0f)
    constant Brazil(0x10)
    constant Australia(0x11)
    constant otherX(0x12)
    constant otherY(0x13)
    constant otherZ(0x14)
}

scope CART_TYPE {
    constant romOnly(0)
    constant romRam(1)
    constant romSram(2)
}
if !{defined CART_TYPE} {
    define CART_TYPE(romOnly)
}

scope ROM_SPEED {
    constant fast(0x10)
    constant slow(0x00)
}
if !{defined ROM_SPEED} {
    define ROM_SPEED(slow)
}


scope SnesHeader {

if {ROM_SIZE} <= 1 {
    constant _HEADER_ROM_SIZE(0x7)
} else if {ROM_SIZE} <= 2 {
    constant _HEADER_ROM_SIZE(0x8)
} else if {ROM_SIZE} <= 4 {
    constant _HEADER_ROM_SIZE(0x9)
} else if {ROM_SIZE} <= 8 {
    constant _HEADER_ROM_SIZE(0xA)
} else if {ROM_SIZE} <= 16 {
    constant _HEADER_ROM_SIZE(0xB)
} else if {ROM_SIZE} <= 32 {
    constant _HEADER_ROM_SIZE(0xC)
} else if {ROM_SIZE} <= 64 {
    constant _HEADER_ROM_SIZE(0xD)
}
constant _HEADER_MAP_MODE(MEMORY_MAP.{MEMORY_MAP}|ROM_SPEED.{ROM_SPEED})

private.seek(0x00ffb0)
    db  "FF"                    // $ffbo - 2 Digit marker code
    db  "SNES"                  // $ffb2 - 4 Character game id
EmptyBytes:
    db  0, 0, 0, 0, 0, 0, 0     // $ffb6 - fixed value (7 bytes)

    db  0                       // $ffbd - expansion RAM size
    db  0                       // $ffbe - special version
    db  0                       // $ffbf - cartridge type sub-number

    fill 21, ' '
private.seek(0x00ffc0)
    db  {ROM_NAME}              // $ffc0 - ROM name (21 characters)
private.seek(0x00ffd5)
    db  _HEADER_MAP_MODE        // $ffd5 - map mode
    db  CART_TYPE.{CART_TYPE}   // $ffd6 - cartridge type
    db  _HEADER_ROM_SIZE        // $ffd7 - ROM size
    db  0                       // $ffd8 - RAM size
    db  REGION.{REGION}         // $ffd9 - Destination code
    db  0x33                    // $ffda - fixed value
    db  {VERSION}               // $ffdb - ROM version
    dw  0xaaaa                  // $ffdc - checksum compliment
    dw  0x5555                  // $ffde - checksum

// Native Interrupt Vectors
    dw 0                        // $ffe0
    dw 0                        // $ffe2
    dw CopHandler               // $ffe4 - native cop
    dw BreakHandler             // $ffe6 - native brk
    dw EmptyHandler             // $ffe8 - native abort
    dw NmiHandler               // $ffea - native nmi
    dw 0                        // $ffec
    dw IrqHandler               // $ffee - native irq

// Emulated Interrupt Vectors
    dw 0                        // $fff0
    dw 0                        // $fff2
    dw EmptyHandler             // $fff4 - emu cop
    dw 0                        // $fff6
    dw EmptyHandler             // $fff8 - emu abort
    dw EmptyHandler             // $fffa - emu nmi
    dw ResetHandler             // $fffc - emu reset
    dw 0                        // $fffe - emu irq
}

private.seek(0)

// vim: ft=bass-65816 ts=4 sw=4 et:

