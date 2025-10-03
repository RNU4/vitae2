## Todo

- [x] check if `*` is a pointer
  - check last token and make sure it's not an IDENTIFIER or a FUNCTION  
  - also make `*` into a `$` so it's easier to parse
- [x] ensure right order by converting to reverse polish notation
- [x] handle pointers correctly
- [x] handle strings
- [x] handle tables
- [x] integrate garbage collector
- [x] allow extern functions from C


## Low Priority Todo

- [ ] handle making table templates
    - for example `tb = {.x = "hello", .y = 42, .c = {42,123,43} }`
- [ ] allow importing files
- [x] interface for interacting with the vm
- [ ] optimize VM performance
- [ ] add detailed logging
- [ ] improve error messages

## Possible bugs

- [x] `assembler_handle_function_definition()` variable_name does not copy string  
      so if the token is freed it can cause a crash later on

## Bugs
- [x] Does or does not allocate space for return variable if it is not used  
      (either way it's not good)
- [x] Does not check if a function exists if it's part of an expression
- [ ] No error if a function known to have returns does not end with a return
- [x] No error if a function with no return is used as one with a return
- [ ] Error if a function with no return is used as standalone call

- [x] Arguments are in reversed order  
      - example: `fn(a,b) return a+b` is compiled like `fn(b,a) return a+b`
- [x] If & else labels don’t take into account `stack_offset`
- [x] No error if a function is given too many arguments or too few
- [ ] Expressions like `-13+4`, `13+(-22)`, `a=-4` produce wrong code generation  
      (possibly fixed now)
- [x] `]=` gets turned into a single token instead of `]` `=`

## possible improvement
put all the const strings at start of the stack and access them that way









## asserts
Assertion failed: ftoken_next != NULL, file assembler.h, line 397