# Liquid #
Liquid is a scripting language.

# Why #
After reading an article about LLVM and how easy it is to create a DSL with it, I created this project (Sadly, I have no link the article itself). That was something I wanted to try. I started with some simple programming language constructs (and with no real concept of what the language should be). Additionally, I wanted to figure out how usable flex and bison are, since I didn't want to develop my own parser and lexer. All things which I had in my mind for a long time but never took the time to do. With LLVM it seams to be a feasible task. Just combining flex/bison with LLVM, voilà, that should be simple, right? However, while defining the syntax, more and more ideas of language elements that would be nice to have came up (thanks to @lucasb-eyer for the inspiring discussions). It resulted in a mix of some popular languages like python (at least the block formatting), ruby, some functional aspects and there may be others. Turns out that it took more time and effort than I thought. But it was still fun. 

# Installation #

## Prerequisite ##
- LLVM 19.0 
- flex and bison.
- CMake 3.12 

## Build ##
1. Clone this repository.
```
git clone https://github.com/xkbeyer/liquid.git
```
2. Run cmake and generate the build files into the build directory `build`.
```
cmake -B build -S .
```

### Optional arguments

| Define Name | Description               |
| ---------- | ------------------------- |
| LLVM_ROOT  | path to llvm installation |
| BISON_ROOT | path to bison             |
| FLEX_ROOT  | path to flex              |

These are helpful in case cmake can't find the packages.

```
cmake -B build -S . -DLLVM_ROOT=/path/to/llvm -DBISON_ROOT=/path/to/bison -DFLEX_ROOT=/path/to/flex
```

Make sure all variables are set properly and then generate the build.
One can use `ccmake` or `cmake-gui` on windows to change or check the variables.

3. Run the make
```
cmake --build build
```
or
```
cmake --build build --target install
```

### Windows ###
After cmake was run the solution file is in the build directory. Start Visual Studio and you are ready to compile it.

# Usage #
```
liq script-file -h -d -v -q -ipath1;path2...;pathn
```
where
- h help: shows the usage.
- d debug: Disables the code optimizer. 
- v verbose: print a lot of information.
- q quiet: don't show any output. 
- i defines a list of additional path to look for files to import.

Liquid does parse the file, generates the code in memory and runs it.

**Examples**
```
./liq test.liq
./liq test.liq -q
./liq test.liq -v -d -i import/path;import/other/path
```

# Language Syntax #
## Literals ##
Can be any literal word including -_%$? and digits.
It must start with a letter or an underscore.
```
a-b
otto4all
price?
_makeNew!
```
This can be used for symbols as variable names, function name etc.

## Strings ##
A string is enclosed in `"` or `'`. A quote in a string must be escaped by a `\`.
```
"This is a string."
"This is
a multiple
line string."
'The man said:"This is a string." and left the place.'
"But \"here\" we need them."
```

## Numbers ##
Simple Integer numbers, decimal numbers.
Currently no binary and hex format is supported.
```
5
230
-88
27.5
0.345
.1
-1234.56
1.2e2
1.2e+2
1.2e-2
```

## Boolean ##
A boolean can take the symbol `true` or `false`. 

## Array ##
An array is a container which can hold elements of arbitrary types.
```
var array = [1,2,3,4]
```
The elements don't have to be of the same type.
```
var list = [1+2, 'Otto', true]
```
_Hint:_ Currently only valid with a `var` declaration and an assignment.

Adding an element to the array above
```
list << 5
```
results in `[3, 'Otto', true, 5]`

__Note__ _This feature is currently under construction and not stable._

## Comments ##
### One Line ##
One line comment starts with `#`. All characters after that symbol are ignored until the end of line symbol.
### Multiple Lines ###
A comment starts with `@{` and ends with `@}`. All text in between is ignored.

## Variable ##
### Name ###
The same rules apply as for literals.
### Declaration ###
Variables can be declared as integer, number, boolean or string.
```
int i
double d
string text
boolean b
```
And they can get an initial value:
```
int _abc = 1
int a = 1
int b = 2 
int a-b = a - b
boolean b = true
string text = 'This is a string.'
```
The `var` keyword is used to auto deduce the type of a variable.
```
var abc = 1     # deduce to int
var s = "Hello" # deduce to string.
var something   # type deduce will take place with the next assignment.
```

## Program ##
A program consists of several program blocks and each program block consists of statements.
Each statement can be
- a control expression
- an assignment
- a function declaration
- a variable declaration
- a return statements

and so on.

## Program Block ##
Program blocks are defined by the indention (similar to python).
```
block1
  block2
    block3
  block2
block1	
```

## Control Expressions ##

### if ###

```
if bool-expression 
  statements
else
  statements

```

### while ###

```
while id > 0
  id = id - 1
  do-something
else
  display("upps nix\n")
```

### return ###

```
return expression
```

## Assignment ##
```
var a = 5
```
## Function ##
Functions are defined as followed:

```
def aFunction( type param1, type param2 ) : return-type
  statements
  return expression
```
or if no return value is provided:
```
def aFunction( type param1, type param2 )
  statements
```

or with return type deduction:
```
def aFunction( type param1, type param2 )
  statements
  return expression
```

or if no parameter is needed:
```
def aFunction()
  statements
```

or if parameter type can/should be deduced at call time:
```
def aFunction(var param1, var param2)
  statements
  return expression
```

and any combination.

Calling a function
```
aFunction( 1, "text" )
aFunction()
int i = aFunction( 23 )
```


## Class ##
```
def classname
    type instance-variable
    def method
        method-body
```
_Hint:_ The keyword `var` can't be used to declare an instance variable.

Creating an object of a class
```
classname variable
```

The current class instance can be accesses via the keyword `self`. This is obligate if a class instance variable will be accessed.

Example:
```
def simple
    int myint = 5
    def get() : int
        return self.myint

simple p

int i = p.get()
```

## Miscellaneous ##
Any class method can be declared outside a class declaration if the first argument is the class instance object.
```
def set(simple s, int val)
    s.myint = val
```
On the other side each method of a class can be called in two ways
```
simple p
int i = p.get()
```
or
```
int i = get(p)
```

Importing other script files can be done via the `import` keyword.
```
import some-other-file
```

# Known issues #
## Array
Adding a member to a array in an inner scope doesn't work.
```
var myArray = []
if true
    myArray << 5
```
In the above example the variable `myArray` is unknown in the scope of the if block.
