# TODO

- [x] Handle minus numbers in table (put in hash)

- [ ] global table


## known bugs

### Table bug #1
### possible solution have a hashtable just for numbers with ints as keys, that way backlog could be removed
- [ ] Example: -10 can be accessed by doing "-10"
- [ ] Very large numbers can be accessed with a string 19231939012 == "19231939012"




next thing to do is make external functions work so i can actually test tables..........
basically {} should be converted to table_create() and table_create will be a external function
so {2,2,2} should be turned into table_create(2,2,2)
just use the same system as strings when creating the file