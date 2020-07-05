// dma/_transferscale.inc
// ======================
//
// Transfer scale settings for the DMA module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

