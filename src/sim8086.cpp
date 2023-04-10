#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;


#define assert(expression) if (!(expression)) {*((int *)0) = 0;}

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
    OPCODE_MOV_IMMEDIATE_REGISTER_OR_MEMORY = 0b1100011,
    OPCODE_MOV_IMMEDIATE_TO_REGISTER = 0b1011,
    OPCODE_MOV_MEMORY_TO_ACCUMULATOR = 0b1010000,
    OPCODE_MOV_ACCUMULATOR_TO_MEMORY = 0b1010001,
};

enum Mod {
    MOD_MEMORY_MODE                     = 0b00,
    MOD_MEMORY_MODE_8_BIT_DISPLACEMENT  = 0b01,
    MOD_MEMORY_MODE_16_BIT_DISPLACEMENT = 0b10,
    MOD_REGISTER_MODE                   = 0b11,
};


char *register_table[16] = {
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

u8 print_memory_address_and_displacement(u8 rm, u8 mod, u8 *memory)
{
    switch (rm)
    {
        case 0b000: {
            printf("[bx + si");
        } break;
        
        case 0b001: {
            printf("[bx + di");
        } break;
        
        case 0b010: {
            printf("[bp + si");
        } break;
        
        case 0b011: {
            printf("[bp + di");
        } break;
        
        case 0b100: {
            printf("[si");
        } break;
        
        case 0b101: {
            printf("[di");
        } break;
        
        case 0b110: {
            if (mod == MOD_MEMORY_MODE) {
                printf("[");
            } else {
                printf("[bp");
            }
        } break;
        
        case 0b111: {
            printf("[bx");
        } break;
    }
    
    u8 size_used = 0;
    switch (mod)
    {
        case MOD_MEMORY_MODE: {
            if (rm == 0b110) {
                u16 displacement = *(u16 *)memory;
                memory += 2;
                size_used += 2;
                printf("%d]", displacement);
            } else {
                printf("]");
            }
        } break;
        
        case MOD_MEMORY_MODE_8_BIT_DISPLACEMENT: {
            s8 number = *(s8 *)memory;
            memory++;
            
            char sign = (number < 0) ? '-' : '+';
            if (number) {
                printf(" %c %d]", sign, -number);
            } else {
                printf("]");
            }
            
            size_used += 1;
        } break;
        
        case MOD_MEMORY_MODE_16_BIT_DISPLACEMENT: {
            s16 number = *(s16 *)memory;
            memory += 2;
            
            char sign = (number < 0) ? '-' : '+';
            if (number) {
                printf(" %c %d]", sign, (number < 0) ? -number : number);
            } else {
                printf("]");
            }
            
            size_used += 2;
        } break;
    }
    
    return size_used;
}

void decode_asm_8086(u32 size, u8 *memory)
{
    while (size)
    {
        u8 first_byte = *memory++;
        
        if (((first_byte >> 2) & 0b111111) == OPCODE_MOV_REGISTER_MEMORY_TO_OR_FROM_REGISTER)
        {
            printf("mov ");
            
            u8 second_byte = *(memory++);
            
            u8 w = first_byte & 1;
            u8 d = first_byte & 2;
            u8 mod = ((second_byte >> 6) & 0b11);
            if (mod == MOD_REGISTER_MODE)
            {
                u8 rm = second_byte & 0b111;
                u8 reg = ((second_byte >> 3) & 0b111);
                
                char *rm_register_name  = register_table[(w << 3) | rm];
                char *reg_register_name = register_table[(w << 3) | reg];
                
                if (d) {
                    printf("%s, %s", reg_register_name, rm_register_name);
                } else {
                    printf("%s, %s", rm_register_name, reg_register_name);
                }
                
                size -= 2;
            }
            else
            {
                u8 rm = second_byte & 0b111;
                u8 reg = ((second_byte >> 3) & 0b111);
                
                char *reg_register_name = register_table[(w << 3) | reg];
                
                if (d) {
                    printf("%s, ", reg_register_name);
                }
                
                u8 size_used = print_memory_address_and_displacement(rm, mod, memory);
                size -= size_used;
                memory += size_used;
                
                if (!d) {
                    printf(", %s", reg_register_name);
                }
                
                
                size -= 2;
            }
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_IMMEDIATE_REGISTER_OR_MEMORY)
        {
            printf("mov ");
            
            u8 second_byte = *memory++;
            
            u8 w = first_byte & 1;
            u8 mod = (second_byte >> 6) & 0b11;
            u8 rm = second_byte & 0b111;
            
            u8 size_used = print_memory_address_and_displacement(rm, mod, memory);
            size -= size_used;
            memory += size_used;
            
            if (w) {
                u16 number = *(u16 *)memory;
                memory += 2;
                
                printf(", word %d", number);
                
                size -= 2;
            } else {
                u8 number = *(u8 *)memory;
                memory += 1;
                
                printf(", byte %d", number);
                
                size -= 1;
            }
            
            size -= 2;
        }
        else if (((first_byte >> 4) & 0b1111) == OPCODE_MOV_IMMEDIATE_TO_REGISTER)
        {
            printf("mov ");
            
            u8 w = (first_byte >> 3) & 1;
            u8 w_plus_reg = first_byte & 0b1111;
            char *register_name = register_table[w_plus_reg];
            
            if (w == 1) {
                u16 number = *(u16 *)memory;
                u8 second_byte = *(memory++);
                u8 third_byte = *(memory++);
                
                u16 number2 = (third_byte << 8) | second_byte;
                printf("%s, %d", register_name, number);
                
                size -= 3;
            } else {
                u8 number = *(memory++);
                printf("%s, %d", register_name, number);
                
                size -= 2;
            }
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_MEMORY_TO_ACCUMULATOR)
        {
            printf("mov ");
            
            printf("ax, ");
            
            u8 w = first_byte & 1;
            
            if (w) {
                u16 number = *(u16 *)memory;
                memory += 2;
                size -= 2;
                
                printf("[%d]", number);
            } else {
                u8 number = *(u8 *)memory;
                memory += 1;
                size -= 1;
                
                printf("[%d]", number);
            }
            
            size -= 1;
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_ACCUMULATOR_TO_MEMORY)
        {
            printf("mov ");
            
            u8 w = first_byte & 1;
            
            if (w) {
                u16 number = *(u16 *)memory;
                memory += 2;
                size -= 2;
                
                printf("[%d]", number);
            } else {
                u8 number = *(u8 *)memory;
                memory += 1;
                size -= 1;
                
                printf("[%d]", number);
            }
            
            printf(", ax");
            
            size -= 1;
        }
        
        else
        {
            printf("ERROR: opcode given by first byte [%c%c%c%c%c%c%c%c] not implemented\n", 
                   BYTE_TO_BINARY(first_byte));
            
            break;
        }
        
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stdout, "USAGE: %s [compiled 8086 program]\n", argv[0]);
        return 1;
    }
    
    char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        u32 size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        u8 *memory = (u8 *)malloc(size);
        fread(memory, size, size, file);
        
        fclose(file);
        
        
        printf("bits 16\n");
        
        decode_asm_8086(size, memory);
    }
    else
    {
        printf("ERROR: could not read file %s\n", filename);
    }
    
    return 0;
}

