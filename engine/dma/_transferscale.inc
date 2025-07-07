// dma/_transferscale.inc
// ======================
//
// Transfer scale settings for the DMA module.
//
//
// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Dma {

constant TRANSFER_SCALE = 32


// ::TODO get proper numbers::
constant TRANSFERS_PER_NTSC_FRAME = 4000 / TRANSFER_SCALE
constant TRANSFERS_PER_PAL_FRAME = 5000 / TRANSFER_SCALE

assert(TRANSFERS_PER_PAL_FRAME < 0x100)


// Calculate the transfersLeft offset for a Dma VBlank transfer.
//
// PARAM: BS - data block size
// PARAM: VC - VBlank cycles (excluding DMA transfer)
expression CalcDmaTime(BS, VC) = (BS + VC + Dma.TRANSFER_SCALE - 1) / Dma.TRANSFER_SCALE

// Calculate the transfersLeft offset for a hard-coded VBlank transfer.
//
// PARAM: VC - VBlank cycles
expression CalcDmaTime(VC) = (VC + Dma.TRANSFER_SCALE - 1) / Dma.TRANSFER_SCALE

}

// vim: ft=bass-65816 ts=4 sw=4 et:

