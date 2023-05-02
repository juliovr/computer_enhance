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
    OPCODE_MOV_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER = 0b10001110,
    OPCODE_MOV_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY = 0b10001100,
    
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
    HAS_DATA = 0x40,
    SEGMENT = 0x80,
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
    
    Register_es,
    Register_cs,
    Register_ss,
    Register_ds,
    
    Register_count,
};

struct RegisterDefinition {
    RegisterType type;
    char *name;
    u8 bytes;
};

RegisterDefinition registers_definitions[8][2] = {
    { {Register_a, "al", 0b0000}, {Register_a,  "ax", 0b1000} },
    { {Register_c, "cl", 0b0001}, {Register_c,  "cx", 0b1001} },
    { {Register_d, "dl", 0b0010}, {Register_d,  "dx", 0b1010} },
    { {Register_b, "bl", 0b0011}, {Register_b,  "bx", 0b1011} },
    { {Register_a, "ah", 0b0100}, {Register_sp, "sp", 0b1100} },
    { {Register_c, "ch", 0b0101}, {Register_bp, "bp", 0b1101} },
    { {Register_d, "dh", 0b0110}, {Register_si, "si", 0b1110} },
    { {Register_b, "bh", 0b0111}, {Register_di, "di", 0b1111} },
};

// These register are only 2 bits long, but the leading 2 aditional "11" bits are for identify these registers
// as if were wide in the simulation.
RegisterDefinition segment_registers[4] = {
    {Register_es, "es", 0b1100},
    {Register_cs, "cs", 0b1101},
    {Register_ss, "ss", 0b1110},
    {Register_ds, "ds", 0b1111},
};

struct DisplacementAddress {
    RegisterDefinition first_displacement;
    RegisterDefinition second_displacement;
    s16 offset;
};

struct Instruction {
    OperationType operation_type;
    RegisterDefinition source_register;
    RegisterDefinition dest_register;
    u8 binary;
    u8 d;
    u8 s;
    u8 w;
    u8 mod;
    u8 reg;
    u8 rm;
    u8 sr;
    u8 flags;
    u16 value;
    DisplacementAddress displacement_address;
};

struct Register {
    RegisterType type;
    u16 value;
};

struct State {
    Register registers[12];
    bool parity_flag;
    bool zero_flag;
    bool sign_flag;
};

inline bool str_equals(char *a, char *b)
{
    return strcmp(a, b) == 0;
}


inline RegisterDefinition get_register_definition(u8 w, u8 register_bytes)
{
    assert(register_bytes < array_count(registers_definitions));
    
    RegisterDefinition register_definition = registers_definitions[register_bytes][w];
    return register_definition;
}

inline RegisterDefinition get_segment_register_definition(u8 sr)
{
    assert(sr < array_count(segment_registers));
    
    RegisterDefinition segment_register = segment_registers[sr];
    return segment_register;
}

inline u8 count_bits(int n)
{
    u8 count = 0;
    while (n) {
        count += (n & 1);
        n >>= 1;
    }
    
    return count;
}

#endif //SIM8086_H
