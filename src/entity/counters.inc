
scope Entity {

// Counts the number of enemies of a specific Id
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A = entity Id
// OUTPUT: A = number of matching entities
a16()
i16()
code()
scope CountEnemiesWithId: {
allocateTmpWord(tmp_count)

    asl
    tax
    lda.l   Entity.Data.List,x
    // A = entity Id

    ldx.w   Entity.lists.enemies
    beq     EmptyList

    stz.w   tmp_count

    Loop:
        cmp.w   BaseEntity.romPtr,x
        bne     +
            inc.w   tmp_count
        +

        txy
        ldx.w   BaseEntity.next,y
        bne     Loop

    lda.w   tmp_count
    rts

EmptyList:
    lda.w   #0
    rts
}



// Counts the number of enemy entities active
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// OUTPUT: A = number of entities in enemy list
a16()
i16()
code()
scope CountActiveEnemies: {
    lda.w   #0
    ldx.w   lists.enemies
    beq     EndLoop

    Loop:
        inc
        txy
        ldx.w   BaseEntity.next,y
        bne     Loop

EndLoop:
    rts
}



// Counts the number of entities in the given list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: X = list Id
// OUTPUT: A = number of entities in enemy list
macro CountEntitiesInList(listId) {
    assert16a()
    assert16i()

    ldx.w   #Entity.lists.{listId}
    jsr     Entity.CountEntitiesInList
}



// Counts the number of entities in the given list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: X = address of list
// OUTPUT: A = number of entities in enemy list
a16()
i16()
code()
scope CountEntitiesInList: {
    lda.w   #0
    ldy.w   0,x
    beq     EndLoop

    Loop:
        inc
        txy
        ldx.w   BaseEntity.next,y
        bne     Loop

EndLoop:
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

