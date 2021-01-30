// scripting/_variables.inc
// ========================
//
// Variables used by the scripting subsystem.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Scripting {


constant MAX_ACTIVE_SCRIPTS = Room.N_ROOM_SCRIPTS


// Script data is stored inside the roomData
// (wram 7E data block)
constant scriptData = Room.roomData
constant scriptData.size = Room.roomData.size



// Temporary placeholder for the program counter of the currently active script.
//
// NOTE: This variable is only set if required.  It will not be used if
//       the bytecode does not need to save the `programCounter` register (Y).
//
// (word addr)
allocate(programCounter, dp, 2)


constant __VariablesBegin = __MEMORY__.ramBlocks.wram7e.pos


// The index (within the `scripts` SoA) of the currently active script.
//
// NOTE: This value is only valid when a script is being processed.
//
// (word index)
allocate(currentScriptIndex, wram7e, 2)


// Index at the end of the active `scripts` SoA.
//
// This value is equal to `2 * number of active scripts` (index past the last element).
//
// (word index)
allocate(activeScriptsEnd, wram7e, 2)


// Structure of Arrays for active script data
namespace scripts {
    // The resume function to execute when the script is processed.
    // Index into `BytecodeResumeFunctionTable`.
    //
    // If the script is not active, then `resumeOpcode` is zero.
    //
    // When a script is first activated `resumeOpcode` will be set to a special
    // resume instruction that will start normal script execution.
    //
    // (byte index * Room.N_ROOM_SCRIPTS)
    allocate(resumeOpcode, wram7e, 2 * MAX_ACTIVE_SCRIPTS)

    // The script Id of the active script
    // (byte * Room.N_ROOM_SCRIPTS)
    constant scriptId = resumeOpcode + 1


    // The program counter for the script
    //
    // If the value is 0, then the script is not active.
    //
    // (word index * Room.N_ROOM_SCRIPTS)
    allocate(programCounter, wram7e, 2 * MAX_ACTIVE_SCRIPTS)


    // Data storage for the resume function.
    // (word * Room.N_ROOM_SCRIPTS)
    allocate(resumeArg1, wram7e, 2 * MAX_ACTIVE_SCRIPTS)


    // Data storage for the resume function.
    // (word * Room.N_ROOM_SCRIPTS)
    allocate(resumeArg2, wram7e, 2 * MAX_ACTIVE_SCRIPTS)
}


// Script active flags
// (byte flag * Room.N_ROOM_SCRIPTS)
allocate(activeScripts, wram7e, Room.N_ROOM_SCRIPTS)


constant __VariablesEnd = __MEMORY__.ramBlocks.wram7e.pos

}

// vim: ft=bass-65816 ts=4 sw=4 et:

