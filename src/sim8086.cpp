#include <stdio.h>
#include <stdlib.h>

#include "sim8086.h"

u8 get_next_byte(FileContent *file_content)
{
    u8 byte = *(u8 *)file_content->memory++;
    --file_content->size_remaining;
    
    return byte;
}

u16 get_next_word(FileContent *file_content)
{
    u16 word = *(u16 *)file_content->memory;
    file_content->memory += 2;
    file_content->size_remaining -= 2;
    
    return word;
}


void print_memory_address_and_displacement(u8 w, u8 rm, u8 mod, FileContent *file_content)
{
    if (mod != MOD_REGISTER_MODE)
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
    }
    
    switch (mod)
    {
        case MOD_MEMORY_MODE: {
            if (rm == 0b110) {
                u16 displacement = get_next_word(file_content);
                printf("%d]", displacement);
            } else {
                printf("]");
            }
        } break;
        
        case MOD_MEMORY_MODE_8_BIT_DISPLACEMENT: {
            s8 number = get_next_byte(file_content);
            
            char sign = (number < 0) ? '-' : '+';
            if (number) {
                printf(" %c %d]", sign, (number < 0) ? -number : number);
            } else {
                printf("]");
            }
        } break;
        
        case MOD_MEMORY_MODE_16_BIT_DISPLACEMENT: {
            s16 number = get_next_word(file_content);
            
            char sign = (number < 0) ? '-' : '+';
            if (number) {
                printf(" %c %d]", sign, (number < 0) ? -number : number);
            } else {
                printf("]");
            }
        } break;
        
        case MOD_REGISTER_MODE: {
            char *register_name = register_table[(w << 3) | rm];
            printf("%s", register_name);
        } break;
    }
}


void print_instruction(Instruction instruction, FileContent *file_content)
{
    switch (instruction.operation_type)
    {
        case Op_add: { printf("add "); } break;
        case Op_sub: { printf("sub "); } break;
        case Op_cmp: { printf("cmp "); } break;
    }
    
    if (instruction.flags & PRINT_WORD_BYTE_TEXT) {
        if (instruction.w) {
            printf("word ");
        } else {
            printf("byte ");
        }
    }
    
    char *reg_register_name = register_table[(instruction.w << 3) | instruction.reg];
    if (instruction.flags & REG_SOURCE_DEST) {
        if (instruction.d) {
            printf("%s, ", reg_register_name);
        }
    }
    
    if (instruction.flags & ACCUMULATOR) {
        printf("%s", reg_register_name);
    }
    
    if (instruction.flags & PRINT_DISPLACEMENT) {
        print_memory_address_and_displacement(instruction.w, instruction.rm, instruction.mod, file_content);
    }
    
    if (instruction.flags & REG_SOURCE_DEST) {
        if (!instruction.d) {
            printf(", %s", reg_register_name);
        }
    } else {
        if (instruction.s == 0 && instruction.w == 1) {
            u16 number = get_next_word(file_content);
            printf(", %d", number);
        } else {
            u8 number = get_next_byte(file_content);
            printf(", %d", number);
        }
    }
}


void decode_asm_8086(FileContent *file_content)
{
    while (file_content->size_remaining)
    {
        u8 first_byte = get_next_byte(file_content);
        
        if (((first_byte >> 2) & 0b111111) == OPCODE_MOV_REGISTER_MEMORY_TO_OR_FROM_REGISTER)
        {
            printf("mov ");
            
            u8 second_byte = get_next_byte(file_content);
            
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
            }
            else
            {
                u8 rm = second_byte & 0b111;
                u8 reg = ((second_byte >> 3) & 0b111);
                
                char *reg_register_name = register_table[(w << 3) | reg];
                
                if (d) {
                    printf("%s, ", reg_register_name);
                }
                
                print_memory_address_and_displacement(w, rm, mod, file_content);
                
                if (!d) {
                    printf(", %s", reg_register_name);
                }
            }
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_IMMEDIATE_REGISTER_OR_MEMORY)
        {
            printf("mov ");
            
            u8 second_byte = get_next_byte(file_content);
            
            u8 w = first_byte & 1;
            u8 mod = (second_byte >> 6) & 0b11;
            u8 rm = second_byte & 0b111;
            
            print_memory_address_and_displacement(w, rm, mod, file_content);
            
            if (w) {
                u16 number = get_next_word(file_content);
                printf(", word %d", number);
            } else {
                u8 number = get_next_byte(file_content);
                printf(", byte %d", number);
            }
        }
        else if (((first_byte >> 4) & 0b1111) == OPCODE_MOV_IMMEDIATE_TO_REGISTER)
        {
            printf("mov ");
            
            u8 w = (first_byte >> 3) & 1;
            u8 w_plus_reg = first_byte & 0b1111;
            char *register_name = register_table[w_plus_reg];
            
            if (w == 1) {
                u16 number = *(u16 *)file_content->memory;
                u8 second_byte = get_next_byte(file_content);
                u8 third_byte = get_next_byte(file_content);
                
                u16 number2 = (third_byte << 8) | second_byte;
                printf("%s, %d", register_name, number);
            } else {
                u8 number = get_next_byte(file_content);
                printf("%s, %d", register_name, number);
            }
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_MEMORY_TO_ACCUMULATOR)
        {
            printf("mov ");
            
            printf("ax, ");
            
            u8 w = first_byte & 1;
            
            if (w) {
                u16 number = get_next_word(file_content);
                printf("[%d]", number);
            } else {
                u8 number = get_next_byte(file_content);
                printf("[%d]", number);
            }
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_ACCUMULATOR_TO_MEMORY)
        {
            printf("mov ");
            
            u8 w = first_byte & 1;
            
            if (w) {
                u16 number = get_next_word(file_content);
                printf("[%d]", number);
            } else {
                u8 number = get_next_byte(file_content);
                printf("[%d]", number);
            }
            
            printf(", ax");
        }
        else if (((first_byte >> 2) & 0b111111) == OPCODE_ARITHMETIC_IMMEDIATE_TO_REGISTER_OR_MEMORY)
        {
            u8 second_byte = get_next_byte(file_content);
            
            Instruction instruction = {};
            instruction.s = (first_byte >> 1) & 1;
            instruction.w = first_byte & 1;
            instruction.mod = (second_byte >> 6) & 0b11;
            instruction.rm = second_byte & 0b111;
            instruction.operation_type = operation_types[(second_byte >> 3) & 0b111];
            instruction.flags = PRINT_WORD_BYTE_TEXT | PRINT_DISPLACEMENT;
            
            print_instruction(instruction, file_content);
        }
        else if ((((first_byte >> 2) & 0b111111) == OPCODE_ADD_REGISTER_OR_MEMORY) ||
                 (((first_byte >> 2) & 0b111111) == OPCODE_SUB_REGISTER_OR_MEMORY) ||
                 (((first_byte >> 2) & 0b111111) == OPCODE_CMP_REGISTER_OR_MEMORY))
        {
            u8 second_byte = get_next_byte(file_content);
            
            Instruction instruction = {};
            instruction.d = (first_byte >> 1) & 1;
            instruction.w = first_byte & 1;
            instruction.mod = (second_byte >> 6) & 0b11;
            instruction.reg = (second_byte >> 3) & 0b111;
            instruction.rm = second_byte & 0b111;
            instruction.operation_type = operation_types[(first_byte >> 3) & 0b111];
            instruction.flags = REG_SOURCE_DEST | PRINT_DISPLACEMENT;
            
            print_instruction(instruction, file_content);
        }
        else if ((((first_byte >> 1) & 0b1111111) == OPCODE_ADD_IMMEDIATE_TO_ACCUMULATOR) ||
                 (((first_byte >> 1) & 0b1111111) == OPCODE_SUB_IMMEDIATE_FROM_ACCUMULATOR) ||
                 (((first_byte >> 1) & 0b1111111) == OPCODE_CMP_IMMEDIATE_WITH_ACCUMULATOR))
        {
            Instruction instruction = {};
            instruction.w = first_byte & 1;
            instruction.reg = 0b000; // ax || al register
            instruction.operation_type = operation_types[(first_byte >> 3) & 0b111];
            instruction.flags = ACCUMULATOR;
            
            print_instruction(instruction, file_content);
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
        fread(memory, size, 1, file);
        
        fclose(file);
        
        FileContent content = {};
        content.memory         = memory;
        content.total_size     = size;
        content.size_remaining = size;
        
        printf("bits 16\n");
        decode_asm_8086(&content);
    }
    else
    {
        printf("ERROR: could not read file %s\n", filename);
    }
    
    return 0;
}

