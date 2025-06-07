// scripting/_variables.inc
// ========================
//
// Variables used by the scripting subsystem.
//
//
// SPDX-FileCopyrightText: © 2021 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2021 Marcus Rowe <undisbeliever@gmail.com>
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
allocate(programCounter, zeropage, 2)


constant __VariablesBegin = __MEMORY__.ramBlocks.wram7e.pos


// Non-zero if the room startup script is active.
//
// (byte flag)
allocate(startupScriptActive, wram7e, 1)


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
//
// NOTE: Scripts will not be activated if `startupScriptActive` is set.
// Instead they will be activated after the room startup script ends.
namespace scripts {
    constant INDEX_INCREMENT = 2

    // The script Id of the active script
    // (byte * Room.N_ROOM_SCRIPTS)
    allocate(scriptId, wram7e, 2 * MAX_ACTIVE_SCRIPTS)


    // The resume function to execute when the script is processed.
    // Index into `BytecodeResumeFunctionTable`.
    //
    // If the script is not active, then `resumeOpcode` is zero.
    //
    // When a script is first activated `resumeOpcode` will be set to a special
    // resume instruction that will start normal script execution.
    //
    // (byte index * Room.N_ROOM_SCRIPTS)
    constant resumeOpcode = scriptId + 1


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


    assertOnePage(scriptId,       2 * MAX_ACTIVE_SCRIPTS)
    assertOnePage(resumeOpcode,   2 * MAX_ACTIVE_SCRIPTS)
    assertOnePage(programCounter, 2 * MAX_ACTIVE_SCRIPTS)
    assertOnePage(resumeArg1,     2 * MAX_ACTIVE_SCRIPTS)
    assertOnePage(resumeArg2,     2 * MAX_ACTIVE_SCRIPTS)
}



// Script active flags.
//
// If the room startup script is active, this array is used to mark which
// scripts are to be activated after the startup script ends.
//
// (byte flag * Room.N_ROOM_SCRIPTS)
allocate(activeScripts, wram7e, Room.N_ROOM_SCRIPTS)
assertOnePage(activeScripts, Room.N_ROOM_SCRIPTS)



namespace BackgroundTasks {

    // Status of the currently running (if any) background task.
    //
    //   cr000000
    //      c = set if the task is completed
    //      r = set if the task is running
    //      0 = no task is running or waiting for acknowledgement
    //
    // (2 bit flags)
    allocate(taskStatus, wram7e, 1)
    constant taskStatus.TASK_COMPLETED_FLAG       = 0x80
    constant taskStatus.TASK_RUNNING_FLAG         = 0x40
    constant taskStatus.READY_TO_START_A_NEW_TASK = 0


    // Function Table index for the current background task
    //
    // This variable is 0 if no background tasks are running.
    //
    // The high byte must always be 0.
    //
    // (byte index into BackgroundTasksFunctionTable)
    allocate(backgroundTask, wram7e, 2)


    // State variables used by the background task
    // (state.size bytes of data)
    constant state.size = 16
    allocate(state, wram7e, state.size)
}


constant __VariablesEnd = __MEMORY__.ramBlocks.wram7e.pos

}

// vim: ft=bass-65816 ts=4 sw=4 et:

