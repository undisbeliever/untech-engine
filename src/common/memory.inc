// common/memory.inc
// =================
//
// Handles ROM and RAM mapping.
//
// The following need to be defined before including this file:
//
//  * MEMORY_MAP - set to one of the values in the MEMORY_MAP scope
//  * ROM_SIZE   - the ROM size in Mbits


scope MEMORY_MAP {
    constant LOROM(0x20)
    constant HIROM(0x21)
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

scope private {
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
}


evaluate ::romBlockNames.count(0)

// Creates a new data block in a given ROM location.
//
// The data block is accessed with the `rodata` macro.
//
// PARAMS:
//  * id - the name of the block
//  * start - the starting address of the block
//  * end - the ending address of the block
macro createDataBlock(name, evaluate start, evaluate end) {
    if {defined ::MEMORY_FINALIZED} {
        error "Memory module has been finalized, cannot create Data Block"
    }

    if {defined ::romBlocks.{name}.start} {
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

    evaluate n(0)
    while {n} < {::romBlockNames.count} {
        define bn({::romBlockNames.{n}})

        if ({start} & 0x3fffff) <= ({::romBlocks.{bn}.end} & 0x3fffff) && ({end} & 0x3fffff) >= ({::romBlocks.{bn}.start} & 0x3fffff) {
            error "ROM Block `{name}` overlaps ROM block `{bn}`"
        }

        evaluate n({n} + 1)
    }

    global define ::romBlockNames.{::romBlockNames.count}({name})
    global evaluate ::romBlockNames.count({::romBlockNames.count} + 1)

    global evaluate ::romBlocks.{name}.pos({start})
    global evaluate ::romBlocks.{name}.start({start})
    global evaluate ::romBlocks.{name}.end({end})
}


// seeks to the next position in the selected block id
macro rodata(id) {
    if {defined ::MEMORY_FINALIZED} {
        error "Memory module has been finalized, cannot seek to Data Block"
    }

    if {defined ::romBlock.current} {
        if (pc() - 1) > {::romBlocks.{::romBlock.current}.end} {
            error "Overflow in rom block {::romBlock.current}"
        }

        global evaluate ::romBlocks.{::romBlock.current}.pos(pc())
    }

    global define ::romBlock.current({id})
    private.seek({::romBlocks.{id}.pos})

    if pc() >= {::romBlocks.{id}.end} {
        error "Overflow in rom block {id}"
    }
}

// duplicate of `createDataBlock`, written for clarity.
macro createCodeBlock(id, start, end) {
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
macro storeBlockStart(name, blockId) {
    constant {name}({::romBlocks.{blockId}.start})
}

// Creates a constant containing the total number bytes used by a block.
//
// This value is NOT the current number of bytes used. Instead the
// constant will contain the total number of bytes used by the block
// when `finalizeMemory` is called.
macro storeBlockUsed(name, blockId) {
    constant {name}(::romBlocks.{blockId}.last - {::romBlocks.{blockId}.start})
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
macro storeBlockCount(name, blockId, evaluate elementSize) {
    if {elementSize} <= 0 {
        error "Invalid elementSize"
    }

    constant {name}((::romBlocks.{blockId}.last - {::romBlocks.{blockId}.start}) / {elementSize})
}


evaluate ::romBlock.stackSize(0)

// pushes the current code/data block onto the stack
macro pushBlock() {
    global define ::romBlock.stack.{::romBlock.stackSize}({::romBlock.current})

    global evaluate ::romBlock.stackSize({::romBlock.stackSize} + 1)
}

// pulls the current code/data block from the stack
macro popBlock() {
    if {::romBlock.stackSize} <= 0 {
        error "No block stack"
    }

    global evaluate ::romBlock.stackSize({::romBlock.stackSize} - 1)

    rodata({::romBlock.stack.{::romBlock.stackSize}})
}


evaluate ::ramBlockNames.count(0)

// Create a new ram block with a given position and size.
//
// RAM blocks can overlap. This is to reuse memory in different contexts.
//
// The RAM block is accessed with the `allocate` macro.
//
// Information about the block can be accessed by the following defines:
//
//   * `::blocks.{id}.pos` -  the current position of the block
//   * `::blocks.{id}.size` -  the number of bytes assigned to the
//   * `::blocks.{id}.remaining` -  the number of bytes remaining in the block
//   * `::blocks.{id}.start` - the starting address of the block
//   * `::blocks.{id}.end` - the last address of the block
macro createRamBlock(id, evaluate start, evaluate end) {
    if {defined ::MEMORY_FINALIZED} {
        error "Memory module has been finalized, cannot create RAM Block"
    }

    if {defined ::blocks.{id}.pos} {
        error "RAM Block `{id}` already exists"
    }
    if {end} < {start} {
        error "RAM Block `{id}` cannot have end before start"
    }

    global define ::ramBlockNames.{::ramBlockNames.count}({id})
    global evaluate ::ramBlockNames.count({::ramBlockNames.count} + 1)

    global evaluate ::blocks.{id}.start({start})
    global evaluate ::blocks.{id}.end({end})

    global evaluate ::blocks.{id}.pos({start})
    global evaluate ::blocks.{id}.remaining({end} - {start} + 1)
    global evaluate ::blocks.{id}.size(0)

    global define blocks.{id}.size(::blocks.{id}.size)
}

// Allocate memory from the RAM block to a given constant name
macro allocate(name, blockId, evaluate size) {
    if {defined ::MEMORY_FINALIZED} {
        error "Memory module has been finalized, cannot allocate memory"
    }

    if !{defined ::blocks.{blockId}.size} {
        error "block {blockId} does not exist"
    }

    if ({::blocks.{blockId}.remaining} < {size}) {
        error "Cannot allocate {size} byte(s) from block {blockId}: Not enough space"
    }

    constant {name}({::blocks.{blockId}.pos})

    global evaluate ::blocks.{blockId}.pos({::blocks.{blockId}.pos} + {size})
    global evaluate ::blocks.{blockId}.size({::blocks.{blockId}.size} + {size})
    global evaluate ::blocks.{blockId}.remaining({::blocks.{blockId}.remaining} - {size})
}


// Finalizes the memory module.
//
// Prints memory usage if `SHOW_MEMORY_USAGE` is defined.
//
// This macro MUST BE called ONCE at the end of the program.
macro finalizeMemory() {
    if {defined ::MEMORY_FINALIZED} {
        error "Memory module has already been finalized"
    }
    global define ::MEMORY_FINALIZED(1)

    if {::romBlock.stackSize} != 0 {
        error "Block stack is not empty"
    }

    if {defined ::romBlock.current} {
        // update position of current block
        global evaluate ::romBlocks.{::romBlock.current}.pos(pc())
    }


    // build used constants
    evaluate n(0)
    while {n} < {::romBlockNames.count} {
        define bn({::romBlockNames.{n}})

        constant ::romBlocks.{bn}.last({::romBlocks.{bn}.pos})

        evaluate n({n} + 1)
    }


    if {defined SHOW_MEMORY_USAGE} {
        __printMemoryUsage()
    }
}

macro __printMemoryUsage() {
    print "\n"
    print "DATA USAGE:\n"
    print "\n"
    print "Address | Size    | Used    | Free    | Block Name\n"
    print "--------+---------+---------+---------+--------------------------\n"
    print " 00ff80 |      80 |      80 |       0 | SNES Header\n"

    evaluate totalSize(80)
    evaluate totalUsed(80)

    evaluate n(0)
    while {n} < {::romBlockNames.count} {
        define bn({::romBlockNames.{n}})

        evaluate size({::romBlocks.{bn}.end} - {::romBlocks.{bn}.start} + 1)
        evaluate used({::romBlocks.{bn}.pos} - {::romBlocks.{bn}.start})

        evaluate totalSize({totalSize} + {size})
        evaluate totalUsed({totalUsed} + {used})

        print " "
        print_hex(6, {::romBlocks.{bn}.start})
        print " | "
        print_padded(7, {size})
        print " | "
        print_padded(7, {used})
        print " | "
        print_padded(7, {size} - {used})
        print " | {bn}\n"

        evaluate n({n} + 1)
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

    evaluate n(0)
    while {n} < {::ramBlockNames.count} {
        define bn({::ramBlockNames.{n}})

        print " "
        print_hex(6, {::blocks.{bn}.start})
        print " | "
        print_padded(7, {::blocks.{bn}.end} - {::blocks.{bn}.start} + 1)
        print " | "
        print_padded(7, {::blocks.{bn}.size})
        print " | "
        print_padded(7, {::blocks.{bn}.remaining})
        print " | {bn}\n"

        evaluate n({n} + 1)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

