// metasprite/_dataformat.inc
// ==========================
//
// MetaSprite Data Format.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Data format may be subject to changes
if MetaSprite.Data.METASPRITE_FORMAT_VERSION != 40 {
    error "Invalid UnTech MetaSprite Format"
}

// In order to simplify processing of the MetaSprites, there are no
// signed integers in the MetaSprite data structures.
//
// Instead a biased data type called `int_ms8` is used with an offset of 128.
//
// Integers can be converted to `int_ms8` with the following formula:
//      int_ms8 value = (integerValue + INT_MS8_OFFSET)
//
constant INT_MS8_OFFSET = 128


// `Index+1` is an word index data format with the following meaning
//
//      0:          There is no data (NULL)
//      1-0xffff:   Data index + 1
//
// As the data structs in this file are offset by one there is
// no need to modify the index once it has been read.

namespace MetaSprite {

namespace Data {
    // Table of word indexes within the FrameData block
    constant FrameList = Project.MS_FrameList

    // Table of word indexes within the AnimationData block
    constant AnimationList = Project.MS_AnimationList

    // Table of word indexes within PaletteData
    constant PaletteList = Project.MS_PaletteList;

    constant FrameSetData = Project.MS_FrameSetData;
    constant FrameSetListCount = Project.MS_FrameSetListCount

    constant FrameData = Project.MS_FrameData;
    constant FrameObjectsData = Project.MS_FrameObjectsData;
    constant ActionPointsData = Project.MS_ActionPointsData;
    constant CollisionBoxData = Project.MS_CollisionBoxData;
    constant AnimationData = Project.MS_AnimationData;

    // Palette Data
    // (30 bytes per palette)
    constant PaletteData = Project.MS_PaletteData

    code()
}

namespace Format {

    // The root FrameSet data structure
    namespace FrameSet {
        struct(Data.FrameSetData)
            // Index of the palette table in `Data.PaletteList`
            // (word index)
            field(paletteTable, 2)
            // Number of palettes for the framseSet
            // (uint8)
            field(nPalettes, 1)

            // Index+1 of the tileset in the `DMA_Tile16Data` block
            // May be NULL
            //
            // If not NULL and tileset lifespan is dynamic then the
            // tileset will be uploaded to the end of the VRAM slot on
            // MetaSprite activation.
            //
            // (Index+1 format)
            field(tileset, 2)

            // The tileset size and lifestyle
            // (see TilesetType namespace)
            field(tilesetType, 1)

            // Index of the frame table in `Data.FrameList`
            field(frameTable, 2)
            // Number of frames for the frameSet
            // (uint8)
            field(nFrames, 1)

            // Index of the animation table in `Data.AnimationList`
            // (word index)
            field(animationTable, 2)
            // Number of animations
            // (uint8)
            field(nAnimations, 1)
        endstruct()

        namespace tilesetType {
            namespace Size {
                constant ONE_TILE = 0x00
                constant TWO_TILES = 0x02
                constant ONE_ROW = 0x04
                constant TWO_ROWS = 0x06
            }
            namespace Lifespan {
                constant FIXED = 0x00
                constant DYNAMIC = 0x08
            }
        }
    }

    // Represents a MetaSprite Frame
    //
    // Any of these values can be NULL (0), in which case processing
    // will be aborted
    namespace Frame {
        struct(Data.FrameData)
            // Index+1 of the FrameObjects data in the `FrameObjectsData` block
            // May be NULL
            // (Index+1 format)
            field(frameObjects, 2)

            // Index+1 of the ActionPoints data in the `ActionPointsData` block
            // May be NULL
            // (Index+1 format)
            field(actionPoints, 2)

            // Index into the frame's collision box data.
            // (index into CollisionBoxes struct)
            field(collisionBoxes, 2)

            // Index+1 of the tileset in the `DMA_Tile16Data` block
            //
            // MUST exist if FrameSet is dynamic
            //
            // Optional: Never read on fixed tilesets
            // (Index+1 format)
            field(tileset, 2)

            // The frame's tile hitbox, used for collision testing between
            // the entity and the tilemap.
            //
            // NOTE: The frame's origin point MUST BE inside the tile
            //       hitbox and not touching the hitbox edges.
            //
            // Optional: If the frame has no tile hitbox then all 32
            //           bits of TileHitbox MUST be set.
            //
            // ::TODO move into CollisionBoxes::
            namespace TileHitbox {
                // Distance from origin to the left side of the hitbox.
                // Cannot be zero.
                // (uint8)
                field(left, 1)
                // Distance from origin to the right side of the hitbox.
                // Cannot be zero.
                // (uint8)
                field(right, 1)

                // Distance from the origin to the top of the hitbox
                // Equal to `-top`.  Cannot be zero.  Must be a negative value.
                // (sint8)
                field(yOffset, 1)

                // Height of the hitbox
                // Cannot be zero. `height + yOffset` cannot be zero.
                // (uint8, max value = 127)
                field(height, 1)
            }
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
    namespace FrameObjects {
        assert(Data.FrameObjectsData & 0xffff != 0)
        struct(Data.FrameObjectsData - 1)
            // Determines the number of objects used by the frame
            //
            // Value is `number of objects - 1`.
            // The maximum allowed value is 31 (32 objects).
            field(count, 1)
            constant COUNT_MASK = 0x1f

            namespace Object {
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
                namespace charAttr {
                    constant mask = 0xF01F
                    constant sizeBit = 0x0200
                    constant blockBit = 0x0020
                }
            }
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
    namespace ActionPoints {
        assert(Data.ActionPointsData & 0xffff != 0)
        struct(Data.ActionPointsData - 1)
            // The type of action point
            //
            // See Project.ActionPoints function table
            //
            // MUST NOT BE 0. If type is 0, the loop is terminated
            //
            // (function table byte index)
            field(type, 1)

            // Position of the action point.
            //
            // This field is the argument given to the ActionPoint function.
            //
            // (int_ms8 pair, xPos then yPos)
            field(positionPair, 2)
        endstruct()
    }


    // The entity collision boxes.
    //
    // Each collision box is optional.  If a collision box does not exist then
    // all 32 bits of the box must be set.
    //
    // The maximum width or height for any of these boxes is 127 pixels.
    //
    // NOTE: All MetaSprite frames contain a `CollisionBoxes` data structure.
    //       Thus this struct IS NOT accessed with a `index+1` index.
    //
    namespace CollisionBoxes {
        struct(Data.CollisionBoxData)

            // Collides with an opponents Shield or Hurtbox.
            //
            // This collision box is tested first.  If there is a shield collision,
            // then the Hitbox/Hurtbox collision test will not be preformed.
            //
            // This collision box is ignored for the player entity and the
            // entity in the outer loop of `TestEntityListHitboxAgainstEntityList`.
            //
            namespace Shield {
                field(xOffset, 1) // int_ms8 format
                field(width, 1)   // uint8 (offset by 1)
                field(yOffset, 1) // int_ms8 format
                field(height, 1)  // uint8 (offset by 1)
            }

            // Collides with an opponents Shield or Hurtbox.
            namespace Hurtbox {
                field(xOffset, 1) // int_ms8 format
                field(width, 1)   // uint8 (offset by 1)
                field(yOffset, 1) // int_ms8 format
                field(height, 1)  // uint8 (offset by 1)
            }

            // Collides with an opponents Shield or Hitbox.
            namespace Hitbox {
                field(xOffset, 1) // int_ms8 format
                field(width, 1)   // uint8 (offset by 1)
                field(yOffset, 1) // int_ms8 format
                field(height, 1)  // uint8 (offset by 1)
            }
        endstruct()
    }


    // The animation data
    namespace Animation {
        struct(Data.AnimationData)
            // The animation to execute when all frames have been rendered
            // If >= FrameSet.nAnimations then the animation stops
            // (byte)
            field(nextAnimation, 1)

            // The format of the `Animation.Frames.duration` field
            // (DurationFormat uint8 value)
            field(durationFormat, 1)

            // The size of the Frames table.
            // (byte - MUST BE <= 0xfd)
            field(frameTableSize, 1)

            // Animation frame table, repeated `frameTableSize / 2` times
            namespace Frames {
                // The frame to display
                // (byte)
                field(frameId, 1)

                // Duration of the frame
                // (value depends on the `durationFormat` value)
                field(duration, 1)
            }
        endstruct()
    }

    // The format of `Animation.Frames.duration` field
    namespace AnimationDurationFormat {
        createEnum(0, 2)

            // Frame duration = number of display frames to wait
            enum(FRAME)

            // Frame duration = 1/75th of a second
            enum(TIME)

            // Frame duration = distance the entity travels vertically
            // (0:3:5 fixed point pixels)
            enum(DISTANCE_VERTICALL)

            // Frame duration = distance the entity to travel horizontally
            // (0:3:5 fixed point pixels)
            enum(DISTANCE_HORIZONTAL)

        endEnum()

        constant _MASK = %110
        assert(__ENUM__.last == _MASK)
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

