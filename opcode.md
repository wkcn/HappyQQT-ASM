Name|Regs|Opcode|Proc|Description
-|-|-|-|-
AAA| |00110111|8086|ASCII Adjust After Addition
AAD|Imm8|11010101|Pentium|ASCII Adjust Register AX Before Division
 | |1101010100001010|8086|ASCII Adjust Register AX Before Division
AAM|Imm8|11010100|Pentium|ASCII Adjust AX Register After Multiplication
 | |1101010000001010|8086|ASCII Adjust AX Register After Multiplication
AAS| |00111111|8086|ASCII Adjust AL Register After Substraction
ADC|Reg,Reg|0001001woorrrmmm|8086|Add Integers with Carry
 |Mem,Reg|0001000woorrrmmm|8086|Add Integers with Carry
 |Reg,Mem|0001001woorrrmmm|8086|Add Integers with Carry
 |Acc,Imm|0001010w|8086|Add Integers with Carry
 |Reg,Imm8|1000001woo010mmm|8086|Add Integers with Carry
 |Mem,Imm8|1000001woo010mmm|8086|Add Integers with Carry
 |Reg,Imm|1000000woo010mmm|8086|Add Integers with Carry
 |Mem,Imm|1000000woo010mmm|8086|Add Integers with Carry
ADD|Reg,Reg|0000001woorrrmmm|8086|Add Integers
 |Mem,Reg|0000000woorrrmmm|8086|Add Integers
 |Reg,Mem|0000001woorrrmmm|8086|Add Integers
 |Acc,Imm|0000010w|8086|Add Integers
 |Reg,Imm8|1000001woo000mmm|8086|Add Integers
 |Mem,Imm8|1000001woo000mmm|8086|Add Integers
 |Reg,Imm|1000000woo000mmm|8086|Add Integers
 |Mem,Imm|1000000woo000mmm|8086|Add Integers
AND|Reg,Reg|0010001woorrrmmm|8086|Logical AND
 |Mem,Reg|0010000woorrrmmm|8086|Logical AND
 |Reg,Mem|0010001woorrrmmm|8086|Logical AND
 |Acc,Imm|0010010w|8086|Logical AND
 |Reg,Imm8|1000001woo100mmm|8086|Logical AND
 |Mem,Imm8|1000001woo100mmm|8086|Logical AND
 |Reg,Imm|1000000woo100mmm|8086|Logical AND
 |Mem,Imm|1000000woo100mmm|8086|Logical AND
ARPL|Reg16,Reg16|01100011oorrrmmm|80286|Adjust Requester Privilege Level of Selector
 |Mem16,Reg16|01100011oorrrmmm|80286|Adjust Requester Privilege Level of Selector
BOUND|Reg16,Mem32|01100010oorrrmmm|80186|Check Array Index Against Bounds
 |Reg32,Mem64|01100010oorrrmmm|80386|Check Array Index Against Bounds
BSF|RegWord,RegWord|0000111110111100oorrrmmm|80386|Bit Scan Forward
 |RegWord,MemWord|0000111110111100oorrrmmm|80386|Bit Scan Forward
BSR|RegWord,RegWord|0000111110111101oorrrmmm|80386|Bit Scan Reverse
 |RegWord,MemWord|0000111110111101oorrrmmm|80386|Bit Scan Reverse
BSWAP|RegWord|0000111111001rrr|80486|Byte swap
BT|RegWord,Imm8|0000111110111010oo100mmm|80386|Bit Test
 |MemWord,Imm8|0000111110111010oo100mmm|80386|Bit Test
 |RegWord,RegWord|0000111110100011oorrrmmm|80386|Bit Test
 |MemWord,RegWord|0000111110100011oorrrmmm|80386|Bit Test
BTC|RegWord,Imm8|0000111110111010oo111mmm|80386|Bit Test and Complement
 |MemWord,Imm8|0000111110111010oo111mmm|80386|Bit Test and Complement
 |RegWord,RegWord|0000111110111011oorrrmmm|80386|Bit Test and Complement
 |MemWord,RegWord|0000111110111011oorrrmmm|80386|Bit Test and Complement
BTR|RegWord,Imm8|0000111110111010oo110mmm|80386|Bit Test and Reset
 |MemWord,Imm8|0000111110111010oo110mmm|80386|Bit Test and Reset
 |RegWord,RegWord|0000111110110011oorrrmmm|80386|Bit Test and Reset
 |MemWord,RegWord|0000111110110011oorrrmmm|80386|Bit Test and Reset
BTS|RegWord,Imm8|0000111110111010oo101mmm|80386|Bit Test and Set
 |MemWord,Imm8|0000111110111010oo101mmm|80386|Bit Test and Set
 |RegWord,RegWord|0000111110101011oorrrmmm|80386|Bit Test and Set
 |MemWord,RegWord|0000111110101011oorrrmmm|80386|Bit Test and Set
CBW| |10011000|8086|Convert Byte to Word
CDQ| |10011001|80386|Convert Doubleword to Quad-Word
CLC| |11111000|8086|Clear Carry Flag (CF)
CLD| |11111100|8086|Clear Direction Flag (DF)
CLI| |11111010|8086|Clear Interrupt Flag (IF)
CLTS| |0000111100000110|80286|Clear Task-Switched Flag in Control Register Zero
CMC| |11110101|8086|Complementer Carry Flag (CF)
CMOVcc|Reg,Reg|000011110100ccccoorrrmmm|PentiumPro|Conditional Move
 |Reg,Mem|000011110100ccccoorrrmmm|PentiumPro|Conditional Move
CMP|Reg,Reg|0011101woorrrmmm|8086|Compare
 |Mem,Reg|0011100woorrrmmm|8086|Compare
 |Reg,Mem|0011101woorrrmmm|8086|Compare
 |Acc,Imm|0011110w|8086|Compare
 |Reg,Imm8|1000001woo111mmm|8086|Compare
 |Mem,Imm8|1000001woo111mmm|8086|Compare
 |Reg,Imm|1000000woo111mmm|8086|Compare
 |Mem,Imm|1000000woo111mmm|8086|Compare
CMPSB| |10100110|8086|Compare String - Byte
CMPSW| |10100111|8086|Compare String - Word
CMPSD| |10100111|80386|Compare String - Doubleword
CMPXCHG|Reg,Reg|000011111011000woorrrmmm|80486|Compare and Exchange
 |Mem,Reg|000011111011000woorrrmmm|80486|Compare and Exchange
CMPXCHG8B|Mem64|0000111111000111oo001mmm|Pentium|Compare and Exchange 8 Bytes
CPUID| |0000111110100010|Pentium|CPU Identification code to EAX
CWD| |10011001|8086|Convert Word to Doubleword
CWDE| |10011000|80386|Convert Word to Extended Doubleword
DAA| |00100111|8086|Decimal Adjust Register After Addition
DAS| |00101111|8086|Decimal Adjust AL Register After Substraction
DEC|RegWord|01001rrr|8086|Decrement by One
 |Reg|1111111woo001mmm|8086|Decrement by One
 |Mem|1111111woo001mmm|8086|Decrement by One
DIV|Reg|1111011woo110mmm|8086|Unsigned Integer Divide
 |Mem|1111011woo110mmm|8086|Unsigned Integer Divide
ENTER|Imm16,Imm8|11001000|80186|Make Stack Frame for Procedure Parameter
HLT| |11110100|8086|Halt
IDIV|Reg|1111011woo111mmm|8086|Signed Divide
 |Mem|1111011woo111mmm|8086|Signed Divide
IMUL|RegWord,RegWord,Imm8|01101011oorrrmmm|80186|Signed Integer Multiply
 |RegWord,MemWord,Imm8|01101011oorrrmmm|80186|Signed Integer Multiply
 |RegWord,RegWord,Imm|01101001oorrrmmm|80186|Signed Integer Multiply
 |RegWord,MemWord,Imm|01101001oorrrmmm|80186|Signed Integer Multiply
 |RegWord,Imm8|0110101111rrrqqq|80186|Signed Integer Multiply
 |RegWord,Imm|0110100111rrrqqq|80186|Signed Integer Multiply
 |RegWord,RegWord|0000111110101111oorrrmmm|80386|Signed Integer Multiply
 |RegWord,MemWord|0000111110101111oorrrmmm|80386|Signed Integer Multiply
 |Reg|1111011woo101mmm|8086|Signed Integer Multiply
 |Mem|1111011woo101mmm|8086|Signed Integer Multiply
IN|Acc,Imm8|1110010w|8086|Input from Port
 |Acc,DX|1110110w|8086|Input from Port
INC|RegWord|01000rrr|8086|Increment by 1
 |Reg|1111111woo000mmm|8086|Increment by 1
 |Mem|1111111woo000mmm|8086|Increment by 1
INSB| |01101100|80186|Input Byte
INSW| |01101101|80186|Input Word
INSD| |01101101|80386|Input DoubleWord
INT|3|11001100|8086|Call to Interrupt Procedure
 |Imm8|11001101|8086|Call to Interrupt Procedure
INTO| |11001110|8086|Interrupt on Overflow
INVD| |0000111100001000|80486|Invalidate data cache
INVLPG|Mem|0000111100000001oo111mmm|80486|Invalidate TBL entry
IRET| |11001111|8086|Return from Interrupt
IRETD| |11001111|80386|Return from Interrupt - 32-bit Mode
LAHF| |10011111|8086|Load Flags into AH Register
LAR|RegWord,RegWord|0000111100000010oorrrmmm|80286|Load Access Rights Byte
 |RegWord,MemWord|0000111100000010oorrrmmm|80286|Load Access Rights Byte
LDS|Reg16,Mem32|11000101oorrrmmm|8086|Load Pointer Using DS
 |Reg32,Mem64|11000101oorrrmmm|80386|Load Pointer Using DS
LES|Reg16,Mem32|11000100oorrrmmm|8086|Load Pointer Using ES
 |Reg32,Mem64|11000100oorrrmmm|80386|Load Pointer Using ES
LFS|Reg16,Mem32|0000111110110100oorrrmmm|80386|Load Pointer Using FS
 |Reg32,Mem64|0000111110110100oorrrmmm|80386|Load Pointer Using FS
LGS|Reg16,Mem32|0000111110110101oorrrmmm|80386|Load Pointer Using GS
 |Reg32,Mem64|0000111110110101oorrrmmm|80386|Load Pointer Using GS
LSS|Reg16,Mem32|0000111110110010oorrrmmm|80386|Load Pointer Using SS
 |Reg32,Mem64|0000111110110010oorrrmmm|80386|Load Pointer Using SS
LEA|RegWord,Mem|10001101oorrrmmm|8086|Load Effective Address
LEAVE| |11001001|80186|High Level Procedure Exit
LGDT|Mem64|0000111100000001oo010mmm|80286|Load Global Descriptor Table
LIDT|Mem64|0000111100000001oo011mmm|80286|Load Interrupt Descriptor Table
LLDT|Reg16|0000111100000000oo010mmm|80286|Load Local Descriptor Table
 |Mem16|0000111100000000oo010mmm|80286|Load Local Descriptor Table
LMSW|Reg16|0000111100000001oo110mmm|80286|Load Machine Status Word
 |Mem16|0000111100000001oo110mmm|80286|Load Machine Status Word
LODSB| |10101100|8086|Load Byte
LODSW| |10101101|8086|Load Word
LODSD| |10101101|80386|Load Doubleword
LSL|RegWord,RegWord|0000111100000011oorrrmmm|80286|Load Segment Limit
 |RegWord,MemWord|0000111100000011oorrrmmm|80286|Load Segment Limit
LTR|Reg16|0000111100000000oo011mmm|80286|Load Task Register
 |Mem16|0000111100000000oo011mmm|80286|Load Task Register
MOV|MemOfs,Acc|1010001w|8086|Move Data
 |Acc,MemOfs|1010000w|8086|Move Data
 |Reg,Imm|1011wrrr|8086|Move Data
 |Mem,Imm|1100011woo000mmm|8086|Move Data
 |Reg,Reg|1000101woorrrmmm|8086|Move Data
 |Reg,Mem|1000101woorrrmmm|8086|Move Data
 |Mem,Reg|1000100woorrrmmm|8086|Move Data
 |Reg16,Seg|10001100oosssmmm|8086|Move Data
 |Seg,Reg16|10001110oosssmmm|8086|Move Data
 |Mem16,Seg|10001100oosssmmm|8086|Move Data
 |Seg,Mem16|10001110oosssmmm|8086|Move Data
 |Reg32,CRn|000011110010000011sssrrr|80386|Move Data
 |CRn,Reg32|000011110010001011sssrrr|80386|Move Data
 |Reg32,DRn|000011110010000111sssrrr|80386|Move Data
 |DRn,Reg32|000011110010001111sssrrr|80386|Move Data
 |Reg32,TRn|000011110010010011sssrrr|80386|Move Data
 |TRn,Reg32|000011110010011011sssrrr|80386|Move Data
MOVSB| |10100100|8086|Move Byte
MOVSW| |10100101|8086|Move Word
MOVSD| |10100101|80386|Move Doubleword
MOVSX|RegWord,Reg8|0000111110111110oorrrmmm|80386|Move with Sign Extension
 |RegWord,Mem8|0000111110111110oorrrmmm|80386|Move with Sign Extension
 |RegWord,Reg16|0000111110111111oorrrmmm|80386|Move with Sign Extension
 |RegWord,Mem16|0000111110111111oorrrmmm|80386|Move with Sign Extension
MOVZX|RegWord,Reg8|0000111110110110oorrrmmm|80386|Move with Zero Extension
 |RegWord,Mem8|0000111110110110oorrrmmm|80386|Move with Zero Extension
 |RegWord,Reg16|0000111110110111oorrrmmm|80386|Move with Zero Extension
 |RegWord,Mem16|0000111110110111oorrrmmm|80386|Move with Zero Extension
MUL|Reg|1111011woo100mmm|8086|Unsigned Integer Multiply of AL, AX or EAX
 |Mem|1111011woo100mmm|8086|Unsigned Integer Multiply of AL, AX or EAX
NEG|Reg|1111011woo011mmm|8086|Negate(Two's Complement)
 |Mem|1111011woo011mmm|8086|Negate(Two's Complement)
NOP| |10010000|8086|No Operation
NOT|Reg|1111011woo010mmm|8086|Negate(One's Complement)
 |Mem|1111011woo010mmm|8086|Negate(One's Complement)
OR|Reg,Reg|0000101woorrrmmm|8086|Logical Inclusive OR
 |Mem,Reg|0000100woorrrmmm|8086|Logical Inclusive OR
 |Reg,Mem|0000101woorrrmmm|8086|Logical Inclusive OR
 |Acc,Imm|0000110w|8086|Logical Inclusive OR
 |Reg,Imm8|1000001woo001mmm|8086|Logical Inclusive OR
 |Mem,Imm8|1000001woo001mmm|8086|Logical Inclusive OR
 |Reg,Imm|1000000woo001mmm|8086|Logical Inclusive OR
 |Mem,Imm|1000000woo001mmm|8086|Logical Inclusive OR
OUT|Imm8,Acc|1110011w|8086|Output To Port
 |DX,Acc|1110111w|8086|Output To Port
OUTSB| |01101110|80186|Output Byte
OUTSW| |01101111|80186|Output Word
OUTSD| |01101111|80386|Output Doubleword
POP|RegWord|01011rrr|8086|Pop a Word from the Stack
 |MemWord|10001111oo000mmm|8086|Pop a Word from the Stack
 |SegOld|00sss111|8086|Pop a Word from the Stack
 |Seg|0000111110sss001|80386|Pop a Word from the Stack
POPA| |01100001|80186|POP All Registers
POPAD| |01100001|80386|POP All Registers - 32-bit Mode
POPF| |10011101|8086|POP Stack into FLAGS
POPFD| |10011101|80386|POP Stack into EFLAGS
PUSH|RegWord|01010rrr|8086|Push Operand onto Stack
 |MemWord|11111111oo110mmm|8086|Push Operand onto Stack
 |SegOld|00sss110|8086|Push Operand onto Stack
 |Seg|0000111110sss000|80386|Push Operand onto Stack
 |Imm8|01101010|80186|Push Operand onto Stack
 |Imm|01101000|80186|Push Operand onto Stack
PUSHW|Imm16|01101000|80286|PUSH Word
PUSHD|Imm32|01101000|80386|PUSH Double Word
PUSHA| |01100000|80186|PUSH All Registers
PUSHAD| |01100000|80386|PUSH All Registers - 32-bit Mode
PUSHF| |10011100|8086|PUSH FLAGS
PUSHFD| |10011100|80386|PUSH EFLAGS
RCL|Reg,1|1101000woo010mmm|8086|Rotate Left through Carry - Uses CF for Extension
 |Mem,1|1101000woo010mmm|8086|Rotate Left through Carry - Uses CF for Extension
 |Reg,CL|1101001woo010mmm|8086|Rotate Left through Carry - Uses CF for Extension
 |Mem,CL|1101001woo010mmm|8086|Rotate Left through Carry - Uses CF for Extension
 |Reg,Imm8|1100000woo010mmm|80186|Rotate Left through Carry - Uses CF for Extension
 |Mem,Imm8|1100000woo010mmm|80186|Rotate Left through Carry - Uses CF for Extension
RCR|Reg,1|1101000woo011mmm|8086|Rotate Right through Carry - Uses CF for Extension
 |Mem,1|1101000woo011mmm|8086|Rotate Right through Carry - Uses CF for Extension
 |Reg,CL|1101001woo011mmm|8086|Rotate Right through Carry - Uses CF for Extension
 |Mem,CL|1101001woo011mmm|8086|Rotate Right through Carry - Uses CF for Extension
 |Reg,Imm8|1100000woo011mmm|80186|Rotate Right through Carry - Uses CF for Extension
 |Mem,Imm8|1100000woo011mmm|80186|Rotate Right through Carry - Uses CF for Extension
RDMSR| |0000111100110010|Pentium|Read from Model Specific Register
RET|NEAR|11000011|8086|Return from subprocedure
RET|imm NEAR|11000010|8086|Return from subprocedure
RET|FAR|11001011|8086|Return from subprocedure
RET|imm FAR|11001010|8086|Return from subprocedure
RDPMC| |0000111100110011|PentiumPro|Read Performance Monitor Counter
ROL|Reg,1|1101000woo000mmm|8086|Rotate Left through Carry - Wrap bits around
 |Mem,1|1101000woo000mmm|8086|Rotate Left through Carry - Wrap bits around
 |Reg,CL|1101001woo000mmm|8086|Rotate Left through Carry - Wrap bits around
 |Mem,CL|1101001woo000mmm|8086|Rotate Left through Carry - Wrap bits around
 |Reg,Imm8|1100000woo000mmm|80186|Rotate Left through Carry - Wrap bits around
 |Mem,Imm8|1100000woo000mmm|80186|Rotate Left through Carry - Wrap bits around
ROR|Reg,1|1101000woo001mmm|8086|Rotate Right through Carry - Wrap bits around
 |Mem,1|1101000woo001mmm|8086|Rotate Right through Carry - Wrap bits around
 |Reg,CL|1101001woo001mmm|8086|Rotate Right through Carry - Wrap bits around
 |Mem,CL|1101001woo001mmm|8086|Rotate Right through Carry - Wrap bits around
 |Reg,Imm8|1100000woo001mmm|80186|Rotate Right through Carry - Wrap bits around
 |Mem,Imm8|1100000woo001mmm|80186|Rotate Right through Carry - Wrap bits around
RSM| |0000111110101010|Pentium|Return from System Management mode
SALC| |11010110|Pentium Pro|Set AL on Carry
SAHF| |10011110|8086|Load Flags into AH Register
SAL|Reg,1|1101000woo100mmm|8086|Shift Arithmetic Left
 |Mem,1|1101000woo100mmm|8086|Shift Arithmetic Left
 |Reg,CL|1101001woo100mmm|8086|Shift Arithmetic Left
 |Mem,CL|1101001woo100mmm|8086|Shift Arithmetic Left
 |Reg,Imm8|1100000woo100mmm|80186|Shift Arithmetic Left
 |Mem,Imm8|1100000woo100mmm|80186|Shift Arithmetic Left
SAR|Reg,1|1101000woo111mmm|8086|Shift Arithmetic Right
 |Mem,1|1101000woo111mmm|8086|Shift Arithmetic Right
 |Reg,CL|1101001woo111mmm|8086|Shift Arithmetic Right
 |Mem,CL|1101001woo111mmm|8086|Shift Arithmetic Right
 |Reg,Imm8|1100000woo111mmm|80186|Shift Arithmetic Right
 |Mem,Imm8|1100000woo111mmm|80186|Shift Arithmetic Right
SETcc|Reg8|000011111001ccccoo000mmm|80386|Set Byte on Condition Code
 |Mem8|000011111001ccccoo000mmm|80386|Set Byte on Condition Code
SHL|Reg,1|1101000woo100mmm|8086|Shift Logic Left
 |Mem,1|1101000woo100mmm|8086|Shift Logic Left
 |Reg,CL|1101001woo100mmm|8086|Shift Logic Left
 |Mem,CL|1101001woo100mmm|8086|Shift Logic Left
 |Reg,Imm8|1100000woo100mmm|80186|Shift Logic Left
 |Mem,Imm8|1100000woo100mmm|80186|Shift Logic Left
SHR|Reg,1|1101000woo101mmm|8086|Shift Logic Right
 |Mem,1|1101000woo101mmm|8086|Shift Logic Right
 |Reg,CL|1101001woo101mmm|8086|Shift Logic Right
 |Mem,CL|1101001woo101mmm|8086|Shift Logic Right
 |Reg,Imm8|1100000woo101mmm|80186|Shift Logic Right
 |Mem,Imm8|1100000woo101mmm|80186|Shift Logic Right
SBB|Reg,Reg|0001101woorrrmmm|8086|Substract Integers with Borrow
 |Mem,Reg|0001100woorrrmmm|8086|Substract Integers with Borrow
 |Reg,Mem|0001101woorrrmmm|8086|Substract Integers with Borrow
 |Acc,Imm|0001110w|8086|Substract Integers with Borrow
 |Reg,Imm8|1000001woo011mmm|8086|Substract Integers with Borrow
 |Mem,Imm8|1000001woo011mmm|8086|Substract Integers with Borrow
 |Reg,Imm|1000000woo011mmm|8086|Substract Integers with Borrow
 |Mem,Imm|1000000woo011mmm|8086|Substract Integers with Borrow
SCASB| |10101110|8086|Compare Byte
SCASW| |10101111|8086|Compare Word
SCASD| |10101111|80386|Compare Doubleword
SGDT|Mem64|0000111100000001oo000mmm|80286|Store Global Descriptor Table
SHLD|RegWord,RegWord,Imm8|0000111110100100oorrrmmm|80386|Double Precision Shift Left
 |MemWord,RegWord,Imm8|0000111110100100oorrrmmm|80386|Double Precision Shift Left
 |RegWord,RegWord,CL|0000111110100101oorrrmmm|80386|Double Precision Shift Left
 |MemWord,RegWord,CL|0000111110100101oorrrmmm|80386|Double Precision Shift Left
SHRD|RegWord,RegWord,Imm8|0000111110101100oorrrmmm|80386|Double Precision Shift Right
 |MemWord,RegWord,Imm8|0000111110101100oorrrmmm|80386|Double Precision Shift Right
 |RegWord,RegWord,CL|0000111110101101oorrrmmm|80386|Double Precision Shift Right
 |MemWord,RegWord,CL|0000111110101101oorrrmmm|80386|Double Precision Shift Right
SIDT|Mem64|0000111100000001oo001mmm|80286|Store Interrupt Descriptor Table
SLDT|Reg16|0000111100000000oo000mmm|80286|Store Local Descriptor Table Register (LDTR)
 |Mem16|0000111100000000oo000mmm|80286|Store Local Descriptor Table Register (LDTR)
SMSW|Reg16|0000111100000001oo100mmm|80286|Store Machine Status Word
 |Mem16|0000111100000001oo100mmm|80286|Store Machine Status Word
STC| |11111001|8086|Set Carry Flag(CF)
STD| |11111101|8086|Set Direction Flag(DF)
STI| |11111011|8086|Set Interrupt Flag(IF)
STOSB| |10101010|8086|Store String Data Byte
STOSW| |10101011|8086|Store String Data Word
STOSD| |10101011|80386|Store String Data DoubleWord
STR|Reg16|0000111100000000oo001mmm|80286|Store Task Register
 |Mem16|0000111100000000oo001mmm|80286|Store Task Register
SUB|Reg,Reg|0010101woorrrmmm|8086|Subtract
 |Mem,Reg|0010100woorrrmmm|8086|Subtract
 |Reg,Mem|0010101woorrrmmm|8086|Subtract
 |Acc,Imm|0010110w|8086|Subtract
 |Reg,Imm8|1000001woo101mmm|8086|Subtract
 |Mem,Imm8|1000001woo101mmm|8086|Subtract
 |Reg,Imm|1000000woo101mmm|8086|Subtract
 |Mem,Imm|1000000woo101mmm|8086|Subtract
TEST|Reg,Reg|1000010woorrrmmm|8086|Test Operands
 |Mem,Reg|1000010woorrrmmm|8086|Test Operands
 |Reg,Mem|1000010woorrrmmm|8086|Test Operands
 |Acc,Imm|1010100w|8086|Test Operands
 |Reg,Imm|1111011woo000mmm|8086|Test Operands
 |Mem,Imm|1111011woo000mmm|8086|Test Operands
VERR|Reg16|0000111100000000oo100mmm|80286|Verify Read
 |Mem16|0000111100000000oo100mmm|80286|Verify Read
VERW|Reg16|0000111100000000oo101mmm|80286|Verify Write
 |Mem16|0000111100000000oo101mmm|80286|Verify Write
WAIT| |10011011|8086|Wait for FPU
WBINVD| |0000111100001001|80486|Write Back and Invalidate Data Cache
WRMSR| |0000111100110000|Pentium|Write to Model Specific Register
XADD|Reg,Reg|000011111100000woorrrmmm|80486|Exchange and Add
 |Mem,Reg|000011111100000woorrrmmm|80486|Exchange and Add
XCHG|AccWord,RegWord|10010rrr|8086|Exchange
 |RegWord,AccWord|10010rrr|8086|Exchange
 |Reg,Reg|1000011woorrrmmm|8086|Exchange
 |Mem,Reg|1000011woorrrmmm|8086|Exchange
 |Reg,Mem|1000011woorrrmmm|8086|Exchange
XLAT| |11010111|8086|Translate
XOR|Reg,Reg|0011001woorrrmmm|8086|Exclusive-OR
 |Mem,Reg|0011000woorrrmmm|8086|Exclusive-OR
 |Reg,Mem|0011001woorrrmmm|8086|Exclusive-OR
 |Acc,Imm|0011010w|8086|Exclusive-OR
 |Reg,Imm8|1000001woo110mmm|8086|Exclusive-OR
 |Mem,Imm8|1000001woo110mmm|8086|Exclusive-OR
 |Reg,Imm|1000000woo110mmm|8086|Exclusive-OR
 |Mem,Imm|1000000woo110mmm|8086|Exclusive-OR
CALL|MemFar|11111111oo011mmm|8086|Call a Procedure
 |Near|11101000|8086|Call a Procedure
 |Far|10011010|8086|Call a Procedure
 |RegWord|11111111oo010mmm|8086|Call a Procedure
 |MemNear|11111111oo010mmm|8086|Call a Procedure
Jcc|Short|0111cccc|8086|Jump on Some Condition Code
 |Near|000011111000cccc|80386|Jump on Some Condition Code
JCXZ|Short|11100011|8086| 
JCXE|Short|11100011|8086| 
JECXZ|Short|11100011|8086| 
JECXE|Short|11100011|8086| 
JMP|MemFar|11111111oo101mmm|8086| 
 |Short|11101011|8086| 
 |Near|11101001|8086| 
 |Far|11101010|8086| 
 |RegWord|11111111oo100mmm|8086| 
 |MemNear|11111111oo100mmm|8086| 
LOOP|Short|11100010|8086|Loop Control While ECX Counter Not Zero
LOOPZ|Short|11100001|8086|Loop while Zero
LOOPE|Short|11100001|8086|Loop while Equal
LOOPNZ|Short|11100000|8086|Loop while Not Zero
LOOPNE|Short|11100000|8086|Loop while Not Equal
LOCK| |11110000|8086|Assert Lock# Signal Prefix
LOCK:| |11110000|8086|Assert Lock# Signal Prefix
REP| |11110011|8086|Repeat Following String Operation
REPE| |11110011|8086|Repeat while Equal
REPZ| |11110011|8086|Repeat while Zero
REPNE| |11110010|8086|Repeat while Not Equal
REPNZ| |11110010|8086|Repeat while Not Zero
CS:| |00101110|8086|CS segment override prefix
DS:| |00111110|8086|DS segment override prefix
ES:| |00100110|8086|ES segment override prefix
FS:| |01100100|80386|FS segment override prefix
GS:| |01100101|80386|GS segment override prefix
SS:| |00110110|8086|SS segment override prefix
