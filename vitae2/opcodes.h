typedef enum opcode {
    nop,
    halt,
    pop,
    pushc,
    add,
    sub,
    mul,
    _div,
    mod,
    get,
    set,
    dset,
    dget,
    dpushc,
    dsetc,
    dgetc,
    
    dstorec,
    dloadc,
    /* comparison */
    eql,
    nql,
    grt,
    lss,

    table_access,
    table_alloc,
    table_set,
   // table_destroy,
  //  table_push,
  //  table_pop,
    
    /* Jump */
    jmp, /*goto (int)*/
    jmpr, /*relative to position (int) //? should be faster since it does 1 less calculation*/
    jmpf, /*goto on condition false (int)*/
    jmpt, /*goto on condition true (int)*/


    alloc,
    concat,
    /*function stuff*/
    call,
    call_extern,
    ret,
    retv, /*return value*/

    
    opcode_count,
}opcode_e;