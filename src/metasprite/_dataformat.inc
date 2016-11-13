
// Data format may be subject to changes
if MetaSprite.Data.METASPRITE_FORMAT_VERSION != 24 {
    error "Invalid UnTech MetaSprite Format"
}

// In order to simplify processing of the MetaSprites, there are no
// signed integers in the MetaSprite data structures.
//
// Instead a data type called `int_ms8` is used with the format:
//
//      ROM DATA = (value + INT_MS8_OFFSET)
//
constant INT_MS8_OFFSET(128)


scope MetaSprite {

scope Data {
    storeBlockStart(FrameSetList, MS_FrameSetList)
    storeBlockStart(PaletteList,  MS_PaletteList)
    storeBlockStart(FrameList,    MS_FrameList)
    storeBlockStart(AnimationList,MS_AnimationList)

    rodata(MS_FrameSetData)
        constant FrameSetOffset(pc() & 0xff0000)

    rodata(MS_PaletteData)
        constant PaletteBank(pc() >> 16)
        constant PaletteOffset(pc() & 0xff0000)

    rodata(MS_FrameData)
        constant FrameOffset(pc() & 0xff0000)

    rodata(MS_FrameObjectsData)
        constant FrameObjectsOffset(pc() & 0xff0000)

    rodata(MS_AnimationData)
        constant AnimationBytecodeOffset(pc() & 0xff0000)

    rodata(MS_TileHitboxData)
        constant TileHitboxOffset(pc() & 0xff0000)

    rodata(MS_EntityHitboxData)
        constant EntityHitboxOffset(pc() & 0xff0000)

    rodata(MS_ActionPointsData)
        constant ActionPointsOffset(pc() & 0xff0000)

    code()
}

scope Format {

    // The root FrameSet data structure
    scope FrameSet {
        struct(Data.FrameSetOffset)
            // Index of the palette table in `Data.PaletteList`
            field(paletteTable, 2)
            // Number of palettes for the framseSet
            field(nPalettes, 1)

            // Address of the tileset in the `DMA_Tile16Data` block
            // May be NULL
            field(tileset, 2)

            // The tileset size and lifestyle
            field(tilesetType, 1)

            // Index of the frame table in `Data.FrameList`
            field(frameTable, 2)
            // Number of frames for the frameSet
            field(nFrames, 1)

            // Index of the animation table in `Data.AnimationList`
            field(animationTable, 2)
            // Number of animations
            field(nAnimations, 1)
        endstruct()

        scope tilesetType {
            scope Size {
                constant ONE_TILE(0x00)
                constant TWO_TILES(0x02)
                constant ONE_ROW(0x04)
                constant TWO_ROWS(0x08)
            }
            scope Lifespan {
                constant FIXED(0x80)
                constant DYNAMIC(0x00)
            }
        }
    }

    // Represents a MetaSprite Frame
    //
    // Any of these values can be NULL (0), in which case processing
    // will be aborted
    scope Frame {
        struct(Data.FrameOffset)
            // Address of the FrameObjects in the `MS_FrameObjectsData` block
            // May be NULL
            field(frameObjects, 2)

            // Address of the EntityHitbox in the `MS_EntityHitboxData` block
            // May be NULL
            field(entityHitbox, 2)

            // Address of the TileHitbox` in the `MS_TileHitboxData` block
            // May be NULL
            field(tileHitbox, 2)

            // Address of the ActionPoints in the `MS_ActionPointsData` block
            // May be NULL
            field(actionPoints, 2)

            // Address of the tileset in the `DMA_Tile16Data` block
            //
            // MUST exist if FrameSet is dynamic
            //
            // Optional: Never read on fixed tilesets
            field(tileset, 2)
        endstruct()
    }


    // Represents the metasprite frames objects.
    //
    // This format is converted into OAM data with the following:
    //
    //   oam.xpos = entity.xPos - 128 + frameObject.xPos
    //   oam.ypos = entity.yPos - 128 + frameObject.yPos
    //   oam.size = frameObject.attr & 1
    //   if frameObject.char & 0x20 == 0:
    //      oam.charAttr = (frameObject.charAttr & 0xF01F) + entity.blockOneCharAttrOffset
    //   else:
    //      oam.charAttr = (frameObject.charAttr & 0xF01F) + entity.blockTwoCharAttrOffset
    scope FrameObjects {
        struct(Data.FrameObjectsOffset)
            // Determines the number of objects used by the frame
            //
            // Value is `number of objects - 1`.
            // The maximum allowed value is 31 (32 objects).
            field(count, 1)
            constant COUNT_MASK(0x1f)

            scope Object {
                // X offset relative origin
                // int_ms8 format
                field(xOffset, 1)

                // Y offset relative origin
                // int_ms8 format
                field(yOffset, 1)

                // Object attributes
                //
                // vhoo00s0 00bccccc
                //      v     - vflip
                //      h     - hflip
                //      oo    - order (0 - 3)
                //      s     - size (0 = small, 1 = large)
                //      b     - block number (0 / 1)
                //      ccccc - character number within block.
                field(charAttr, 2)
                scope charAttr {
                    constant mask(0xF01F)
                    constant sizeBit(0x0200)
                    constant blockBit(0x0020)
                }
            }
        endstruct()
    }


    // The hitbox of the entity, used by the physics engine for entity
    // collisions.
    //
    // The collision hitbox is represented by multiple Axis-Aligned
    // Bounding Boxes, each of a different type.
    //
    // The type is dependant of the implementation code, but allows for
    // flexibility in defining different collision areas on a frame.
    //
    // For example, an enemy frame could consist of:
    //
    //      * a sword AABB (the part the hurts the player)
    //      * a shield AABB (the part where no damage would occur if hit)
    //      * a weak-point AABB (where double damage would occur if hit)
    //      * a body AABB (where normal damage would occur if hit)
    //
    //
    // This data structure uses 16 bit values for the width and height,
    // as it saves 20% cpu time in the check collision code (on average).
    //
    // In order to save processing an "outer" hitbox is used. This
    // hitbox is tested first, then only if that hits, will the inner
    // hitboxes be tested.
    //
    // Some hitboxes only involve one AABB. In this case count is 0 and
    // the outer hitbox is checked.
    scope EntityHitbox {
        struct(Data.EntityHitboxOffset)
            // Determines the number of hitboxes used by the entity.
            //
            // Value is `number of hitboxes - 1`. The maximum allowed
            // value is 3 (4 hitboxes).
            //
            // If zero then only the outer hitbox is checked, the inner
            // hitboxes are not processed
            field(count, 1)
            constant COUNT_MASK(3)

            // Outer hitbox, checked first
            scope Outer {
                field(xOffset, 1) // int_ms8 format
                field(yOffset, 1) // int_ms8 format
                field(width, 2) // uint16
                field(height, 2) // uint16
            }

            // The inner hitboxes, repeated `count + 1` times
            //
            // All inner hitboxes MUST be inside the outer hitbox.
            scope Inner {
                // type of hitbox
                // (EntityHitboxType uint8 value)
                field(type, 1)

                field(xOffset, 1) // int_ms8 format
                field(yOffset, 1) // int_ms8 format
                field(width, 2) // uint16
                field(height, 2) // uint16

                constant size(7)
            }

            // The type of a single hitbox
            //
            // Only set if count == 0
            constant singleHitboxType(Inner.type)
        endstruct()
    }

    // The entity hitbox type
    // See entity collision code for collision table
    scope EntityHitboxType {
        createEnum(0, 2)

        enum(BODY)
        enum(BODY_WEAK)
        enum(BODY_ATTACK)   // can be hurt by attack types
        enum(SHIELD)
        enum(SHIELD_ATTACK)
        enum(ATTACK)        // will phase through other attack hitboxes

        constant _TABLE_SHIFT(3)
        constant _MASK((1 << (_TABLE_SHIFT + 1)) - 2)
        constant _TABLE_MASK(_MASK << _TABLE_SHIFT | _MASK)

        assert({__enum.current} <= 2 << _TABLE_SHIFT)
    }


    // A hitbox of the entity, used by the physics engine for collisions
    // with the meta-tilemap.
    //
    // For the moment the engine only supports a single AABB hitbox.
    scope TileHitbox {
        struct(Data.TileHitboxOffset)
            field(xOffset, 1) // int_ms8 format
            field(yOffset, 1) // int_ms8 format
            field(width, 1) // uint8
            field(height, 1) // uint8
        endstruct()
    }


    // Action points used by the metasprite animation engine.
    //
    // The data structure is NULL terminated.
    //
    // This is used by the animator to tell the game-loop when to
    // preform certain actions.
    //
    // Examples of action points include:
    //      * Fire weapon frame - would includes the position and direction
    //        of the projectile launched
    //      * Feed touching ground - used for run/walk sounds.
    //
    scope ActionPoints {
        struct(Data.ActionPointsOffset)
            // The type of action point
            // Value passed to the entity's ProcessPoint routine
            //
            // MUST NOT BE 0. If Parameter is 0, the loop is terminated
            field(parameter, 1)

            field(xPos, 1)  // int_ms8
            field(yPos, 1) // int_ms8
        endstruct()
    }


    scope Animation {
        scope Bytecode {
            createEnum(0, 2)

            // Stops the animation.
            enum(STOP)

            // Goto the start of the animation
            enum(GOTO_START)

            // Goto a specific animation
            // param: (uint8) animation Id
            enum(GOTO_ANIMATION)

            // Updates the animation PC by a specific offset
            // param: (int_ms8) byte offset
            enum(GOTO_OFFSET)

            // Sets the given frame and waits a given amount of frames.
            //
            // If the metasprite frame could not be set, then the engine will
            // try again in the next display frame.
            //
            // param: (uint8) frame Id
            // param: (uint8) number of frames to wait
            enum(SET_FRAME_AND_WAIT_FRAMES)

            // Sets the given frame and waits a given amount of time.
            //
            // If the metasprite frame could not be set, then the engine will
            // try again in the next display frame.
            //
            // param: (uint8) frame Id
            // param: (uint8) time to wait (1/75ths of a second)
            enum(SET_FRAME_AND_WAIT_TIME)

            // Sets the given frame and waits until the entity has moved a given
            // distance horizontally.
            //
            // If the metasprite frame could not be set, then the engine will
            // try again in the next display frame.
            //
            // NOTE: Internally uses 1:7:8 fixed point math; the animation
            // will not preform correctly if entity velocity is out of bounds.
            //
            // param: (uint8) frame Id
            // param: (0:3:5 fixed point) distance to wait
            enum(SET_FRAME_AND_WAIT_XVECL)

            // Sets the given frame and waits until the entity has moved a given
            // distance vertically.
            //
            // If the metasprite frame could not be set, then the engine will
            // try again in the next display frame.
            //
            // NOTE: Internally uses 1:7:8 fixed point math; the animation
            // will not preform correctly if entity velocity is out of bounds.
            //
            // param: (uint8) frame Id
            // param: (0:3:5 fixed point) distance to wait
            enum(SET_FRAME_AND_WAIT_YVECL)
        }
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

