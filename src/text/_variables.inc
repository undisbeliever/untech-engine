
scope Text {

    // The buffer in which to hold the data to transfer to VRAM
    //
    // This buffer is only loaded to the low byte of VRAM in order
    // to save space and DMA time
    //
    // (whatever the format)
    constant buffer.size(32 * 32)
    allocate(buffer, wram7e, buffer.size)

    // The VRAM word address to store the buffer to
    // (shadow)
    allocate(bufferVramWaddr, shadow, 2)

    // If non-zero then the buffer is transferred to VRAM on vblank
    // (dp)
    allocate(bufferDirty, dp, 1)

    // Variables used by the different text modes
    // (dp)
    constant vars.size(4)
    allocate(vars, dp, vars.size)

    // string pointer 1
    // (dp far addr)
    allocate(str1, dp, 3)

    // string pointer 2
    // (dp far addr)
    allocate(str2, dp, 3)

    // A temporary buffer used for string generation
    // (wram7e)
    constant tmpString.size(12)
    allocate(tmpString, wram7e, tmpString.size)

    // Padding character
    // (wram7e)
    allocate(paddingChar, wram7e, 1)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

