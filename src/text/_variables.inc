
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

    // A temporary string to print
    // (wram7e)
    constant tmpString.size(12)
    allocate(tmpString, wram7e, tmpString.size)
}

// vim: ft=asm ts=4 sw=4 et:

