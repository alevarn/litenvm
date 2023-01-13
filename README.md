# LitenVM: A Small Stack-based Virtual Machine in C

This is a minimalistic stack-based virtual machine written in C called *LitenVM* (*liten* means small in Swedish). *LitenVM* is similar to the Java Virtual Machine (JVM), but it has a much smaller instruction set, with only 22 different instructions. Additionally, *LitenVM* supports various features including strings, integer arithmetic, conditional and unconditional jumps, method calls, and subtype polymorphism. It also includes built-in support for native classes and methods, which can be utilized for string concatenation and console output. However, *LitenVM* lacks certain features that are essential for a commercial-grade virtual machine in today's world, such as a garbage collector, just-in-time compiler, and bytecode verifier to prevent the execution of dangerous code.

## Binary Format

Down below is a context-free grammar that captures the main rules of *LitenVM*'s binary format. However, some restrictions cannot be expressed directly in context-free grammar. These limitations are added as side notes in the end.

```
Program ::= ConstantPool InstructionStream

ConstantPool ::= Length ConstantEntry*
ConstantEntry ::= 0x00 Class 
                  | 0x01 Field 
                  | 0x02 Method 
                  | 0x03 String
Class ::= NameLen Name ParentIdx 
            Fields Methods
Field ::= NameLen Name ClassIdx Index
Method ::= NameLen Name ClassIdx Address 
            Args Locals
String ::= TextLen Text

InstructionStream ::= Length Instruction*
```

A program consists of a constant pool section followed by an instruction stream section.
Both the constant pool section and the instruction stream section start with a 32-bit `Length` value that gives the number of constant pool entries or instruction stream entries. Note that all 32-bit values in the binary format must use big-endian. This is done to achieve portability between machines because otherwise different machines can interpret the same byte sequence as two completely different numbers. Each constant pool entry starts with an 8-bit value to identify the entry type, for example, the `Class` entry has the entry code `0`.

The `Class`, `Field` and `Method` entry starts with a 32-bit `NameLen` value that gives the length of the null-terminated string `Name`. Similarly, the `String` entry starts with a 32-bit `TextLen` value that gives the length of the null-terminated string `Text`. The remaining values in the `Class`, `Field`, and `Method` rules are all 32-bit integer values and are explained below. 

The instruction stream is simply a 32-bit `Length` value that gives the number of instructions followed by the actual instructions where each instruction is a 40-bit value. The instruction set is explained below.

## Constant Pool

Similarly to the JVM, *LitenVM* uses a constant pool to store static data. 
The constant pool both in JVM and *LitenVM* starts at index one. 
The zero index is reserved for the null reference. 
There are four different constant pool entry types and down below is a description of each and their fields:

1. `Class`: Stores static data about a class.
     - `Name`: The name of the class.
     - `Parent`: A constant pool index that refers to the parent class (or zero if the class has no parent).
     - `Fields`: The number of fields defined in the class.
     - `Methods`: The number of methods defined in the class.
     - `VTable`: A pointer to the virtual method table.
2. `Field`: Stores static data about a field that belongs to a class. 
     - `Name`: The name of the field.
     - `Class`: A constant pool index that refers to the class that the field is defined in.
     - `Index`: The position of the field in the class. The first field starts at index 0, the second field starts at index 1, and so on. 
3. `Method`: Stores static data about a method that belongs to a class. 
     - `Name`: The name of the method.
     - `Class`: A constant pool index that refers to the class that the method is defined in.
     - `Address`: The instruction stream position of the method's first instruction.
     - `Args`: The number of arguments that the method takes.
     - `Locals`: The number of local variables defined in the method.
4. String: A string literal that is used in the program.
     - `Text`: A null-terminated string. 
     
There are some predefined entries in the constant pool for native classes and methods 
that are implemented directly in *LitenVM*. These predefined constant pool entries have been given hardcoded indices between 
`0xfffffff8 - 0xffffffff`, so they are placed at the bottom of the constant pool. 
We have the following native classes and methods:
- `String class` (`0xfffffff8`): A class to represent a string.
- `Console class` (`0xfffffff9`): A class to represent the console.
- `Console.println` (`0xfffffffa`): The `println` method takes a `Console` object and a `String` object and prints the string to the console. 
- `StringBuilder class` (`0xfffffffb`): A class for working with strings. Similar to the `StringBuilder` class in Java.
- `StringBuilder.appendString` (`0xfffffffc`):  The `appendString` method takes a `StringBuilder` object and a `String` object and appends the string to the current string stored in the `StringBuilder` object. The method also returns the `this` reference just as in Java. 
- `StringBuilder.appendInt` (`0xfffffffd`): Same as above but takes an integer instead of a string.
- `StringBuilder.appendBool` (`0xfffffffe`): Same as above but takes an integer value of 0 (false) or 1 (true). 
- `StringBuilder.toString` (`0xffffffff`): The `toString` method takes a `StringBuilder` object and return the `String` object that is internally stored in the `StringBuilder`.

## Instruction set

For simplicity, all *LitenVM* instructions have a fixed length
of 40-bits. The first 8-bits stores the operation code (opcode) that specifies the operation to be performed.
The remaining 32-bits stores an immediate value that could be an integer value, a jump address, or an index into the constant pool.
Not all instructions use the immediate value for example `ADD`, `SUB`
and `RETURN`. Instructions that do not use the immediate value will still occupy 40-bits.
The instruction set of *LitenVM* consists of 22 different instructions and down below is a list describing all of them. 
Note that next to the name of each instruction is the opcode written as a hexadecimal number:

- `PUSH` (`0x00`): Will push the immediate value onto the evaluation stack.
- `PUSH_STRING` (`0x01`): Will push a string object onto the evaluation stack. The immediate value must be an index that refers to an `String` entry inside the constant pool.
- `PUSH_VAR` (`0x02`): Will push the value of an argument or local variable onto the evaluation stack. The immediate value must be an index that refers to an argument or local variable that is stored inside the  current call frame. Note that `PUSH_VAR 0` will always push the current object reference (known as `this` in many programming languages) because *LitenVM* does not support static methods. 
- `PUSH_FIELD` (`0x03`):  Will push the value of an object's field onto the evaluation stack. The immediate value must be an index that refers to a `Field` entry inside the constant pool. This instruction will consume the topmost element of the evaluation stack, which should be an object reference.
    
- `POP` (`0x04`): Removes the topmost element from the evaluation stack.
- `POP_VAR` (`0x05`): Will pop the topmost value from the evaluation stack and store it inside an argument or local variable. The interpretation of the immediate value is the same as the one for `PUSH_VAR`.
- `POP_FIELD` (`0x06`): Will pop the two topmost values from the evaluation stack, first the value to store and then the object reference, lastly this instruction will update the object's field with the new value. The interpretation of the immediate value is the same as the one for `PUSH_FIELD`.

- `ADD` (`0x07`): Will pop the two topmost values from the evaluation stack, add them together and then push the sum back onto the evaluation stack. 
- `SUB` (`0x08`): Same as above but performs subtraction instead.
- `MUL` (`0x09`): Same as above but performs multiplication instead.
- `DIV` (`0x0A`): Same as above but performs division instead.

- `CALL` (`0x0B`): Will pop the $n$ topmost values from the evaluation stack where $n$ is the number of arguments that the method takes. The immediate value must be an index that refers to a `Method` entry inside the constant pool. This instruction will create a new call frame and update the program counter to be equal to the address of the first instruction of the method to call. 

- `RETURN` (`0x0C`): Will reset the program counter to the old address stored inside the current call frame and then pop the call frame from the call stack. 


- `NEW` (`0x0D`): Will push a new object reference onto the evaluation stack.  The immediate value must be an index that refers to a `Class` entry inside the constant pool.

- `DUP` (`0x0E`): Will push the current topmost element of the evaluation stack onto the evaluation stack again.

- `JUMP` (`0x80`): Performs an unconditional jump by setting the program counter to be equal to the immediate value. Note that we are using absolute and not relative addresses. 
- `JUMP_XX` (`0x81-0x86`): Performs a jump if the condition is satisfied. The condition is checked by popping the two topmost elements and then comparing them with the `XX` operator. There are six different operators `XX = {eq,ne,lt,le,gt,ge}`, each operator corresponds to a unique instruction in the instruction set.
