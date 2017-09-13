// metasprite/vram.inc
// ===================
//
// MetaSprite VRAM slots handler.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


Warnings.Register(NO_MS_VRAM_SLOTS, 31, 24, 0)

namespace MetaSprite {
namespace Vram {

// Reuse tilesetAddr address with the upload routines
constant tilesetAddr = Dma.Tile16.tileListAddr


// Resets allocation lists
//
// REQUIRES: 8 bit A, 8 bit Index, DB = $7e
macro _Init() {
    assert8a()
    assert8i()

    constant ARRAY_INCREMENT = MetaSprite.Vram.slots.ARRAY_INCREMENT
    constant ROW_SLOTS_MIN_INDEX = MetaSprite.Vram.slots.ROW_SLOTS_MIN_INDEX
    constant N_ELEMENTS = MetaSprite.Vram.slots.N_ELEMENTS

    // chain linked list
    ldx.b   #0
    lda.b   #ARRAY_INCREMENT

    clc

    Loop:
        sta.w   MetaSprite.Vram.slots.next,x
        tax

        // c clear from branch
        adc.b   #ARRAY_INCREMENT
        cmp.b   #ARRAY_INCREMENT * N_ELEMENTS
        bcc     Loop


    // Separate the slots into two lists

    lda.b   #0x80
    sta.w   MetaSprite.Vram.slots.next + ROW_SLOTS_MIN_INDEX - ARRAY_INCREMENT
    sta.w   MetaSprite.Vram.slots.next + ARRAY_INCREMENT * (N_ELEMENTS - 1)


    // Clear the slot pools

    rep     #$20
a16()
    assert(MetaSprite.Vram.slotPool.oneTile + 1 == MetaSprite.Vram.slotPool.twoTiles)
    assert(MetaSprite.Vram.slotPool.oneRow + 1 == MetaSprite.Vram.slotPool.twoRows)

    lda.w   #0x8080
    sta.w   MetaSprite.Vram.slotPool.oneTile
    sta.w   MetaSprite.Vram.slotPool.oneRow


    // Initialize the first free slot lists

    assert(MetaSprite.Vram.slotPool.freeTiles + 1 == MetaSprite.Vram.slotPool.freeRows)
    lda.w   #MetaSprite.Vram.slots.ROW_SLOTS_MIN_INDEX << 8
    sta.w   MetaSprite.Vram.slotPool.freeTiles

    sep     #$20
a8()
}


// Removes the Vram Slot from the Entity
//
// REQUIRES: 8 bit A, 8 bit Index, DB = $7e
// INPUT: DP = Entity
macro _Deallocate() {
    assert8a()
    assert8i()

    assert(MetaSprite.StatusFlags.vramSet == 0x01)

    lda.b	BaseEntity.MetaSprite.status
    lsr

    bcc     Skip
        jsr     MetaSprite.Vram._DeallocateSlot_A
    Skip:
}



// Activates the given entity's vram table
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: X = frameSet
// BRANCH: ReturnFailure on failure
// MODIFIES: Register size
macro _Activate() {
    assert8a()
    assert16i()

    assert(MetaSprite.StatusFlags.vramSet == 0x01)

    lda.b	BaseEntity.MetaSprite.status
    lsr
    bcs     AlreadyActive
        // X = frameset
        lda.l   MetaSprite.Format.FrameSet.tilesetType,x

        rep     #$30
    a16()
        and.w   #MetaSprite.Vram._ActivateTable.mask
        tay

        lda.l   MetaSprite.Format.FrameSet.tileset,x

        tyx
        jmp     (MetaSprite.Vram._ActivateTable,x)

    Success:
    AlreadyActive:
}

code()
_ActivateTable:
    dw  _ActivateFixed.OneTile
    dw  _ActivateFixed.TwoTiles
    dw  _ActivateFixed.OneRow
    dw  _ActivateFixed.TwoRows
    dw  _ActivateDynamic.OneTile
    dw  _ActivateDynamic.TwoTiles
    dw  _ActivateDynamic.OneRow
    dw  _ActivateDynamic.TwoRows

assertPowerOfTwo(pc() - _ActivateTable)
constant _ActivateTable.mask = pc() - _ActivateTable - 2


// Deallocates the vram slot from the entity.
//
// References are decreased and the slot is moved into the free slots
// pool if if necessary.
//
//
// REQUIRES: 8 bit A, 8 bit Index, DB = 0x7e
// INPUT: DP = entity
// INPUT: A = `BaseEntity.MetaSprite.status >> 1`
a8()
i8()
code()
function _DeallocateSlot_A {
    macro _RemoveSlot(freeSlotPool, oneSlotPool, twoSlotPool) {
        assert8a()
        assert8i()

        // X = current
        namespace {freeSlotPool} {
            constant Return = UpdateStatus

            assert(StatusFlags.dynamicTileset == 0x40)

            // if entity is not dynamic:
            //     if current.prev is NULL:
            //          if current == twoSlotPool:
            //              twoSlotPool = current.next
            //          else:
            //              oneSlotPool = current.next
            //     else:
            //          current.prev.next = current.next
            //
            //     if current.next:
            //          current.next.prev = current.prev
            //
            // tmp = freeSlotPool
            // freeSlotPool = current
            // current.next = tmp
            //
            // if current.pair != NULL
            //    freeSlotPool = current.pair
            //    freeSlotPool.next = current

            assert(StatusFlags.dynamicTileset == 0x40)

            bit.b   BaseEntity.MetaSprite.status
            bvs     SkipPoolCheck
                lda.w   slots.next,x
                ldy.w   slots.prev,x
                bpl     NotFirst
                    cpx.w   slotPool.{twoSlotPool}
                    bne     +
                        sta.w   slotPool.{twoSlotPool}
                        bra     EndIf
                    +
                        sta.w   slotPool.{oneSlotPool}
                        bra     EndIf

                NotFirst:
                    // A = slots.next
                    // Y = slots.prev
                    sta.w   slots.next,y
                EndIf:

                // A = slots.next
                tay
                bmi     NotLast
                    lda.w   slots.prev,x
                    sta.w   slots.prev,y
                NotLast:
            SkipPoolCheck:

            lda.w   slotPool.{freeSlotPool}
            stx.w   slotPool.{freeSlotPool}
            sta.w   slots.next,x

            // If slot has a pair
            // Insert second slot into free list
            ldy.w   slots.pair,x
            bmi     Return
                txa
                sty.w   slotPool.{freeSlotPool}
                sta.w   slots.next,y
        }
    }

    // A = BaseEntity.MetaSprite.status >> 1

    asl
    and.b   #MetaSprite.StatusFlags.vramSlotMask
    tax

    dec.w   slots.count,x
    bne     UpdateStatus

    RemoveSlots:
        cpx.b   #slots.ROW_SLOTS_MIN_INDEX
        bcs     RemoveRows

            _RemoveSlot(freeTiles, oneTile, twoTiles)
            bra     UpdateStatus

        RemoveRows:
            _RemoveSlot(freeRows, oneRow, twoRows)

UpdateStatus:
    lda.b   #StatusFlags.vramSet | StatusFlags.vramSlotMask | StatusFlags.dynamicTileset
    trb.b   BaseEntity.MetaSprite.status
    rts
}



// Uploads a tileset to VRAM
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// ASSUMES: Entity has a frameSet with a dynamic tileset.
//          Entity has a slot allocated to it.
//          Entity has a frame with a tileset
//
// INPUT: DP: Entity
// OUTPUT: C set if tileset uploads successfully
a16()
i16()
code()
function _UploadDynamicTileset {
    ldx.b   BaseEntity.MetaSprite.currentFrame
    beq     ReturnTrue

GivenFrame:
    lda.l   MetaSprite.Format.Frame.tileset,x
    beq     ReturnTrue
    sta.w   tilesetAddr
    tay

        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.vramSlotMask
        tax

    tya

    sep     #$10
i8()
    // Check if tileset is valid
    cmp.w   slots.tileset,x
    beq     ReturnTrueRep

    pea     ReturnFromUploadRoutine - 1


    lda.l   slots.VramAddress,x
    sta.w   Dma.Tile16.vramWaddr

    ldy.w   slots.pair,x
    bpl     TwoSlots

        // One Slot
        cpx.b   #slots.ROW_SLOTS_MIN_INDEX
        bcs     +
            jmp     Dma.Tile16.QueueOneTileFromList
        +
            jmp     Dma.Tile16.QueueOneRowFromList


    TwoSlots:
        tyx
        lda.l   slots.VramAddress,x
        sta.w   Dma.Tile16.vramWaddr2

        // Two Slots
        cpx.b   #slots.ROW_SLOTS_MIN_INDEX
        bcs     +
            jmp     Dma.Tile16.QueueTwoTilesFromList
        +
            jmp     Dma.Tile16.QueueTwoRowsFromList


ReturnFromUploadRoutine:
    rep     #$30
a16()
i16()
    bcc     Return
        // Tileset uploaded successfully
        // Change the tileset address to reflect this

        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.vramSlotMask
        tax

        lda.w   tilesetAddr
        sta.w   slots.tileset,x

        sec
        rts


ReturnTrueRep:
    rep     #$30
a16()

ReturnTrue:
    sec

Return:
    rts

}



// Processes a fixed metasprite
//
// A fixed tileset is one that does not change throughout the life of
// the frameSet that the metasprite belongs to. Because of this we can
// scan though all the fixed slots in order to detect duplicates.
//
// ASSUMES: Entity has a frameSet with a fixed tileset.
//          Entity does not have a slot attached to it.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT:
//      DP: Entity
//       A: Tileset Address
//
// RETURN:
//      Jump to ReturnSuccess on Success
//      Jump to ReturnFailure on Failure
//      Register size unknown
namespace _ActivateFixed {
    constant ReturnSuccess = MetaSprite.Activate.Vram._Activate.Success
    constant ReturnFailure = MetaSprite.Activate.ReturnFailure

    inline Process(name, freeSlotPool, usedSlotPool, nSlots, uploadRoutine) {
        a16()
        i16()
        code()
        function {name} {
            sta.w   tilesetAddr

            sep     #$10
        i8()

            ldx.w   slotPool.{usedSlotPool}
            bmi     NotFound
            -
                cmp.w   slots.tileset,x
                beq     Found_X

                ldy.w   slots.next,x
                bmi     NotFound

                cmp.w   slots.tileset,y
                beq     Found_Y

                ldx.w   slots.next,y
                bpl     -

        NotFound:
        a16()
        i8()
            if {nSlots} == 1 {
                NotFound_Single({freeSlotPool}, {usedSlotPool}, {uploadRoutine})
                jmp     SetState_OneSlot
            } else {
                NotFound_Dual({freeSlotPool}, {usedSlotPool}, {uploadRoutine})
                jmp     SetState_TwoSlots
            }

        Found_Y:
            tyx
        Found_X:
            sep     #$30
        a8()
        i8()
            inc.w   slots.count,x

            if {nSlots} == 1 {
                jmp     SetState_OneSlot
            } else {
                jmp     SetState_TwoSlots
            }

        NoDmaTime:
        NoSlotsFound:
            jmp     ReturnFailure
        }
    }


    // Creates a new slot and uploads it to vram (if possible)
    // INPUT: 16 bit A, 8 bit Index
    // OUTPUT: 8 bit A, 8 bit Index
    // OUTPUT: X = slot
    // BRANCH: to NoSlotsFound if no slots available
    // BRANCH: to NoDmaTime if no slots available
    macro NotFound_Single(freeSlotPool, usedSlotPool, uploadRoutine) {
        assert16a()
        assert8i()

        ldx.w   slotPool.{freeSlotPool}
        bmi     NoSlotsFound

        // Upload to VRAM
        lda.l   slots.VramAddress,x
        sta.w   Dma.Tile16.vramWaddr

        jsr     Dma.Tile16.{uploadRoutine}
        bcc     NoDmaTime

        // Insert slot into fixed tiles list

        sep     #$30
    a8()
    i8()
        ldx.w   slotPool.{freeSlotPool}

        // current->tileset = tilesetAddr
        // current->count = 1
        // current->prev = NULL
        // current->pair = NULL
        //
        // freeSlotPool = current->next
        //
        // current->next = usedSlotPool
        // if current->next != null:
        //     current->next->prev = current
        // usedSlotPool = current

        rep     #$20
            lda.w   tilesetAddr
            sta.w   slots.tileset,x
        sep     #$20

        lda.b   #1
        sta.w   slots.count,x

        lda.b   #0x80
        sta.w   slots.prev,x
        sta.w   slots.pair,x

        lda.w   slots.next,x
        sta.w   slotPool.{freeSlotPool}

        lda.w   slotPool.{usedSlotPool}
        sta.w   slots.next,x

        tay
        bmi     +
            txa
            sta.w   slots.prev,y
    +
        txa
        sta.w   slotPool.{usedSlotPool}
    }


    // Creates two new slot and uploads them to vram (if possible)
    // INPUT: 16 bit A, 8 bit Index
    // OUTPUT: 8 bit A, 8 bit Index
    // OUTPUT: X = slot index
    // BRANCH: to NoSlotsFound if no slots available
    // BRANCH: to NoDmaTime if no slots available
    macro NotFound_Dual(freeSlotPool, usedSlotPool, uploadRoutine) {
        assert16a()
        assert8i()

        ldx.w   slotPool.{freeSlotPool}
        bmi     NoSlotsFound

        ldy.w   slots.next,x
        bmi     NoSlotsFound

        // Upload to VRAM
        lda.l   slots.VramAddress,x
        sta.w   Dma.Tile16.vramWaddr

        tyx
        lda.l   slots.VramAddress,x
        sta.w   Dma.Tile16.vramWaddr2

        jsr     Dma.Tile16.{uploadRoutine}
        bcc     NoDmaTime

        sep     #$30
    a8()
    i8()
        ldx.w   slotPool.{freeSlotPool}
        ldy.w   slots.next,x

        // Remove both slots from free list
        // Connect second slot to first slot (pair).
        // Insert first slot into fixed tiles list
        //
        // first->tileset = tilesetAddr
        // first->count = 1
        // first->prev = NULL
        // first->pair = second
        //
        // freeSlotPool = second->next
        //
        // first->next = usedSlotPool
        // if first->next:
        //     first->next->prev = first
        // usedSlotPool = first

        // X = first
        // Y = second

        rep     #$20
            lda.w   tilesetAddr
            sta.w   slots.tileset,x
        sep     #$20

        lda.b   #1
        sta.w   slots.count,x

        lda.b   #0x80
        sta.w   slots.prev,x
        tya
        sta.w   slots.pair,x

        lda.w   slots.next,y
        sta.w   slotPool.{freeSlotPool}

        lda.w   slotPool.{usedSlotPool}
        sta.w   slots.next,x

        tay
        bmi     +
            txa
            sta.w   slots.prev,y
    +
        txa
        sta.w   slotPool.{usedSlotPool}
    }


    // Update the MetaSprite status and charAttr Offsets then jumps to Return
    //
    // REQUIRES: 8 bit A, 8 bit Index
    // INPUT: X = slot index
    // OUTPUT: C set
    // RETURN: Jump to Return
    // MODIFIES: register size
    a8()
    i8()
    code()
    function SetState_OneSlot {
        assert(MetaSprite.StatusFlags.paletteSet == 0x80)

        lda.b   BaseEntity.MetaSprite.status
        asl
        txa
        bcc     +
            ora.b   #StatusFlags.paletteSet | StatusFlags.vramSet
            bra     ++
        +
            ora.b   #StatusFlags.vramSet
        +

        sta.b   BaseEntity.MetaSprite.status

        rep     #$20
    a16()
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        and.w   #~OamFormat.charAttr.char.mask
        ora.l   slots.CharAttrOffset,x
        sta.b   BaseEntity.MetaSprite.blockOneCharAttrOffset

        jmp     ReturnSuccess
    }


    // Update the MetaSprite status and charAttr Offsets then jumps to Return
    //
    // REQUIRES: 8 bit A, 8 bit Index
    // INPUT: X = slot 1 index
    // OUTPUT: C set
    // RETURN: Jump to Return
    // MODIFIES: register size
    a8()
    i8()
    code()
    function SetState_TwoSlots {
        assert(MetaSprite.StatusFlags.paletteSet == 0x80)

        lda.b   BaseEntity.MetaSprite.status
        asl
        txa
        bcc     +
            ora.b   #StatusFlags.paletteSet | StatusFlags.vramSet
            bra     ++
        +
            ora.b   #StatusFlags.vramSet
        +

        sta.b   BaseEntity.MetaSprite.status

        rep     #$20
    a16()
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        and.w   #~OamFormat.charAttr.char.mask
        ora.l   slots.CharAttrOffset,x
        sta.b   BaseEntity.MetaSprite.blockOneCharAttrOffset

        txy
        ldx.w   slots.pair,y

        and.w   #~OamFormat.charAttr.char.mask
        ora.l   slots.CharAttrOffset,x
        sta.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset

        jmp     ReturnSuccess
    }


    Process(OneTile, freeTiles, oneTile, 1, QueueOneTileFromList)
    Process(TwoTiles, freeTiles, twoTiles, 2, QueueTwoTilesFromList)
    Process(OneRow, freeRows, oneRow, 1, QueueOneRowFromList)
    Process(TwoRows, freeRows, twoRows, 2, QueueTwoRowsFromList)
}



// Allocates the VRAM slot(s) of a dynamic tileset MetaSprite, uploading
// the MetaSprite frame's tiles (if necessary).
//
// ASSUMES: Entity has a frameSet with a dynamic tileset.
//          Entity does not have a slot attached to it.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT:
//      DP: Entity
//       A: Tileset Address
//
// RETURN:
//      Jump to ReturnSuccess on Success
//      Jump to ReturnFailure on Failure
//      Register size unknown
namespace _ActivateDynamic {
    constant ReturnSuccess = MetaSprite.Activate.Vram._Activate.Success
    constant ReturnFailure = MetaSprite.Activate.ReturnFailure

// ::SHOULDO add dynamic tileset with fixed portion of tiles::
// ::: ie, tiles 5-7 are fixed through the life of the entity::
// ::: this could be enabled by setting FrameSet.tileset to non-zero::


    // Single code: (split into two halfs)
    //
    // if freeSlotPool == NULL:
    //    return false
    //
    // current = freeSlotPool
    // freeSlotPool = current->next
    // current->tileset = NULL
    // current->count = 1
    // current->prev = NULL
    // current->next = NULL
    // current->pair = NULL
    //
    // MetaSprite.status &= ~StatusFlags.vramSlotMask
    // MetaSprite.status |= current
    // MetaSprite.status |= StatusFlags.vramSet | StatusFlags.dynamicTileset
    //
    // MetaSprite.blockOneCharAttrOffset &= ~OamFormat.change.char.mask
    // MetaSprite.blockOneCharAttrOffset |= current->CharAttrOffset
    //
    // return _UploadDynamicTileset()

    inline Process_Single(name, freeSlotPool, fallThrough) {
        a16()
        i16()
        code()
        function {name} {
            sep     #$10
        i8()

            ldx.w   slotPool.{freeSlotPool}
            bmi     NoSlotsFound

            ldy.w   slots.next,x
            sty.w   slotPool.{freeSlotPool}

            if {fallThrough} {
                _FallThrough:
            } else {
                bra     Finalize_Single
            }
        }
    }
    Process_Single(OneTile, freeTiles, 0)
    Process_Single(OneRow, freeRows, 1)


    code()
    a16()
    i8()
    function Finalize_Single {
        assert(pc() == OneRow._FallThrough)

        stz.w   slots.tileset,x

        assert(slots.next + 1 == slots.prev)
        lda.w   #0x8080
        sta.w   slots.next,x

        assert(slots.count + 1 == slots.pair)
        lda.w   #0x8001
        sta.w   slots.count,x

        sep     #$20
    a8()
        assert(MetaSprite.StatusFlags.paletteSet == 0x80)

        lda.b   BaseEntity.MetaSprite.status
        asl
        txa
        bcc     +
            ora.b   #StatusFlags.paletteSet | StatusFlags.vramSet | StatusFlags.dynamicTileset
            bra     ++
        +
            ora.b   #StatusFlags.vramSet | StatusFlags.dynamicTileset
        +

        sta.b   BaseEntity.MetaSprite.status

        rep     #$30
    a16()
    i16()
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        and.w   #~OamFormat.charAttr.char.mask
        ora.l   slots.CharAttrOffset,x
        sta.b   BaseEntity.MetaSprite.blockOneCharAttrOffset

        jsr     MetaSprite.Vram._UploadDynamicTileset
        bcc     Fail
        jmp     ReturnSuccess
    }


    a16()
    i8()
    code()
    NoSlotsFound:
        Warnings.Warn(NO_MS_VRAM_SLOTS)
    Fail:
        jmp     ReturnFailure


    // Dual code: (split into two halfs)
    //
    // if freeSlotPool == NULL:
    //    return false
    //
    // if freeSlotPool->next == NULL:
    //    return false
    //
    // first = freeSlotPool
    // second = first->next
    //
    // freeSlotPool = second->next
    //
    // current->tileset = NULL
    // first->count = 1
    // first->prev = NULL
    // first->next = NULL
    // first->pair = second
    //
    // MetaSprite.status &= ~StatusFlags.vramSlotMask
    // MetaSprite.status |= current
    // MetaSprite.status |= StatusFlags.vramSet | StatusFlags.dynamicTileset
    //
    // MetaSprite.blockOneCharAttrOffset &= ~OamFormat.change.char.mask
    // MetaSprite.blockOneCharAttrOffset |= current->CharAttrOffset
    // MetaSprite.blockTwoCharAttrOffset &= ~OamFormat.change.char.mask
    // MetaSprite.blockTwoCharAttrOffset |= current->CharAttrOffset
    //
    // return _UploadDynamicTileset()

    inline Process_Dual(name, freeSlotPool, fallThrough) {
        a16()
        i16()
        code()
        function {name} {
            sep     #$10
        i8()

            ldx.w   slotPool.{freeSlotPool}
            bmi     NoSlotsFound

            ldy.w   slots.next,x
            bmi     NoSlotsFound


            stz.w   slots.tileset,x

            assert(slots.next + 1 == slots.prev)
            lda.w   #0x8080
            sta.w   slots.next,x

            sep     #$20
        a8()
            lda.w   slots.next,y
            sta.w   slotPool.{freeSlotPool}

            if {fallThrough} {
                _FallThrough:
            } else {
                bra     Finalize_Dual
            }
        }
    }
    Process_Dual(TwoRows, freeRows, 0)
    Process_Dual(TwoTiles, freeTiles, 1)


    a8()
    i8()
    code()
    function Finalize_Dual {
        assert(pc() == TwoTiles._FallThrough)

        lda.b   #1
        sta.w   slots.count,x

        tya
        sta.w   slots.pair,x

        assert(MetaSprite.StatusFlags.paletteSet == 0x80)
        lda.b   BaseEntity.MetaSprite.status
        asl
        txa
        bcc     +
            ora.b   #StatusFlags.paletteSet | StatusFlags.vramSet | StatusFlags.dynamicTileset
            bra     ++
        +
            ora.b   #StatusFlags.vramSet | StatusFlags.dynamicTileset
        +

        sta.b   BaseEntity.MetaSprite.status

        rep     #$30
    a16()
    i16()
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        and.w   #~OamFormat.charAttr.char.mask
        ora.l   slots.CharAttrOffset,x
        sta.b   BaseEntity.MetaSprite.blockOneCharAttrOffset

        tyx
        and.w   #~OamFormat.charAttr.char.mask
        ora.l   slots.CharAttrOffset,x
        sta.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset

        jsr     MetaSprite.Vram._UploadDynamicTileset
        bcc     Fail
        jmp     ReturnSuccess
    }
}


// Read only slot data.
rodata(rom0)
namespace slots {
    constant ROW_START = ((VRAM_TILE_SLOTS + 7) / 8)

    // The CharAttr offset for each slot
    function CharAttrOffset {
        macro tiles(variable i) {
            if i < VRAM_TILE_SLOTS {
                dw  (i / 8) * 32 + (i & 7) * 2
                tiles(i + 1)
            }
        }
        macro rows(variable i) {
            if i < VRAM_ROW_SLOTS {
                dw  (ROW_START + i) * 32
                rows(i + 1)
            }
        }

        tiles(0)
        rows(0)
    }

    // The VRAM addresses for each slot
    function VramAddress {
        macro tiles(variable i) {
            if i < VRAM_TILE_SLOTS {
                dw  VRAM_OBJ_WADDR + ((i / 8) * 32 + (i & 7) * 2) * 16
                tiles(i + 1)
            }
        }
        macro rows(variable i) {
            if i < VRAM_ROW_SLOTS {
                dw  VRAM_OBJ_WADDR + ((ROW_START + i) * 32) * 16
                rows(i + 1)
            }
        }

        tiles(0)
        rows(0)
    }
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

