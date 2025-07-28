// common/registers.inc
// ====================
//
// SNES Register Defines.
//
// Names and flags taken from Anomie's SNES Documents
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


// a Data Bank that can access the registers.
constant REGISTER_DB = 0x80


// Tile Map Format
//
// cccccccc
// vhopppcc
//
// cccccccccc = character of tile
// ppp        = palette
// o          = order
// h          = horizontal flip
// v          = vertical flip
namespace Tilemap {
    namespace char {
        constant mask = %0000001111111111
        constant shift = 0
    }
    namespace palette {
        constant mask = %0001110000000000
        constant shift = 10
    }

    constant order = %0010000000000000
    constant hFlip = %0100000000000000
    constant vFlip = %1000000000000000
}

namespace TilemapH {
    namespace char {
        constant mask = %00000011
        constant shift = 0
    }
    namespace palette {
        constant mask = %00011100
        constant shift = 10
    }

    constant order = %00100000
    constant hFlip = %01000000
    constant vFlip = %10000000
}

// Palette format
//
// -bbbbbgggggrrrrr
//
// bbbbb = blue value
// ggggg = green value
// rrrrr = red value
namespace Palette {
    namespace blue {
        constant mask = %0111110000000000
        constant shift = 10
    }
    namespace green {
        constant mask = %0000001111100000
        constant shift = 5
    }
    namespace red {
        constant mask = %0000000000011111
        constant shift = 0
    }
}

expression ToPalette(r, g, b) = ((b & 31) << 10) | ((g & 31) << 5) | (r & 31)


// OAM Table 0 Format
//
// xxxxxxxx
// yyyyyyyy
// cccccccc
// vhoopppN
//
// xxxxxxxx = lower 8 bits of x position of object
// yyyyyyyy = y position of object
// cccccccc = character of object
// N        = Character Table to use(equivalent to 9th bit of c)
// ppp      = palette
// oo       = order
// h        = horizontal flip
// v        = vertical flip
namespace OamFormat {
    constant xPos = 0
    constant yPos = 1
    constant char = 2
    constant attr = 3

    constant size = 4

    constant charAttr = 2

    namespace attr {
        namespace palette {
            constant mask = %00001110
            constant shift = 1
        }

        namespace order {
            constant mask = %00110000
            constant shift = 4
        }

        constant table = %00000001
        constant hFlip = %01000000
        constant vFlip = %10000000
    }

    namespace charAttr {
        namespace char {
            constant mask = %0000000111111111
            constant shift = 0
        }
        namespace palette {
            constant palette = %0000111000000000
            constant shift = 9
        }
        namespace order {
            constant order = %0011000000000000
            constant shift = 12
        }
        constant table = %0000000100000000
        constant hflip = %0100000000000000
        constant vflip = %1000000000000000
    }
}



// Screen Display
//(write, any)
//
// f---bbbb
//
// f    = Force Blank
// bbbb = Screen Brightnes, 0 = Dull, 16 = max
constant INIDISP = 0x2100

namespace INIDISP {
    constant force = %10000000

    namespace brightness {
        constant mask = %00001111
        constant shift = 0
        constant full = 15
    }
}



// Object Size and Chr Address
//(write, v-blank)
//
// sssnnbbb
//
// sss = Object Size
// nn  = Name Select
// bbb = Name Base Address(addr>>14)
constant OBSEL = 0x2101

namespace OBSEL {
    namespace size {
        constant mask = %11100000

        constant s8_16 = %00000000
        constant s8_32 = %00100000
        constant s8_64 = %01000000
        constant s16_32 = %01100000
        constant s16_64 = %10000000
        constant s32_64 = %10100000
        constant s16x32_32x64 = %11000000
        constant s16x32_32x32 = %11100000
    }

    namespace name {
        constant mask = %00011000
        constant shift = 3
    }
    namespace base {
        constant mask = %00000111
        constant shift = 0

        /// Object Tile Address segments sizes.
        /// The VRAM address used is 16384 * `bbb`
        constant walign = 8192
        constant align = 16384
    }
}



// OAM Address and Priority
//(write, v-blank)
//
// p------b aaaaaaaa
//
// p        = Obj priority bit
// b        = Table access(0 = Low Table, 1 = High Table)
// aaaaaaaa = Address
constant OAMADDL = 0x2102
constant OAMADDH = 0x2103

namespace OAMADDH {
    constant lowTable = %00000000
    constant hiTable = %00000001

    /// Sprite Priority Select
    /// When set, `OAMADDL` handles sprite priority.
    constant priority = %10000000
}

// Word Access of OAMADDL/OAMADDH
//(write, word, v-blank)
constant OAMADD = 0x2102

namespace OAMADD {
    namespace address {
        constant mask = %0000000011111111
        constant shift = 0
    }
    constant hiTable = %0000000100000000
    constant priority = %1000000000000000
}



// OAM Data Write
//(write, write twice, v-blank)
constant OAMDATA = 0x2104



// BG Mode and Character Size
//(write, h-blank, v-blank)
//
// DCBAemmm
//
// A/B/C/D = Character size for BG1 - BG4(1 = 16x16 tiles, 0 = 8x8 tiles)
// e       = Mode 1 BG3 Priority
// mmm     = BG Mode
constant BGMODE = 0x2105

namespace BGMODE {
    namespace mode {
        constant mask = %00000111
    }
    constant mode0 = %00000000
    constant mode1 = %00000001
    constant mode1Bg3 = %00001001
    constant mode2 = %00000010
    constant mode3 = %00000011
    constant mode4 = %00000100
    constant mode5 = %00000101
    constant mode6 = %00000110
    constant mode7 = %00000111

    namespace size {
        constant mask = %11110000

        constant bg1 = %00010000
        constant bg2 = %00100000
        constant bg3 = %01000000
        constant bg4 = %10000000
    }
}


// Screen Pixelation
//(write, h-blank, v-blank)
//
// xxxxDCBA
//
// A/B/C/D = Affect BG1 - BG4
// xxxx    = pixel size(0 to 16)
constant MOSAIC = 0x2106

namespace MOSAIC {
    namespace affect {
        constant mask = %00001111

        constant bg1 = %00000001
        constant bg2 = %00000010
        constant bg3 = %00000100
        constant bg4 = %00001000
    }

    namespace size {
        constant mask = %11110000
        constant shift = 4
    }
}



// BG Tilemap Address and Size
//(write, v-blank)
//
// aaaaaass
//
// aaaaaa = Tilemap base address in VRAM(Addr>>10)
// ss     = Tilemap size
constant BG1SC = 0x2107
constant BG2SC = 0x2108
constant BG3SC = 0x2109
constant BG4SC = 0x210A

namespace BGXSC {
    namespace base {
        constant mask = %11111100
        constant shift = 2

        /// Address segments sizes.
        /// The VRAM address used is 2048 * `aaaaaa`
        constant walign = 1024
        constant align = 2048
    }

    namespace map {
        constant mask = %00000011

        constant s32x32 = %00000000
        constant s64x32 = %00000001
        constant s32x64 = %00000010
        constant s64x64 = %00000011
    }
}



// BG1/2 Character Address
//(write, v-blank)
//
// bbbbaaaa
//
// aaaa - Base address for BG1(Addr>>13)
// bbbb - Base address for BG2(Addr>>13)
constant BG12NBA = 0x210B

namespace BG12NBA {
    namespace bg1 {
        constant mask = %00001111
        constant shift = 0
    }

    namespace bg2 {
        constant mask = %11110000
        constant shift = 4
    }

    /// Address segments sizes.
    /// The VRAM address used is 8192 * `aaaa`
    constant walign = 4096
    constant align = 8192
}


// BG 3/4 Character Address
//(write, v-blank)
//
// bbbbaaaa
//
// aaaa - Base address for BG3(Addr>>13)
// bbbb - Base address for BG4(Addr>>13)
constant BG34NBA = 0x210C

namespace BG34NBA {
    namespace bg3 {
        constant mask = %00001111
        constant shift = 0
    }

    namespace bg4 {
        constant mask = %11110000
        constant shift = 4
    }

    /// Address segments sizes.
    /// The VRAM address used is 8192 * `aaaa`
    constant walign = 4096
    constant align = 8192
}



// BG Horizontal Scroll
//(write, write twice, h-blank, v-blank)
//
// ------xx xxxxxxxx
//
// x = BG Horizontal offset
constant BG1HOFS = 0x210D
constant BG2HOFS = 0x210F
constant BG3HOFS = 0x2111
constant BG4HOFS = 0x2113

// Mode 7 BG Horizontal Scroll
//(write, write twice, h-blank, v-blank)
//
// ---mmmmm mmmmmmmm
//
// m = Mode 7 BG Offset(M7HOFS only)
constant M7HOFS = 0x210D



// BG Vertical Scroll
//(write, write twice, h-blank, v-blank)
//
// ------yy yyyyyyyy
//
// y = BG Vertical offset
constant BG1VOFS = 0x210E
constant BG2VOFS = 0x2110
constant BG3VOFS = 0x2112
constant BG4VOFS = 0x2114

// Mode 7 BG Vertical Scroll
//(write, write twice, h-blank, v-blank)
//
// ---mmmmm mmmmmmmm
//
// m = Mode 7 BG1 Offset(M7VOFS only)
constant M7VOFS = 0x210E



// Video Port Control
//(write, v-blank)
//
// i---mmii
//
// i  = Address increment mode
// ii = Address increment amount
// mm = Address remapping
//
// According to the manual only `ii` or `mm` should be set.
constant VMAIN = 0x2115

namespace VMAIN {
    namespace increment {
        constant mask = %00000011

        constant by1 = %00000000
        constant by32 = %00000001
        constant by128 = %00000010
    }

    namespace remap {
        constant mask = %00001100

        constant for32 = %00000100
        constant for64 = %00001000
        constant for128 = %00001100
    }

    namespace incrementMode {
        constant mask = %10000000

        constant low = %00000000
        constant high = %10000000
    }
}



// VRAM Word Address
//(write, word, v-blank)
constant VMADD = 0x2116
constant VMADDL = 0x2116
constant VMADDH = 0x2117



// VRAM Data Write
//(write, word, v-blank)
constant VMDATA = 0x2118
constant VMDATAL = 0x2118
constant VMDATAH = 0x2119



// Mode 7 Select
//(write, v-blank)
//
// oo----vh
//
// oo = Out of screen area
//        00 - Screen Repeat
//        01 - Color 0
//        10 - Tile 0
// h  = horizontal flip
// v  = vertical flip
constant M7SEL = 0x211A

namespace M7SEL {
    namespace outOfScreen {
        constant mask = %11000000

        constant repeat = %00000000
        constant color0 = %10000000
        constant tile0 = %11000000
    }

    constant hFlip = %00000001
    constant vFlip = %00000010
}



// Mode 7 Matrix
//(write, write twice, 1:7:8 signed fixed point, h-blank, v-blank)
//
// The Matrix transformation is:
//    [ X ]   [ M7A  M7B ]   [ SX + M7HOFS - M7X ]   [ M7X ]
//    [ Y ] = [ M7C  M7D ] * [ SY + M7VOFS - M7Y ] + [ M7Y ]
//
// Where SX and SY are screen coordinates
constant M7A = 0x211B
constant M7B = 0x211C
constant M7C = 0x211D
constant M7D = 0x211E

// Mode 7 Center
//(write, write twice, 13 bit signed integer, h-blank, v-blank)
constant M7X = 0x211F
constant M7Y = 0x2120



// CGRAM Address
//(write, h-blank, v-blank)
constant CGADD = 0x2121

namespace CGADD {
    /// Starting color of the OAM palette
    constant objPalette = 128
}



// CGRAM Data Write
//(write, write twice, h-blank, v-blank)
//
// -bbbbbgg gggrrrrr
constant CGDATA = 0x2122



// Window Mask Settings
//(write, h-blank, v-blank)
//
// For each nibble:
//   EIei
//
//   e = Enable Window 1
//   i = Window 1 In/Out(1 = Inside, 0 = Outside)
//   E = Enable Window 2
//   I = Window 2 In/Out(1 = Inside, 0 = Outside)
//
constant W12SEL = 0x2123
constant W34SEL = 0x2124
constant WOBJSEL = 0x2125

namespace WSEL {
    namespace win1 {
        constant enable = %0010
        constant inside = %0000
        constant outside = %0001
    }

    namespace win2 {
        constant enable = %1000
        constant inside = %0000
        constant outside = %0100
    }
}

namespace W12SEL {
    namespace bg1 {
        constant mask = %00001111
        constant shift = 0
    }
    namespace bg2 {
        constant mask = %11110000
        constant shift = 4
    }
}

namespace W34SEL {
    namespace bg3 {
        constant mask = %00001111
        constant shift = 0
    }
    namespace bg4 {
        constant mask = %11110000
        constant shift = 4
    }
}

namespace WOBJSEL {
    namespace obj {
        constant mask = %00001111
        constant shift = 0
    }
    namespace color {
        constant mask = %11110000
        constant shift = 4
    }
}



// Window 1 Left Position
//(write, h-blank, v-blank)
constant WH0 = 0x2126

// Window 1 Right Position
//(write, h-blank, v-blank)
constant WH1 = 0x2127

// Window 2 Left Position
//(write, h-blank, v-blank)
constant WH2 = 0x2128

// Window 2 Right Position
//(write, h-blank, v-blank)
constant WH3 = 0x2129



// Window mask logic for Backgrounds
//(write, h-blank, v-blank)
//
// 44332211
//
// For each bit pair:
//   00 = OR
//   01 = AND
//   10 = XOR
//   11 = XNOR
constant WBGLOG = 0x212A

namespace WBGLOG {
    namespace bg1 {
        constant mask = %00000011
        constant shift = 0
    }
    namespace bg2 {
        constant mask = %00001100
        constant shift = 2
    }
    namespace bg3 {
        constant mask = %00110000
        constant shift = 4
    }
    namespace bg4 {
        constant mask = %11000000
        constant shift = 6
    }

    namespace logic {
        constant or = %00
        constant and = %01
        constant xor = %10
        constant xnor = %11
    }
}


// Window mask logic for Objects and Color Window
//(write, h-blank, v-blank)
//
// ----ccoo
//
// For each bit pair:
//   00 = OR
//   01 = AND
//   10 = XOR
//   11 = XNOR
constant WOBJLOG = 0x212B

namespace WOBJLOG {
    namespace obj {
        constant mask = %00000011
        constant shift = 0
    }
    namespace color {
        constant mask = %00001100
        constant shift = 2
    }

    namespace logic {
        constant or = %00
        constant and = %01
        constant xor = %10
        constant xnor = %11
    }
}



// Main Screen Designation
//(write, h-blank, v-blank)
//
// ----o4321
//
// 1/2/3/4/o - enable BG1 - BG4 & Objects for the main screen
constant TM = 0x212C

namespace TM {
    constant bg1 = %00000001
    constant bg2 = %00000010
    constant bg3 = %00000100
    constant bg4 = %00001000
    constant obj = %00010000
}



// Sub Screen Designation
//(write, h-blank, v-blank)
//
// ----o4321
//
// 1/2/3/4/o - enable BG1 - BG4 & Objects for the sub screen
constant TS = 0x212D

namespace TS {
    constant bg1 = %00000001
    constant bg2 = %00000010
    constant bg3 = %00000100
    constant bg4 = %00001000
    constant obj = %00010000
}



// Window Mask Designation for the Main Screen
//(write, h-blank, v-blank)
//
// ----o4321
//
// 1/2/3/4/o - enable window masking for BG1-BG4 & Objects on the main screen
constant TMW = 0x212E

namespace TMW {
    constant bg1 = %00000001
    constant bg2 = %00000010
    constant bg3 = %00000100
    constant bg4 = %00001000
    constant obj = %00010000
}



// Window Mask Designation for the Sub Screen
//(write, h-blank, v-blank)
//
// ----o4321
//
// 1/2/3/4/o - enable window masking for BG1 - BG4 & Objects for the sub screen
constant TSW = 0x212F

namespace TSW {
    constant bg1 = %00000001
    constant bg2 = %00000010
    constant bg3 = %00000100
    constant bg4 = %00001000
    constant obj = %00010000
}



// Color Addition Select
//(write, h-blank, v-blank)
//
// ccmm--sd
//
// cc = clip color
//       00 - Never
//       01 - Outside color window
//       10 - Inside color window
//       11 - always
//
// mm = prevent color math
//       00 - Never
//       01 - Outside color window
//       10 - Inside color window
//       11 - always
//
// s  = Add subscreen enable
// d  = Direct color mode for 256 color BG
constant CGWSEL = 0x2130

namespace CGWSEL {
    namespace clip {
        constant mask = %11000000

        constant never = %00000000
        constant outside = %01000000
        constant inside = %10000000
        constant always = %11000000
    }
    namespace prevent {
        constant mask = %00110000

        constant never = %00000000
        constant outside = %00010000
        constant inside = %00100000
        constant always = %00110000
    }

    constant addSubscreen = %00000010
    constant directColor = %00000001
}



// Color Math Designation
//(write, h-blank, v-blank)
//
// shbo4321
//
// s           = add/subtract colors(0 = add, 1 = subtract)
// h           = Half color math
// 4/3/2/1/o/b = Enable color math for BG1 - BG4, Objects and Backdrop
constant CGADSUB = 0x2131

namespace CGADSUB {
    namespace color {
        constant add = %00000000
        constant subtract = %10000000
    }
    constant halfColor = %01000000

    namespace enable {
        constant mask = %00111111

        constant bg1 = %00000001
        constant bg2 = %00000010
        constant bg3 = %00000100
        constant bg4 = %00001000
        constant obj = %00010000
        constant backdrop = %00100000
    }
}



// Fixed Color Data
//(write, h-blank, v-blank)
//
// bgrccccc
//
// b/g/r = color plane(s)
// ccccc = color intensity
constant COLDATA = 0x2132

namespace COLDATA {
    namespace plane {
        constant all = %11100000
        constant blue = %10000000
        constant green = %01000000
        constant red = %00100000
    }
    namespace intensity {
        constant mask = %00011111
        constant shift = 0
    }
}



// Screen Mode/ Video Select
//(write, h-blank, v-blank)
//
// se--poIi
//
// s = External Sync
// e = Enable Mode 7 EXTBG
// p = Enable Psudeo-hires mode
// o = Enable overscan
// I = Oject Interlace
// i = Screen Interlace
constant SETINI = 0x2133

namespace SETINI {
    constant extsync = %10000000
    constant extbg = %01000000
    constant psuedoHires = %00001000
    constant overscan = %00000100
    constant objInterlace = %00000010
    constant screenInterlace = %00000001
}



// Multiplication Result
//(read, long, h-blank, v-blank)
//
// Two's compliment product of 16 bit `M7A` and the
// most recent byte written to `M7B`.
//
// Should not be read during Mode 7 rendering.
constant MPY = 0x2134
constant MPYL = 0x2134
constant MPYM = 0x2135
constant MPYH = 0x2136



// Software Latch for H/V Counter
//(read, any)
//
// See `OPHCT` and `OPVCT`
constant SLHV = 0x2137



// Data for OAM Read
//(read, read twice, h-blank, v-blank)
constant OAMDATAREAD = 0x2138



// VRAM Data Read
//(read, word, v-blank)
constant VMDATAREAD = 0x2139
constant VMDATALREAD = 0x2139
constant VMDATAHREAD = 0x213A



// CGRAM Data Read
//(read, v-blank)
constant CGDATAREAD = 0x213B



// Horizontal Scanline Location
//(read, read twice, any)
//
// --------x xxxxxxxx
//
// Notes:
//   * Latched on `SLHV` read or Pin 6 of Port 2(bit 6 `WRIO`) falling trigger.
//   * Range 0 to 339
//   * Visible range: 22 - 277
//   * Bits 1-7 of high byte are open bus. Should mask bits 0-8.
//   * High/Low order reset on `STAT78` read.
constant OPHCT = 0x213C

// Vertical Scanline Location
//(read, read twice, any)
//
// --------y yyyyyyyy
//
// Notes:
//   * Latched on `SLHV` read or Pin 6 of Port 2(bit 6 `WRIO`) falling trigger.
//   * Range 0 to 261 NTSC(+1 when interlacing)
//   * Range 0 to 312 PAL(+1 when interlacing)
//   * Visible range 1 - 224, 1 - 239? on overscan.
//   * Bits 1-7 of high byte are open bus. Should mask bits 0-8.
//   * High/Low order reset on `STAT78` read.
constant OPVCT = 0x213D



// PPU 5c77 Staus Flag and Version
//(read, any)
//
// trm-vvvv
//
// t    = time over flag(more than 34 sprite tiles on a scanline)
// r    = Range overflow(more than 32 sprites on a scanline)
// m    = master/slave select. Unknown function
// vvvv = 5c77 chip number.
constant STAT77 = 0x213E

namespace STAT77 {
    namespace version {
        constant mask = %00001111
        constant shift = 0
    }

    constant time = %10000000
    constant range = %01000000
}



// PPU 5c88 Staus Flag and Version
//(read, any)
//
// fl-pvvvv
//
// f    = Interlace flag(toffled every v-blank)
// l    = External Latch Flag(set when PPU counters latched)
// p    = NTSC/PAL mode(1 = PAL, 0 = NTSC)
// vvvv = 5c78 chip number.
constant STAT78 = 0x213F

namespace STAT78 {
    namespace version {
        constant mask = %00001111
        constant shift = 0
    }

    constant pal = %00010000
    constant interlace = %10000000
    constant extLatch = %01000000
}



// APU I/O Register
//(read/write, any)
constant APUIO0 = 0x2140
constant APUIO1 = 0x2141
constant APUIO2 = 0x2142
constant APUIO3 = 0x2143



// WRAM Data Read/Write
//(read/write, any)
//
// Will not work with a DMA transfer from WRAM to WRAM.
constant WMDATA = 0x2180

// WRAM Address
//(write, long, any)
//
// -------x xxxxxxxx xxxxxxxx
constant WMADD = 0x2181
constant WMADDL = 0x2181
constant WMADDM = 0x2182
constant WMADDH = 0x2183



// NES-style Joypad Access Port 1
//(read/write, any)
//
// Read:  ------ca
// Write: -------l
//
// l = latch line of both ports
// a = data 1 line
// b = data 2 Line
constant JOYSER0 = 0x4016

// write
namespace JOYSER0 {
    constant latch = %00000001
}
// read
namespace JOYSER0 {
    constant data1 = %00000001
    constant data2 = %00000010
}



// NES-style Joypad Access Port 2
//(read, any)
//
// ---111db
//
// b = data 1 line
// d = data 2 line
// 1 = Always set
constant JOYSER1 = 0x4017

namespace JOYSER1 {
    constant data1 = %00000001
    constant data2 = %00000010
}



// Interrupt Enable Flags
//(write, any)
//
// n-vh---a
//
// n  = Enable V-Blank(NMI)
// vh = Enable IRQ
//       00 - Disabled
//       01 - On H-Counter(`HTIME`)
//       10 - On V-Counter(`VTIME`)
//       11 - On V-Counter and H-Counter(`VTIME` and `HTIME`)
// a  = Auto read joypad
constant NMITIMEN = 0x4200

namespace NMITIMEN {
    constant vBlank   = %10000000
    constant vCounter = %00100000
    constant hCounter = %00010000
    constant autoJoy  = %00000001
}



// Programmable I/O Port
//(write, any)
//
// ab------
//
// a = Pin 6 of Port 1
// b = Pin 6 of Port 2
//
// H/V Counters(`OPHCT` and `OPVCT` ) will be latched on lowering
// trigger of Port 2(Transition from 1 to 0).
constant WRIO = 0x4201

namespace WRIO {
    constant port1 = %10000000
    constant port2 = %01000000
}



// Multiplicand A
//(write, any)
constant WRMPYA = 0x4202

// Multiplicand B
//(write, any)
//
// After 8 CPU cycles `RDMPY` will contain the unsigned product of `WRMPYA` and `WRMPYB`.
constant WRMPYB = 0x4203



// Dividend C
//(write, word, any)
constant WRDIV = 0x4204
constant WRDIVL = 0x4204
constant WRDIVH = 0x4205

// Divisor B
//(write, any)
//
// After 16 CPU cycles `RDDIV` will contain the unsigned quotient of `WRDIV` and `WRDIVB`.
// The remainder of the divison is stored in `RDMPY`.
constant WRDIVB = 0x4206



// H Timer
//(write, word, any)
//
// ------h hhhhhhhh
//
// hhhhhhhhh = horizontal counter target(Range 0 - 339)
//
// See `MNITIMEN` for details.
constant HTIME = 0x4207
constant HTIMEL = 0x4207
constant HTIMEH = 0x4208

// V Timer
//(write, word, any)
//
// ------v vvvvvvvv
//
// vvvvvvvvvv = vertical counter target(Range 0 - 261 NTSC and 0 - 311 PAL)
//
// See `MNITIMEN` for details.
constant VTIME = 0x4209
constant VTIMEL = 0x4209
constant VTIMEH = 0x420A



// DMA Enable
//(write, any)
//
// 76543210
//
// 7/6/5/4/3/2/1 = Enable DMA channel.
//
// When a DMA is activated the CPU will be paused until all DMAs are completed.
constant MDMAEN = 0x420B

namespace MDMAEN {
    constant dma0 = %00000001
    constant dma1 = %00000010
    constant dma2 = %00000100
    constant dma3 = %00001000
    constant dma4 = %00010000
    constant dma5 = %00100000
    constant dma6 = %01000000
    constant dma7 = %10000000
}

// HDMA Enable
//(write, any)
//
// 76543210
//
// 7/6/5/4/3/2/1 = Enable HDMA channel.
constant HDMAEN = 0x420C

namespace HDMAEN {
    constant dma0 = %00000001
    constant dma1 = %00000010
    constant dma2 = %00000100
    constant dma3 = %00001000
    constant dma4 = %00010000
    constant dma5 = %00100000
    constant dma6 = %01000000
    constant dma7 = %10000000
}



// ROM Access Speed
//(write, any)
//
// -------r
//
// r = FastROM select(1 = FastROM, 0 = SlowROM)
constant MEMSEL = 0x420D

namespace MEMSEL {
    constant fastrom = %00000001
    constant slowrom = %00000000
}



// NMI Flag and 5A22 Version
//(read, any)
//
// n---vvvv
//
// n    = MNI Flag. Set on start of V-Blank, cleaered on read
//        or at end of V-Blank.
// vvvv = 5A22 Chip version number
//
// This register should be read during the MNI Handler.
constant RDNMI = 0x4210

namespace RDNMI {
    constant nmi = %10000000

    namespace verison {
        constant mask = %00001111
        constant shift = 0
    }
}



// IRQ Flag
//(read, any)
//
// i-------
//
// i = IRQ Flag. Set just after IRQ fires, cleared on read.
//
// This register should be read during the IRQ Handler.
constant TIMEUP = 0x4211



// PPU Status
//(read, any)
//
// vh-----a
//
// v = V-Blank Flag. Set during V-Blank
// h = H-Blank Flag. Set during H-Blank
// a = Auto Joypad read status. Set while reading, cleared when completed.
constant HVBJOY = 0x4212

namespace HVBJOY {
    constant vBlank = %10000000
    constant hBlank = %01000000
    constant autoJoy = %00000001
}



// Programmable I/O Port(in-port)
//
// ab------
//
// a = Pin 6 of Port 1
// b = Pin 6 of Port 2
constant RDIO = 0x4213

namespace RDIO {
    constant port1 = %10000000
    constant port2 = %01000000
}



// Quotient of Divide Result
//(read, word, any)
//
// Set 16 CPU cycles after `WRDIVB` write
constant RDDIV = 0x4214
constant RDDIVL = 0x4214
constant RDDIVH = 0x4215



// Multiplication Product of Divide Remainder
//(read, word, any)
//
// Set 16 CPU cycles after `WRDIVB` write or 8 CPU cycles after `WRMPYB` write.
constant RDMPY = 0x4216
constant RDMPYL = 0x4216
constant RDMPYH = 0x4217



// Autojoy Controller Ports
//(read, word, any)
//
// Should not be read when bit 0 of `HVBJOY` set.
constant JOY1 = 0x4218
constant JOY2 = 0x421A
constant JOY3 = 0x421C
constant JOY4 = 0x421E

constant JOY1L = 0x4218
constant JOY1H = 0x4219
constant JOY2L = 0x421A
constant JOY2H = 0x421B
constant JOY3L = 0x421C
constant JOY3H = 0x421D
constant JOY4L = 0x421E
constant JOY4H = 0x421F

namespace JOY {
    constant b = %1000000000000000
    constant y = %0100000000000000
    constant select = %0010000000000000
    constant start = %0001000000000000
    constant up = %0000100000000000
    constant down = %0000010000000000
    constant left = %0000001000000000
    constant right = %0000000100000000
    constant a = %0000000010000000
    constant x = %0000000001000000
    constant l = %0000000000100000
    constant r = %0000000000010000

    constant buttons = %1100000011000000
    constant dpad = %0000111100000000
    constant shoulder = %0000000000110000

    namespace type {
        constant mask = %0000000000001111

        constant controller = %0000
        constant mouse = %0001
        constant konamiJustifer = %1110
    }
}

namespace JOYH {
    constant b = %10000000
    constant y = %01000000
    constant select = %00100000
    constant start = %00010000
    constant up = %00001000
    constant down = %00000100
    constant left = %00000010
    constant right = %00000001

    constant dpad = %00001111
}
namespace JOYL {
    constant a = %10000000
    constant x = %01000000
    constant l = %00100000
    constant r = %00010000

    namespace type {
        constant mask = %00001111

        constant controller = %0000
        constant mouse = %0001
        constant konamiJustifer = %1110
    }
}



// DMA Control
//(read, write, any)
//
// da-ifttt
//
// d    = Transfer direction(0 = CPU -> PPU, 1 = PPU -> CPU)
// a    = HDMA Addressing Mode(0 = absolute, 1 = indirect). HDMA Only
// i    = DMA Address Increment(0 = Increment, 1 = Decrement). DMA Only
// f    = DMA Fixed Transfer(1 = Address will not change). DMA Only.
// ttt  = Transfer Mode
//          000 - 1 register,  write once( r                )
//          001 - 2 registers, write once( r, r+1           )
//          010 - 1 register,  write twice( r, r             )
//          011 - 2 registers, write twice each( r, r,   r+1, r+1 )
//          100 - 4 registers, write once( r, r+1, r+2, r+3 )
//          101 - 2 register,  write twice alt  - undocumented
//          110 - 1 register,  write twice      - undocumented
//          111 - 2 registers, write twice each - undocumented
//
constant DMAP0 = 0x4300
constant DMAP1 = 0x4310
constant DMAP2 = 0x4320
constant DMAP3 = 0x4330
constant DMAP4 = 0x4340
constant DMAP5 = 0x4350
constant DMAP6 = 0x4360
constant DMAP7 = 0x4370

namespace DMAP {
    namespace transfer {
        constant mask = %00000111

        constant one = %000
        constant two = %001
        constant writeTwice = %010
        constant readTwice = %010
        constant twoWriteTwice = %011
        constant four = %100
    }
    namespace direction {
        constant increment = %00000000
        constant decrement = %00010000
    }
    namespace addressing {
        constant absolute = %00000000
        constant indirect = %01000000
    }
    namespace direction {
        constant toPpu = %00000000
        constant toCpu = %10000000
    }

    constant fixed = %00001000
}



// DMA Destination Register
//(read, write, any)
//
// Bus B Address(PPU Memmory Address, maps to $00:21xx)
constant BBAD0 = 0x4301
constant BBAD1 = 0x4311
constant BBAD2 = 0x4321
constant BBAD3 = 0x4331
constant BBAD4 = 0x4341
constant BBAD5 = 0x4351
constant BBAD6 = 0x4361
constant BBAD7 = 0x4371



// DMA Source Address
//(read, write, long, any)
//
// Bus A Address(CPU Memory Address)
//
// DMA:
//  * Starting address for DMA Transfer.
//  * Address will be incremented/decremented
//  * Bank will not change during DMA Transfer.
//
// HDMA:
//  * Table address for HDMA Transfer.
constant A1T0 = 0x4302
constant A1T1 = 0x4312
constant A1T2 = 0x4322
constant A1T3 = 0x4332
constant A1T4 = 0x4342
constant A1T5 = 0x4352
constant A1T6 = 0x4362
constant A1T7 = 0x4372

constant A1T0L = 0x4302
constant A1T0H = 0x4303
constant A1B0 = 0x4304
constant A1T1L = 0x4312
constant A1T1H = 0x4313
constant A1B1 = 0x4314
constant A1T2L = 0x4322
constant A1T2H = 0x4323
constant A1B2 = 0x4324
constant A1T3L = 0x4332
constant A1T3H = 0x4333
constant A1B3 = 0x4334
constant A1T4L = 0x4342
constant A1T4H = 0x4343
constant A1B4 = 0x4344
constant A1T5L = 0x4352
constant A1T5H = 0x4353
constant A1B5 = 0x4354
constant A1T6L = 0x4362
constant A1T6H = 0x4363
constant A1B6 = 0x4364
constant A1T7L = 0x4372
constant A1T7H = 0x4373
constant A1B7 = 0x4374



// DMA Size / HDMA Indirect Address
//(read, write, word, any)
//
// DMA:
//  * 16 Bits
//  * Number of bytes to transfer
//  * Decremented during DMA.
//  * If 0 then 0x10000 bytes will be transfered.
//
// HDMA:
//  * Holding space for indirect addressing mode
//  * These registers should not be set by the CPU
constant DAS0 = 0x4305
constant DAS1 = 0x4315
constant DAS2 = 0x4325
constant DAS3 = 0x4335
constant DAS4 = 0x4345
constant DAS5 = 0x4355
constant DAS6 = 0x4365
constant DAS7 = 0x4375


constant DAS0L = 0x4305
constant DAS0H = 0x4306
constant DAS1L = 0x4315
constant DAS1H = 0x4316
constant DAS2L = 0x4325
constant DAS2H = 0x4326
constant DAS3L = 0x4335
constant DAS3H = 0x4336
constant DAS4L = 0x4345
constant DAS4H = 0x4346
constant DAS5L = 0x4355
constant DAS5H = 0x4356
constant DAS6L = 0x4365
constant DAS6H = 0x4366
constant DAS7L = 0x4375
constant DAS7H = 0x4376



// HDMA Indirect Address Bank
//(read, write, any)
//
// Used with HDMA only, combined with `DAS0` to form the effective address.
constant DASB0 = 0x4307
constant DASB1 = 0x4317
constant DASB2 = 0x4327
constant DASB3 = 0x4337
constant DASB4 = 0x4347
constant DASB5 = 0x4357
constant DASB6 = 0x4367
constant DASB7 = 0x4377



// HDMA Table Address
//(read, write, word, any)
//
// Used with HDMA only.
//
// Only modify this register if starting HDMA mid frame.
constant A2A0 = 0x4308
constant A2A1 = 0x4318
constant A2A2 = 0x4328
constant A2A3 = 0x4338
constant A2A4 = 0x4348
constant A2A5 = 0x4358
constant A2A6 = 0x4368
constant A2A7 = 0x4378

constant A2A0L = 0x4308
constant A2A0H = 0x4309
constant A2A1L = 0x4318
constant A2A1H = 0x4319
constant A2A2L = 0x4328
constant A2A2H = 0x4329
constant A2A3L = 0x4338
constant A2A3H = 0x4339
constant A2A4L = 0x4348
constant A2A4H = 0x4349
constant A2A5L = 0x4358
constant A2A5H = 0x4359
constant A2A6L = 0x4368
constant A2A6H = 0x4369
constant A2A7L = 0x4378
constant A2A7H = 0x4379



// HDMA Line Counter
//(read, write, any)
//
// rccccccc
//
// r       = Repeat Select(1 = HDMA every line, 0
// ccccccc = Line Count
//
// Used with HDMA only.
//
// Only modify this register if starting HDMA mid frame.
constant NLTR0 = 0x430A
constant NLTR1 = 0x431A
constant NLTR2 = 0x432A
constant NLTR3 = 0x433A
constant NLTR4 = 0x434A
constant NLTR5 = 0x435A
constant NLTR6 = 0x436A
constant NLTR7 = 0x437A

namespace NLTR {
    constant repeat = %10000000

    namespace count {
        constant mask = %01111111
        constant shift = 0
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

