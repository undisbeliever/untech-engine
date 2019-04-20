// common/memory.inc
// =================
//
// Handles ROM and RAM mapping.
//
// The following need to be defined before including this file:
//
//  * MEMORY_MAP - set to one of the values in the MEMORY_MAP scope
//  * ROM_SIZE   - the ROM size in Mbits
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MEMORY_MAP {
    constant LOROM = 0x20
    constant HIROM = 0x21
}

if !{defined MEMORY_MAP} {
    error "MEMORY_MAP is not defined"
}

if !{defined ROM_SIZE} {
    error "ROM_SIZE is not defined (value is in Mbits)"
}
if {ROM_SIZE} > 32 {
    error "ROM_SIZE > 32 Mbits is not supported"
} else if {ROM_SIZE} < 1 {
    error "ROM_SIZE is invalid"
}

namespace private {
    if MEMORY_MAP.{MEMORY_MAP} == MEMORY_MAP.LOROM {
        macro seek(offset) {
            origin (({offset} & 0x3f0000) >> 1) | ({offset} & 0x7fff)
            base {offset}
        }

        macro checkBlockIsValid(name, start, end) {
            if (({end} & 0x3f0000) >> 1 | ({end} & 0x7fff)) > ({ROM_SIZE} * 0x20000) {
                error "ROM block `{name}` is outside ROM_SIZE"
            }
            if ({start} & 0xFFFF) < 0x8000 {
                error "ROM Block `{name}` is not in ROM area"
            }
        }

    } else if MEMORY_MAP.{MEMORY_MAP} == MEMORY_MAP.HIROM {
        macro seek(offset) {
            origin {offset} & 0x3fffff
            base {offset}
        }

        macro checkBlockIsValid(name, start, end) {
            if ({end} & 0x3fffff) > ({ROM_SIZE} * 0x20000) {
                error "ROM block `{name}` is outside ROM_SIZE"
            }
            if (({start} & 0x7fffff) < 0x3f0000) && (({start} & 0xFFFF) < 0x8000) {
                error "ROM Block `{name}` is not in ROM area"
            }
        }
    } else {
        error "Unknown memory map `{MEMORY_MAP}`"
    }

    macro checkBlockIsUnused(evaluate start, evaluate end) {
        evaluate n = 0
        while {n} < {__MEMORY__.romBlockNames.count} {
            define b = __MEMORY__.romBlocks.{__MEMORY__.romBlockNames.{n}}

            if ({start} & 0x3fffff) <= ({b}.end & 0x3fffff) && ({end} & 0x3fffff) >= ({b}.start & 0x3fffff) {
                error "ROM Block `{name}` overlaps ROM block `{bn}`"
            }

            evaluate n = {n} + 1
        }
    }
}


// Pad output file to ROM_SIZE
origin {ROM_SIZE} * 1024 * 1024 / 8 - 1
db 0
origin 0


// When set to non-zero then no more memory can be created or allocated.
variable __MEMORY__.finalized = 0

// Number of ROM Blocks defined in the `__MEMORY__.romBlockNames.<n>` defines.
//
// It is a define because editing of romBlockNames is only done on the root scope.
evaluate __MEMORY__.romBlockNames.count = 0


// Creates a new data block in a given ROM location.
//
// The data block is accessed with the `rodata` macro.
//
// This macro MUST BE called in the root scope.
//
// PARAMS:
//  * id - the name of the block
//  * start - the starting address of the block
//  * end - the ending address of the block
inline createDataBlock(name, evaluate start, evaluate end) {
    if __MEMORY__.finalized {
        error "Memory module has been finalized, cannot create Data Block"
    }

    if {defined __MEMORY__.romBlocks.{name}.start} {
        error "ROM Block `{name}` already exists"
    }

    private.checkBlockIsValid({name}, {start}, {end})

    if {end} < {start} {
        error "ROM Block `{name}` cannot have end before start"
    }
    if ({start} >> 16) != ({end} >> 16) {
        error "ROM Block `{name}` can not cross data banks"
    }
    if ({start} & 0x3fffff) <= 0x00ffff && ({end} & 0x3fffff) >= 0x00ffb0 {
        error "ROM Block `{name}` cannot touch the SNES Header"
    }

    private.checkBlockIsUnused({start}, {end})

    define __MEMORY__.romBlocks.{name}
    variable __MEMORY__.romBlocks.{name}.pos = {start}
    constant __MEMORY__.romBlocks.{name}.index = {__MEMORY__.romBlockNames.count}
    constant __MEMORY__.romBlocks.{name}.start = {start}
    constant __MEMORY__.romBlocks.{name}.end = {end}

    define __MEMORY__.romBlockNames.{__MEMORY__.romBlockNames.count} = {name}
    evaluate __MEMORY__.romBlockNames.count = {__MEMORY__.romBlockNames.count} + 1
}


// Index of the current ROM block.
variable __MEMORY__.currentRomBlockIndex = -1

// seeks to the next position in the selected block id
macro rodata(id) {
    if __MEMORY__.finalized {
        error "Memory module has been finalized, cannot seek to Data Block"
    }

    if !{defined __MEMORY__.romBlocks.{id}} {
        error "Rom block {id} does not exist"
    }

    if __MEMORY__.currentRomBlockIndex >= 0 {
        evaluate cbi = __MEMORY__.currentRomBlockIndex
        define cb = {__MEMORY__.romBlockNames.{cbi}}

        if (pc() - 1) > __MEMORY__.romBlocks.{cb}.end {
            error "Overflow in rom block {cb}"
        }
        __MEMORY__.romBlocks.{cb}.pos = pc()
    }

    __MEMORY__.currentRomBlockIndex = __MEMORY__.romBlocks.{id}.index
    private.seek(__MEMORY__.romBlocks.{id}.pos)

    if pc() >= __MEMORY__.romBlocks.{id}.end {
        error "Overflow in rom block {id}"
    }
}

// duplicate of `createDataBlock`, written for clarity.
inline createCodeBlock(id, start, end) {
    createDataBlock({id}, {start}, {end})
}

// duplicate of `rodata` written for clarity.
macro code(id) {
    rodata({id})
}

macro code() {
    code(code)
}


// Creates a constant containing the starting address of a block
inline storeBlockStart(name, blockId) {
    constant {name} = __MEMORY__.romBlocks.{blockId}.start
}

// Creates a constant containing the total number bytes used by a block.
//
// This value is NOT the current number of bytes used. Instead the
// constant will contain the total number of bytes used by the block
// when `finalizeMemory` is called.
inline storeBlockUsed(name, blockId) {
    constant {name} = __MEMORY__.romBlocks.{blockId}.last - __MEMORY__.romBlocks.{blockId}.start
}

// Creates a constant containing the final number of elements in a
// fixed-sized table block
//
// {name} will contain the value of (<blockUsed> / <elementSize>)
//
// This value is NOT the current block count. Instead the constant will
// contain the total number of elements in the block when
// `finalizeMemory` is called.
// program.
inline storeBlockCount(name, blockId, evaluate elementSize) {
    if {elementSize} <= 0 {
        error "Invalid elementSize"
    }

    constant {name} = (__MEMORY__.romBlocks.{blockId}.last - __MEMORY__.romBlocks.{blockId}.start) / {elementSize}
}


constant __MEMORY__.blockStack.max = 4
variable __MEMORY__.blockStack.size = 0
variable __MEMORY__.blockStack.0 = -1
variable __MEMORY__.blockStack.1 = -1
variable __MEMORY__.blockStack.2 = -1
variable __MEMORY__.blockStack.3 = -1

// pushes the current code/data block onto the stack
macro pushBlock() {
    evaluate stackIndex = __MEMORY__.blockStack.size
    __MEMORY__.blockStack.{stackIndex} = __MEMORY__.currentRomBlockIndex

    __MEMORY__.blockStack.size = __MEMORY__.blockStack.size + 1

    if __MEMORY__.blockStack.size >= __MEMORY__.blockStack.max {
        error "block stack is too deep"
    }
}

// pulls the current code/data block from the stack
macro popBlock() {
    if __MEMORY__.blockStack.size <= 0 {
        error "No block stack"
    }

    __MEMORY__.blockStack.size = __MEMORY__.blockStack.size - 1

    evaluate stackIndex = __MEMORY__.blockStack.size
    evaluate blockIndex = __MEMORY__.blockStack.{stackIndex}
    rodata({__MEMORY__.romBlockNames.{blockIndex}})
}



// Number of ROM Blocks defined in the `__MEMORY__.ramBlockNames.<n>` defines.
//
// It is a define because editing of ramBlockNames is only done on the root scope.
evaluate __MEMORY__.ramBlockNames.count = 0

// Create a new ram block with a given position and size.
//
// This macro MUST BE called in the root scope.
//
// RAM blocks can overlap. This is to reuse memory in different contexts.
//
// The RAM block is accessed with the `allocate` macro.
//
// Information about the block can be accessed by the following
// constants and variables:
//
//   * `__MEMORY__.ramBlocks.{id}.pos` -  the current position of the block
//   * `__MEMORY__.ramBlocks.{id}.size` -  the number of bytes assigned to the
//   * `__MEMORY__.ramBlocks.{id}.remaining` -  the number of bytes remaining in the block
//   * `__MEMORY__.ramBlocks.{id}.start` - the starting address of the block
//   * `__MEMORY__.ramBlocks.{id}.end` - the last address of the block
inline createRamBlock(id, evaluate start, evaluate end) {
    if __MEMORY__.finalized {
        error "Memory module has been finalized, cannot create RAM Block"
    }

    if {defined __MEMORY__.ramBlocks.{id}} {
        error "RAM Block `{id}` already exists"
    }
    if {end} < {start} {
        error "RAM Block `{id}` cannot have end before start"
    }

    define __MEMORY__.ramBlockNames.{__MEMORY__.ramBlockNames.count} = {id}
    evaluate __MEMORY__.ramBlockNames.count = {__MEMORY__.ramBlockNames.count} + 1

    define __MEMORY__.ramBlocks.{id}
    constant __MEMORY__.ramBlocks.{id}.start = {start}
    constant __MEMORY__.ramBlocks.{id}.end = {end}

    variable __MEMORY__.ramBlocks.{id}.pos = {start}
    variable __MEMORY__.ramBlocks.{id}.remaining = {end} - {start} + 1
    variable __MEMORY__.ramBlocks.{id}.size = 0
}

// Allocate memory from the RAM block to a given constant name
inline allocate(name, blockId, evaluate size) {
    if __MEMORY__.finalized {
        error "Memory module has been finalized, cannot allocate memory"
    }

    if !{defined __MEMORY__.ramBlocks.{blockId}} {
        error "block {blockId} does not exist"
    }

    if (__MEMORY__.ramBlocks.{blockId}.remaining < {size}) {
        error "Cannot allocate {size} byte(s) from block {blockId}: Not enough space"
    }

    constant {name} = __MEMORY__.ramBlocks.{blockId}.pos

    __MEMORY__.ramBlocks.{blockId}.pos = __MEMORY__.ramBlocks.{blockId}.pos + {size}
    __MEMORY__.ramBlocks.{blockId}.size = __MEMORY__.ramBlocks.{blockId}.size + {size}
    __MEMORY__.ramBlocks.{blockId}.remaining = __MEMORY__.ramBlocks.{blockId}.remaining - {size}
}


// Finalizes the memory module.
//
// Prints memory usage if `SHOW_MEMORY_USAGE` is defined.
//
// This macro MUST BE called ONCE at the end of the program.
inline finalizeMemory() {
    // update position of current block
    rodata(code)

    if __MEMORY__.finalized {
        error "Memory module has already been finalized"
    }
    __MEMORY__.finalized = 1

    if __MEMORY__.blockStack.size != 0 {
        error "Block stack is not empty"
    }

    _finalizeMemory_buildUseConstants(0)

    if {defined SHOW_MEMORY_USAGE} {
        __printMemoryUsage()
    }
}

inline _finalizeMemory_buildUseConstants(evaluate n) {
    // build used constants
    if {n} < {__MEMORY__.romBlockNames.count} {
        constant __MEMORY__.romBlocks.{__MEMORY__.romBlockNames.{n}}.last = __MEMORY__.romBlocks.{__MEMORY__.romBlockNames.{n}}.pos
        _finalizeMemory_buildUseConstants({n} + 1)
    }
}


macro __printMemoryUsage() {
    print "\n"
    print "DATA USAGE:\n"
    print "\n"
    print "Address | Size    | Used    | Free    | Block Name\n"
    print "--------+---------+---------+---------+--------------------------\n"
    print " 00ff80 |      80 |      80 |       0 | SNES Header\n"

    evaluate totalSize = 80
    evaluate totalUsed = 80

    evaluate n = 0
    while {n} < {__MEMORY__.romBlockNames.count} {
        define blockName = {__MEMORY__.romBlockNames.{n}}
        define block = __MEMORY__.romBlocks.{blockName}

        evaluate size = {block}.end - {block}.start + 1
        evaluate used = {block}.pos - {block}.start

        evaluate totalSize = {totalSize} + {size}
        evaluate totalUsed = {totalUsed} + {used}

        print " "
        print_hex(6, {block}.start)
        print " | "
        print_padded(7, {size})
        print " | "
        print_padded(7, {used})
        print " | "
        print_padded(7, {size} - {used})
        print " | {blockName}\n"

        evaluate n = {n} + 1
    }

    print "========+=========+=========+=========+==========================\n"

    print " TOTAL  | "
    print_padded(7, {totalSize})
    print " | "
    print_padded(7, {totalUsed})
    print " | "
    print_padded(7, {totalSize} - {totalUsed})
    print "\n"


    print "\n"
    print "\n"
    print "RAM USAGE:\n"
    print "\n"
    print "Address | Size    | Used    | Free    | Block Name\n"
    print "--------+---------+---------+---------+--------------------------\n"

    // Don't print totals for RAM blocks as they may overlap

    evaluate n = 0
    while {n} < {__MEMORY__.ramBlockNames.count} {
        define blockName = {__MEMORY__.ramBlockNames.{n}}
        define block = __MEMORY__.ramBlocks.{blockName}

        print " "
        print_hex(6, {block}.start)
        print " | "
        print_padded(7, {block}.end - {block}.start + 1)
        print " | "
        print_padded(7, {block}.size)
        print " | "
        print_padded(7, {block}.remaining)
        print " | {blockName}\n"

        evaluate n = {n} + 1
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

