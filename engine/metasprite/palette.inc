// metasprite/palette.inc
// ======================
//
// MetaSprite palette slots handler.
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


Warnings.Register(NO_MS_PALETTE_SLOTS, 0, 24, 24)

namespace MetaSprite {
namespace Palette {


// Resets allocation lists
//
// REQUIRES: 8 bit A, 8 bit Index, DB = $7e
macro _Init() {
    assert8a()
    assert8i()

    constant ARRAY_INCREMENT = MetaSprite.Palette.slots.ARRAY_INCREMENT
    constant N_ELEMENTS = MetaSprite.Palette.slots.N_ELEMENTS

    // chain linked list

    ldx.b   #0
    lda.b   #ARRAY_INCREMENT

    clc

    Loop:
        sta.w   MetaSprite.Palette.slots.next,x
        stz.w   MetaSprite.Palette.slots.count,x
        tax

        // c clear from branch
        adc.b   #ARRAY_INCREMENT
        cmp.b   #ARRAY_INCREMENT * N_ELEMENTS
        bcc     Loop

    lda.b   #0x80
    sta.w   MetaSprite.Palette.slots.next + ARRAY_INCREMENT * (N_ELEMENTS - 1)
    stz.w   MetaSprite.Palette.slots.count + ARRAY_INCREMENT * (N_ELEMENTS - 1)

    // reset list pointers
    sta.w   MetaSprite.Palette.slotPool.used
    stz.w   MetaSprite.Palette.slotPool.free
}


// Removes the Palette from the Entity
//
// REQUIRES: 8 bit A, 8 bit Index, DB = $7e
// INPUT: DP = Entity
macro _Deallocate() {
    assert8a()
    assert8i()

    assert(MetaSprite.StatusFlags.paletteSet == 0x80)

    lda.b   #MetaSprite.StatusFlags.paletteSet
    trb.b	BaseEntity.MetaSprite.status

    beq     Skip
        // Entity has a palette, decrease the slot counter
        // Remove Slot if counter is 0

        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset + 1
        and.b	#OamFormat.attr.palette.mask
        tax

        dec.w   MetaSprite.Palette.slots.count,x
        bne     Skip
            jsr     MetaSprite.Palette._RemoveSlot
    Skip:
}


// Moves the palette slot from used list and into free slots pool.
//
// The slot MUST BE empty before calling this routine.
//
// REQUITES: 8 bit A, 8 bit Index, DB = 0x7e
// INPUT: X - slot to remove
a8()
i8()
code()
function _RemoveSlot {
	// Remove from used list and add to free list
	//
	// if current.prev is NULL:
	//   slotPool.used = current.next
	// else:
	//	 current.prev.next = current.next
	//
	// if current.next:
	//	 current.next.prev = current.prev
    //
    // tmp = slotPool.free
    // slotPool.free = current
    // current.next = tmp

    ldy.w   slots.prev,x
    bpl     +
        lda.w   slots.next,x
        sta.w   slotPool.used
        bra     ++
    +
        lda.w   slots.next,x
        sta.w   slots.next,y
    +

    tay
    bmi     +
        lda.w   slots.prev,x
        sta.w   slots.prev,y
    +

    lda.w   slotPool.free
    stx.w   slotPool.free
    sta.w   slots.next,x

    rts
}


// Sets the Entity's MetaSprite palette
// Will only upload the palette if entity is activated
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: A: Palette Id
// INPUT: DP: BaseEntity
// OUTPUT: carry set if palette was successfully changed
a8()
i16()
code()
function SetPalette {
    cmp.b   BaseEntity.MetaSprite.palette
    beq     Return      // c set on return

    ldx.b   BaseEntity.MetaSprite.frameSet

    cmp.l   MetaSprite.Format.FrameSet.nPalettes,x
    bcs     ReturnFalse

    sta.b   BaseEntity.MetaSprite.palette

    // If the Entity is active
    //  Upload the new/old palette
    lda.b   #StatusFlags.paletteSet | StatusFlags.vramSet
    bit.b   BaseEntity.MetaSprite.status
    bne     _ProcessSlots

    sec
Return:
    rts

ReturnFalse:
    clc
    rts
}



// Processes the palette slots for the entity, sets the charattr bits and
// and uploads the palette to the Palette Buffer if necessary
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP: BaseEntity - must be active
//         X: MetaSprite frameset address
// OUTPUT: carry set if successful
a8()
i16()
code()
function _ProcessSlots {
allocateTmpWord(tmp_paletteAddr)
allocateTmpWord(tmp_slotIndex)

    lda.b   BaseEntity.MetaSprite.palette

    rep     #$30
a16()
i16()
    and.w   #0x00FF
    asl
    // c clear
    adc.l   MetaSprite.Format.FrameSet.paletteTable,x
    tax
    lda.l   MetaSprite.Data.PaletteList,x
    sta.w   tmp_paletteAddr

    assert(MetaSprite.StatusFlags.paletteSet == 0x80)
    sep     #$10
i8()
    ldy.b   BaseEntity.MetaSprite.status
    bpl     SearchForDuplicateSlot
        // MetaSprite already has a palette
        // Check if palette has changed
        //   If address has changed, decrement reference count
        //   If address is equal, return success

        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset + 1
        and.w	#OamFormat.attr.palette.mask
        tax

        lda.w   tmp_paletteAddr
        cmp.w   slots.paletteAddress,x
        bne     +
            // return true -  c set by cmp
            rep     #$30
            rts
        +

        // Palette has changed
        dec.w   MetaSprite.Palette.slots.count,x
        bne     SearchForDuplicateSlot
            sep     #$30
    a8()
    i8()
            jsr     MetaSprite.Palette._RemoveSlot
            rep     #$20
    a16()
            lda.w   tmp_paletteAddr


a16()
i8()
SearchForDuplicateSlot:
    // A = palette address

    // search for a duplicate used palette slot
    ldx.w   slotPool.used
    bmi     NotFound
    -
        cmp.w   slots.paletteAddress,x
        beq     Found_X

        ldy.w   slots.next,x
        bmi     NotFound

        cmp.w   slots.paletteAddress,y
        beq     Found_Y

        ldx.w   slots.next,y
        bpl     -

    bra     NotFound


// Existing slot was Found
// increment the reference counter
Found_Y:
    tyx
Found_X:
    stx.w   tmp_slotIndex

    // can do this in 16 bit mode ad count is 16 bit here
    inc.w   slots.count,x

SetMetaSpriteState:
    // Set palette bits in offsets
    // INPUT: tmp_slotIndex - the slot index

    // luckally palette bits match the palette slot index and slotIndex always <= 7 * 2

    sep     #$31
a8()
i8()
    lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset + 1
    and.b   #~OamFormat.attr.palette.mask
    ora.w   tmp_slotIndex
    sta.b   BaseEntity.MetaSprite.blockOneCharAttrOffset + 1

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #~OamFormat.attr.palette.mask
    ora.w   tmp_slotIndex
    sta.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1

    lda.b   #MetaSprite.StatusFlags.paletteSet
    tsb.b   BaseEntity.MetaSprite.status

    // Return True - C set by sep above
    rep     #$10
i16()
    rts


// No slot contains the palette and there are no free slots remaining
a16()
i8()
NoSlotsFound:
    Warnings.Warn(NO_MS_PALETTE_SLOTS)

    // Return false
    rep     #$31
    sep     #$20
    rts


// No slot contains the palette, create a new slot and copy the palette data to the buffer
a16()
i8()
NotFound:
    ldx.w   slotPool.free
    bmi     NoSlotsFound

    stx.w   tmp_slotIndex

    // New Slot
    // Remove from free list, insert into used list
    // A = palette address
    // X = empty slot index

    // current->address = metasprite->palette
    // current->count = 1
    //
    // current->prev = NULL
    // slotPool.free = current->next
    //
    // current->next = slotPool.used
    // if current->next != null:
    //     current->next->prev = current
    // slotPool.used = current

    sta.w   slots.paletteAddress,x
    lda.w   #1
    sta.w   slots.count,x

    sep     #$20
a8()
    lda.b   #0x80
    sta.w   slots.prev,x

    ldy.w   slots.next,x
    sty.w   slotPool.free

    lda.w   slotPool.used
    sta.w   slots.next,x

    tay
    bmi     +
        txa
        sta.w   slots.prev,y
+
    txa
    sta.w   slotPool.used

    rep     #$30
a16()

    // Copy Palette to object palette buffer
    // A = slot index
    assert(slots.ARRAY_INCREMENT << 4 == 32)
    asl
    asl
    asl
    asl
    tay

    ldx.w   tmp_paletteAddr

evaluate n = 0
while {n} < 15 {
    lda.l   Data.PaletteData + 2 * {n},x
    sta.w   Dma.ObjPalette.buffer + 2 * ({n} + 1),y

    evaluate n = {n} + 1
}

    sep     #$30
a8()
i8()
    Dma.ObjPalette.MarkDirty()

    jmp     SetMetaSpriteState
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

