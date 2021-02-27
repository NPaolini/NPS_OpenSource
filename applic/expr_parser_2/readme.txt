Specifications of the calculation expressions
--------------------------------------

* operators and relative priority (in ascending order) *

  level 0 -> logical operators
      0_0 ->    ||               (or)
      0_1 ->    ^^               (xor)
      0_2 ->    &&               (and)
  level 1 -> Bit operators
      1_0 ->    |                (or)
      1_1 ->    ^                (xor)
      1_2 ->    &                (and)
  level 2 ->    ==, !=           (equal, not equal)
  level 3 ->    <, >, <=, >=     (minor, major, minor or equal, greater than or equal)
  level 4 ->    <<, >>           (left shift, right shift)

  level 5 ->    -, +             (addition, subtraction)
  level 6 ->    *, /, %          (mul, div, mod)
  level 7 ->    !, ~, -, (, ), , (logical not, bitwise not, reverse sign, parentheses, comma)
  level 8 ->    digit, $, alpha  (numbers, variables, functions/constants)


* defined functions:
    functions without parameters
    NOW     -> date-time in int64 formed by filetime
    DATE    -> only date in int64 formed by filetime
    TIME    -> only time in int64 formed by filetime
    YEAR    -> current year
    MONTH   -> current month
    DAY     -> current day
    HOUR    -> current hour
    MINUTE  -> minutes of the current time
    SEC     -> seconds

    RAND -> returns a random number between 0 and RAND_MAX (32767)


    functions with one parameter
    ABS     -> absolute value -> abs (expr)
    NEG     -> reverses the sign below (equals '-')
    NOTL    -> logical not, result -> zero / one (equal to '!')
    NOTB    -> bitwise not, inverts the bit values (equal to '~')
    SIN     -> sine
    COS     -> cosino
    EXP     -> exponential
    SQRT    -> square root
    LOG     -> logarithm
    TG      -> tangent
    CTG     -> cotangent
    ASIN    -> arc sine
    ACOS    -> arc cosine
    ATG     -> arctangent
    RAD     -> transforms from degrees to radians
    GRAD    -> transforms from radians to degrees

    SWAB2   -> exchanges two to two bytes,        e.g. ABCD -> BADC
    SWAB4   -> exchanges four to four bytes, 			e.g. ABCD -> DCBA
    SWAB8   -> exchanges height to height bytes,  e.g. ABCDEFGH -> HGFEDCBA

    TOBCD_LE   -> transforms the number -> BCD (LittleEndian)
    FROMBCD_LE -> transforms the BCD -> number (LittleEndian)
    TOBCD_BE   -> transforms the number -> BCD (BigEndian)
    FROMBCD_BE -> transforms the BCD -> number (BigEndian)

    functions with two parameters
    POW     -> exponentiation -> POW (a, b)
    SHL     -> left shift -> SHL (a, b) == a << b (equal to <<)
    SHR     -> right shift           							(equal to >>)
    DIFF    -> logical, returns true if the values are different (equal to !=)
    RANDMINMAX -> returns a random value between the two values passed

    functions with three parameters
    IF      -> the first argument is the test, if non-zero returns the second argument, otherwise the third -> IF (a, b, c)

    functions with more parameters
    MIN     -> the minimum of the values -> MIN (a, b, c, d, e, f, etc.)
    MAX     -> the maximum of the values
    AVR     -> average values
    SUM     -> sum of values
    EQU     -> logic equality, result -> zero / one (similar to '==', but for a range of values)

    ANDL    -> logical and (similar to '&&', but for a range of values)
    ANDB    -> binary and (similar to '&', but for a range of values)
    ORL     -> logical or (similar to '||', but for a range of values)
    ORB     -> binary or (similar to '|', but for a range of values)
    XORL    -> logical xor (similar to '^^', but for a range of values)
    XORB    -> binary xor (similar to '^', but for a range of values)

    functions on consecutive data blocks, fz ($var, ndata, step)
    $var must be a variable (or a vectoe element -> $var [1])
    other values can be constants or variables
    BLKSUM  -> similar to SUM
    BLKAVR  -> similar to AVR
    BLKDEVSTD -> standard deviation

    similar to the corresponding without BLK_, but instead for the values, it acts on the data block
    BLK_ORL  -> similar to ORL
    BLK_ORB  -> similar to ORB
    BLK_ANDL  -> similar to ANDL
    BLK_ANDB  -> similar to ANDB

    fz($var, ndata, step, upLimits, downLimits) e.g. if target = 100, up = 105, down = 96
    BLK_CMK -> machine capacity CMK
    BLK_CPK -> process capability CPK

    fz($var1, step1, $var2, step2, ndata)
    BLK_ORL_2  -> similar to BLK_ORL but the comparison is of two blocks of data
    BLK_ANDL_2 -> similar to BLK_ANDL but the comparison is of two blocks of data

    BLK_EQU_2 -> similar to EQU but the comparison is of two blocks of data
    BLK_DIFF_2 -> similar to DIFF but the comparison is of two blocks of data

* Constants defined
    PI      -> assigns the value pi
    E       -> assigns the value e -> exp (1)
    FSEC    -> conversion factor between filetime and seconds
    FMIN    -> conversion factor between filetime and minutes
    FHOUR   -> conversion factor between filetime and hours
    NULL    -> mainly used in the IF function allows to perform no assignment operation.
           In other cases does not perform the operation and the result is the other expression (e.g. a = f (b) + NULL -> a = f (b)).

* Variables Management:
  The variable must begin with the $ character and be followed by one or more of the following characters (a-z, A-Z, 0-9, _).
  It may be followed by a couple of values enclosed in square brackets and identify, respectively, the offset and the step (e.g. $foo [3,2]).
  This permits to have a vector variable. In calling the routine of calculation we can specify a number that identifies the current cycle.
	So, assuming you request the variable 2 with the cycle 4 the required value are to the position 11 (3 + 2 * 4) of the carrier.
	The offset is zero-based, so 11 indicates the twelfth value.
	Even the cycle is zero based;

	The variables are not case sensitive: -> $var xy equals $Var_xy.
	The maximum name length is 64 characters.
----------------------------------------------------------------------
The functions and constants can be indifferently written in uppercase or lowercase.

For to enter values in hexadecimal, the number must begin with a zero and end with the character H (e.g., 02a3bh).

N.B. If you enter only integers, the result will be an integer (unless special cases such as trigonometric
	functions or in the media, or ..), then the formula 5/2 will result 2 even if it requires the real as a return value.
	To get the real result at least one of the values must be a real, then -> 5 / 2.0 = 2.5 (simply enter the point to have a real e.g., 2.)
----------------------------------------------------------------------
######################################################################
----------------------------------------------------------------------
Script file format can be read by the DLL.
----------------------------------------------------------------------
The script is divided into blocks that begin with <block> and ending with </block>.
The beginning and end of the block must be on a separate line from the rest of the data.
The currently defined blocks are:
<VARS>
...
</VARS>

<FUNCT>
...
</FUNCT>

<CALC>
...
</CALC>
--------------
In <VARS> block are inserted the variables used by the script with information
on the type of data, 0 -> integer (64-bit), 1 -> real (double, 64 bits)
Each variables must be on a separate line.
e.g.
<VARS>
$var1=1
$var2=0
...
</VARS>
--------------
In block <FUNCT> the functions, the parameters passed to functions and the local variables must start with the character '@'.
The global variables, inside the functions, can be called in the usual way namely with the '$' character (also as part of the array, e.g., $v1 [4] or $v1 [$v2]).
The functions are no side effect, ie they can not modify global variables (to the left of the equal sign can not be a global variable, but only local or parameters).
The two predefined variables, _skip and _abort, must also begin with the character '@' and they have the same behavior of the <CALC> block.
Jump labels instead they use the same initial character that is '#'.
Jump labels are related to the functions and, therefore, you can use the same name in different functions without interfering with each other.
The functions are called the same way as the default ones, without any particular initial character (may start with the range [a-z] or with
the character '_', every other character is not allowed).
The number of parameters specified in the function is not mandatory that all are present at the time of the call, those not passed simply be worth zero.
If you pass more parameters than those that expect the function will not signal an error, simply the function, not knowing them, ignores them.
The value is returned from the function, with the classic 'return', the lines of code following are ignored, or by assigning a value to the function name.
If you arrive at the bottom of the function without having met any return, it is returned 'null' (which can also be returned with the return).
The definition of the function starts with the character '@' followed by the name and the parentheses that enclose the parameters, also they starting with the character '@'.
The block of code is enclosed in curly brackets '{' and '}'. The first can also be on the same line to the beginning function (where there are name and parameters),
but the closure must be on a line by itself, after the last line of code.
The functions without parameters must be called without parentheses, like the built-in functions, e.g. 'Now' or 'date'.
--------------
In block <CALC> you can put the calculation expressions.
Each expression must be on its own line.e.g.

<CALC>

$today = day
$v2 = month
$v3 = year
$v4 = hour
$v5 = minute
$v6 = sec

$v8 = $v7 * 2
$v10 = sin(rad($v9))
; vector -> zero offset, step one, eight elements
$v13[0, 1, 8] = $v11[0, 1] + $v12[0, 1]

; if $v18 is different from zero to the variable $v14 it is assigned the sum of the variables
; $v15 and $v16, otherwise it is not performed any action
$v14 = if($v18, $v16 + $v15, NULL)

</CALC>
To comment a line must precede the symbol ';'
----------------------------------------------------------------------
There are two special variables (not set on <VARS> area, but handled internally).

The first is $_SKIP. The value assigned to this variable allows to perform jumps related to the labels.
It should be used (preferred) in conjunction with if () for conditional jumps.
To perform an unconditional jump is sufficient to assign an label.
It should be used with caution because it can generate infinite loops or, however, does not return control to the caller until it finishes its execution.

The second variable is $_ABORT. If it acquires a non-zero value does not continue the execution of the remaining rows.

They can not be used with vector variables.
----------------------------------------------------------------------
Labels for jump to be used in conjunction with $_SKIP.
The label must be preceded by the '#' character and can be formed by any alphanumeric characters (plus the '_' character).
e.g. Label:
#step1
#again
etc.

e.g.
$_skip = if($var < 10, #step1, null)
----------------------------------------------------------------------
The single expression can be on multiple lines.
It is sufficient to terminate the lines forming the expression (minus the last one) with the \ character.
There should be no intermediate lines or comments.
e.g.

$_skip = \
  if($test < 10, null, \
    if($test < 20, #end1, \
      if($test < 30, #end2, \
                       #end3)))
----------------------------------------------------------------------
There is the possibility of withdrawing a value from the array by using a variable as an index.
The format is:
$value = $array[$index]

and the ability to enter a datum in an array using a variable as an index.
The format is:
$array[$index] = $value

N.B You can not use an expression within the square brackets, just numbers or variables

Of course it is possible to combine the two modes
e.g.
$array1[$index1] = $array2[$index2]
----------------------------------------------------------------------
