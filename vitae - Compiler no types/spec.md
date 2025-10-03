
```
function main(int y) int:

    int x = y

    if x == 2: 
        return true
    else
        return false
    end

end

```




```
Handle bytecode strings
just push them to stack and refrence them👍

```


### How to handle tokens to bytecode asm 😫
#### Example
``` // so basically left side u need adress and right side u need value
int x = 1
if x == 1:
    x = 3
end

/* predefine by not adding the ':' */
function add(int a, int b) int

int y = add(2,3)

function add(int a, int b) int:
    return a+b
end

function add(int a, int b) int:
    return a+b
end

int x = 1

if x == 1:
    x = 3
end

int y = add(2,3)




```


```
alloc 4*4

push32 dp+0
dp_setconst32,0 //x = 0

push32 9
push32 2
push32 x
call index 8 // calls index and pops 8 bytes from stack (2 32bit variables)
dp_set32

$index
    dp_get_const_32,-sizeof(arg0)
    dp_get_const_32,-sizeof(arg0)-sizeof(arg1)
    addu32
    dp_set_const_x,0
retc,4 // return 4 bytes

```
#### functions declaration (inlang)



```
function add (int a,int b) int
    return a + b
end
```
#### functions declaration (bytecode)
```
$add
dp_get_const_x,-sizeof(arg0)
dp_get_const_x,-sizeof(arg0)-sizeof(arg1)
add_x
dp_set_const_x,0
retc,x
```
#### functions declaration alternative (bytecode)
```
$add
S_DPGETCX2ADD_U32,-sizeof(arg0),-sizeof(arg0)-sizeof(arg1)
dp_set_const_x,0
retc,x
```


#### conversion (inlang)
```
block data_block {
    int x,
    int y
}



data_block* y = data_block(malloc(sizeof(data_block)))

y.x = 2
y.y = 3
```

