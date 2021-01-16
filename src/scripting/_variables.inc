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


constant N_SCRIPTS = Room.N_ROOM_SCRIPTS


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



// The index (within `scriptProgramCounters`) of the currently active script.
//
// NOTE: This value is only valid when a script is being processed.
//
// (word index)
allocate(currentScriptIndex, wram7e, 2)


// The program counter for each script
//
// If the value is 0, then the script is not active.
//
// (Room.N_ROOM_SCRIPTS * word indexes)
allocate(scriptProgramCounters, wram7e, 2 * N_SCRIPTS)


}

// vim: ft=bass-65816 ts=4 sw=4 et:

