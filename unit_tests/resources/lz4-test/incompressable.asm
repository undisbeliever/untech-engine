
variable i = 0
while i < 0x100 {
    db i
    db i
    db i
    db i

    i = i + 1
}

// vim: ft=bass-65816 ts=4 sw=4 et:

