#include <stdio.h>
#include <stdlib.h>

typedef char u8;
typedef unsigned short u16;
typedef unsigned int u32;


#define assert(expression) if (!(expression)) {*((int *)0) = 0;}

#define OPCODE_TO_BINARY(byte) \
(((byte) & (1 << 5)) ? '1' : '0'), \
(((byte) & (1 << 4)) ? '1' : '0'), \
(((byte) & (1 << 3)) ? '1' : '0'), \
(((byte) & (1 << 2)) ? '1' : '0'), \
(((byte) & (1 << 1)) ? '1' : '0'), \
(((byte) & (1 << 0)) ? '1' : '0')


enum Opcode {
    OPCODE_MOV = 0b100010,
};

enum Mod {
    MOD_REGISTER = 0b11,
};


int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stdout, "error\n");
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
        
        char *register_table[16];
        // Byte-size registers
        register_table[0b0000] = "al";
        register_table[0b0001] = "cl";
        register_table[0b0010] = "dl";
        register_table[0b0011] = "bl";
        register_table[0b0100] = "ah";
        register_table[0b0101] = "ch";
        register_table[0b0110] = "dh";
        register_table[0b0111] = "bh";
        
        // Word-size registers
        register_table[0b1000] = "ax";
        register_table[0b1001] = "cx";
        register_table[0b1010] = "dx";
        register_table[0b1011] = "bx";
        register_table[0b1100] = "sp";
        register_table[0b1101] = "bp";
        register_table[0b1110] = "si";
        register_table[0b1111] = "di";
        
        
        printf("bits 16\n");
        
        while (size)
        {
            u8 first_byte = *memory++;
            u8 second_byte = *(memory++);
            
            u8 opcode = (first_byte >> 2) & 0b111111;
            u8 d = first_byte & 2;
            u8 w = first_byte & 1;
            
            switch (opcode)
            {
                case OPCODE_MOV: {
                    printf("mov ");
                    
                    u8 mod = ((second_byte & 0b11000000) >> 6);
                    if (mod == MOD_REGISTER) {
                        u8 rm = second_byte & 0b111;
                        char *register_name = register_table[(w << 3) | rm];
                        printf("%s, ", register_name);
                    }
                    
                    
                    u8 reg = ((second_byte & 0b111000) >> 3);
                    char *register_name = register_table[(w << 3) | reg];
                    printf("%s", register_name);
                    
                } break;
                
                default: {
                    printf("[ERROR]: opcode %c%c%c%c%c%c not implemented\n", OPCODE_TO_BINARY(opcode));
                } break;
            }
            
            
            printf("\n");
            
            size -= 2;
        }
    }
    
    return 0;
}

