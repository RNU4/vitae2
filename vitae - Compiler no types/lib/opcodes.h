typedef enum opcode {
    halt,
    push,

    /*how many bits to pop and push*/
    pop8,
    pop16,
    pop32,
    pop64,

    push8,
    push16,
    push32,
    push64,


    /* pointer & stack*/
    set, /*set a pointer to a value*/
    get, /*return value of pointer*/
    _malloc,
    _calloc,
    _free,

    set8,set16,set32,set64,
    get8,get16,get32,get64,
    sp_set, /*set a value, sp + offset = value*/
    sp_get, /*get a value, sp = sp + offset*/
    
    dp_set8,dp_set16,dp_set32,dp_set64,

    dp_get8,dp_get16,dp_get32,dp_get64,

    dp_set_const8,dp_set_const16,dp_set_const32,dp_set_const64,

    dp_get_const8,dp_get_const16,dp_get_const32,dp_get_const64,

    /*return will be pointer size, input is 32bits?*/
    dp_get_adress,

    /*
    // table manipulation only 64bit (for more high level languages) 
    table_insert_value, //(value 64bit),(key string),(id 64bit)
    table_delete_key, //(key string),(id 64bit)
    table_get_from_id,  
    table_get_value,    //(key string),(id 64bit) returns (value 64bit)
    table_create,
    table_destroy,

    // micro table manipulation no restrictions (less safe) 
    micro_table_insert_value, //(id 64bit),(value any_bitsize),(length 64bit)(key string)
    micro_table_get_value_pointer_from_key, 
    micro_table_get_value,    //(id 64bit),(key string) returns (value any_bitsize)
    micro_table_create,
    */
   
    /* math stuff */
    /*for 64bit int*/
    addi8, subi8, muli8, divi8,
    addi16,subi16,muli16,divi16,
    addi32,subi32,muli32,divi32,
    addi64,subi64,muli64,divi64,

    addu8, subu8, mulu8, divu8,
    addu16,subu16,mulu16,divu16,
    addu32,subu32,mulu32,divu32,
    addu64,subu64,mulu64,divu64,
   
    /*for 64bit float*/
    addf32,subf32,mulf32,divf32,
    addf64,subf64,mulf64,divf64,


    /*more precise conversion*/
    //! is this really needed? could you not just cut off or add x bytes? and it should work

    /*Signed & unsigned (//? is it even needed?)*/
    /*
    i8tou16,    i16tou8,    i32tou8,     i64tou8,
    i8tou32,    i16tou32,   i32tou16,    i64tou16,
    i8tou64,    i16tou64,   i32tou64,    i64tou32,

    u8toi16,    u16toi8,    u32toi8,     u64toi8,
    u8toi32,    u16toi32,   u32toi16,    u64toi16,
    u8toi64,    u16toi64,   u32toi64,    u64toi32,
    */
    /*Floats these are needed*/
    i8tof32,    i16tof32,   i32tof32,    i64tof32,
    i8tof64,    i16tof64,   i32tof64,    i64tof64,

    u8tof32,    u16tof32,   u32tof32,    u64tof32,
    u8tof64,    u16tof64,   u32tof64,    u64tof64,

    f32toi32,   f32tou32,   f32toi64,    f32tou64,
    f64toi32,   f64tou32,   f64toi64,    f64tou64,

    f32tof64,f64tof32, //cut off might work?

    b8to16,   b16to8,   b32to8,    b64to8,
    b8to32,   b16to32,  b32to16,   b64to16,
    b8to64,   b16to64,  b32to64,   b64to32,

    /* Jump */
    jmp, /*goto (int)*/
    jmpr, /*relative to position (int) //? should be faster since it does 1 less calculation*/
    jmpc, /*goto on condition (int)*/
    jmpcr, /*goto on condition (int)*/
    jmpp, /*goto (pointer int)*/

    alloc,

    /*function stuff*/
    call,
    ret,
    retc, /*return with "varibles"*/


    /*SUPA DUPA OPCODES++*/

    // dp get const dp get const add
    S_DPGETCX2ADD_I8,S_DPGETCX2ADD_I16,
    S_DPGETCX2ADD_I32,S_DPGETCX2ADD_I64,
    S_DPGETCX2ADD_U8,S_DPGETCX2ADD_U16,
    S_DPGETCX2ADD_U32,S_DPGETCX2ADD_U64,
    // dp get const push number add
    S_DPGETCPUSHADD_I8,S_DPGETCPUSHADD_I16,
    S_DPGETCPUSHADD_I32,S_DPGETCPUSHADD_I64,
    S_DPGETCPUSHADD_U8,S_DPGETCPUSHADD_U16,
    S_DPGETCPUSHADD_U32,S_DPGETCPUSHADD_U64,
    
    S_LSSJMPCR_I32,
    S_GRTJMPCR_I32,

    S_LSSJMPCR_U32,
    S_GRTJMPCR_U32,
    S_EQLJMPCR_U32,
    S_NQLJMPCR_U32,

    // S_PUSH_PUSH_8 would just be push16
    // S_PUSH_PUSH_16 would just be push32
    // S_PUSH_PUSH_32 would just be push64
    S_PUSH_PUSH_64,// push push

    S_PUSH_PUSH_DPSET8,S_PUSH_PUSH_DPSET16, // x = 4
    S_PUSH_PUSH_DPSET32,S_PUSH_PUSH_DPSET64,// x = 4

    /* comparison */
    //? ecl & nql could simplified? the type should not matter in theory and frees 8 opcodes
    eqlu8,eqli8,eqlu16,eqli16,eqlu32,eqli32,eqlu64,eqli64,
    nqlu8,nqli8,nqlu16,nqli16,nqlu32,nqli32,nqlu64,nqli64,
    grtu8,grti8,grtu16,grti16,grtu32,grti32,grtu64,grti64,
    lssu8,lssi8,lssu16,lssi16,lssu32,lssi32,lssu64,lssi64,
    
    eqlf32,eqlf64,
    nqlf32,nqlf64,
    grtf32,grtf64,
    lssf32,lssf64,

    /*future stuff*/
    vthread, // ? is it possible with jump labels idk(it is but perfomance will decreaes)
    thread, // ? use real thread to create new instance
    call_ex, // ? call external function will be used to give functionality outside being a glorified calculator
    opcode_count,
}opcode_e;