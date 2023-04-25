#ifndef SIM8086_H
#define SIM8086_H


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

#define assert(expression) if (!(expression)) {*((int *)0) = 0;}
#define array_count(array) (sizeof(array) / sizeof((array)[0]))

#define BYTE_TO_BINARY(byte) \
(((byte) & (1 << 7)) ? '1' : '0'), \
(((byte) & (1 << 6)) ? '1' : '0'), \
(((byte) & (1 << 5)) ? '1' : '0'), \
(((byte) & (1 << 4)) ? '1' : '0'), \
(((byte) & (1 << 3)) ? '1' : '0'), \
(((byte) & (1 << 2)) ? '1' : '0'), \
(((byte) & (1 << 1)) ? '1' : '0'), \
(((byte) & (1 << 0)) ? '1' : '0')


enum Opcode {
    OPCODE_MOV_REGISTER_MEMORY_TO_OR_FROM_REGISTER  = 0b100010,
    OPCODE_MOV_IMMEDIATE_TO_REGISTER_OR_MEMORY = 0b1100011,
    OPCODE_MOV_IMMEDIATE_TO_REGISTER = 0b1011,
    OPCODE_MOV_MEMORY_TO_ACCUMULATOR = 0b1010000,
    OPCODE_MOV_ACCUMULATOR_TO_MEMORY = 0b1010001,
    
    OPCODE_ARITHMETIC_IMMEDIATE_TO_REGISTER_OR_MEMORY = 0b100000,
    
    OPCODE_ADD_REGISTER_OR_MEMORY = 0b000000,
    OPCODE_ADD_IMMEDIATE_TO_ACCUMULATOR = 0b0000010,
    
    OPCODE_SUB_REGISTER_OR_MEMORY = 0b001010,
    OPCODE_SUB_IMMEDIATE_FROM_ACCUMULATOR = 0b0010110,
    
    OPCODE_CMP_REGISTER_OR_MEMORY = 0b001110,
    OPCODE_CMP_IMMEDIATE_WITH_ACCUMULATOR = 0b0011110,
    
    // Conditional jumps
    OPCODE_JE     = 0b01110100,
    OPCODE_JL     = 0b01111100,
    OPCODE_JLE    = 0b01111110,
    OPCODE_JB     = 0b01110010,
    OPCODE_JBE    = 0b01110110,
    OPCODE_JP     = 0b01111010,
    OPCODE_JO     = 0b01110000,
    OPCODE_JS     = 0b01111000,
    OPCODE_JNE    = 0b01110101,
    OPCODE_JNL    = 0b01111101,
    OPCODE_JNLE   = 0b01111111,
    OPCODE_JNB    = 0b01110011,
    OPCODE_JNBE   = 0b01110111,
    OPCODE_JNP    = 0b01111011,
    OPCODE_JNO    = 0b01110001,
    OPCODE_JNS    = 0b01111001,
    OPCODE_LOOP   = 0b11100010,
    OPCODE_LOOPZ  = 0b11100001,
    OPCODE_LOOPNZ = 0b11100000,
    OPCODE_JCXZ   = 0b11100011,
};

enum Mod {
    MOD_MEMORY_MODE                     = 0b00,
    MOD_MEMORY_MODE_8_BIT_DISPLACEMENT  = 0b01,
    MOD_MEMORY_MODE_16_BIT_DISPLACEMENT = 0b10,
    MOD_REGISTER_MODE                   = 0b11,
};


enum OperationType {
    Op_none,
    
    Op_mov,
    Op_add,
    Op_sub,
    Op_cmp,
    Op_jmp,
};

enum Flags {
    WORD_BYTE_TEXT_REQUIRED = 0x1,
    REG_SOURCE_DEST = 0x2,
    DISPLACEMENT = 0x4,
    IMMEDIATE_ACCUMULATOR = 0x8,
    IMMEDIATE = 0x10,
    ACCUMULATOR_ADDRESS= 0x20,
    SIGN_EXTEND = 0x40,
};

OperationType arithmetic_operations[8] = {
    Op_add,  // 000
    Op_none, // 001
    Op_none, // 010
    Op_none, // 011
    Op_none, // 100
    Op_sub,  // 101
    Op_none, // 110
    Op_cmp,  // 111
};


struct FileContent {
    u8 *memory;
    u32 total_size;
    u32 size_remaining;
};

enum RegisterType {
    Register_none,
    
    Register_a,
    Register_b,
    Register_c,
    Register_d,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,
    
    Register_count,
};

// TODO: delete and use register matrix
char *registers_names[16] = {
    // Byte-size registers
    "al", // 0000
    "cl", // 0001
    "dl", // 0010
    "bl", // 0011
    "ah", // 0100
    "ch", // 0101
    "dh", // 0110
    "bh", // 0111
    
    // Word-size registers
    "ax", // 1000
    "cx", // 1001
    "dx", // 1010
    "bx", // 1011
    "sp", // 1100
    "bp", // 1101
    "si", // 1110
    "di", // 1111
};

struct Register {
    RegisterType type;
    u16 value;
};

struct Instruction {
    OperationType operation_type;
    RegisterType source_register;
    RegisterType dest_register;
    u8 binary;
    u8 d;
    u8 s;
    u8 w;
    u8 mod;
    u8 reg;
    u8 rm;
    u8 flags;
    u16 value;
};

RegisterType register_types[8][2] = {
    { {Register_a}, {Register_a} },
    { {Register_c}, {Register_c} },
    { {Register_d}, {Register_d} },
    { {Register_b}, {Register_b} },
    { {Register_a}, {Register_sp} },
    { {Register_c}, {Register_bp} },
    { {Register_d}, {Register_si} },
    { {Register_b}, {Register_di} },
};

struct State {
    Register registers[8];
};

inline bool str_equals(char *a, char *b)
{
    return strcmp(a, b) == 0;
}


inline RegisterType get_register_type(u8 w, u8 register_bytes)
{
    u8 count = array_count(register_types);
    assert(register_bytes < array_count(register_types));
    RegisterType type = register_types[register_bytes][w];
    return type;
}


#endif //SIM8086_H
