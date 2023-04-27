#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void print_memory_address_and_displacement(Instruction instruction)
{
    DisplacementAddress displacement = instruction.displacement_address;
    if (displacement.first_displacement.type != Register_none ||
        displacement.second_displacement.type != Register_none ||
        displacement.offset)
    {
        printf("[");
        
        if (displacement.first_displacement.type != Register_none) {
            char *first_register = displacement.first_displacement.name;
            printf("%s", first_register);
            
            if (displacement.second_displacement.type != Register_none) {
                char *second_register = displacement.second_displacement.name;
                printf(" + %s", second_register);
            }
            
            if (displacement.offset) {
                bool is_negative = (displacement.offset < 0);
                char sign = is_negative ? '-' : '+';
                s16 offset = is_negative ? displacement.offset * -1 : displacement.offset;
                printf(" %c %d", sign, offset);
            }
            
        } else {
            if (displacement.offset) {
                bool is_negative = (displacement.offset < 0);
                if (is_negative) {
                    printf("- ");
                }
                s16 offset = is_negative ? displacement.offset * -1 : displacement.offset;
                printf("%d", offset);
            }
        }
        
        printf("]");
    }
}

char * get_opcode_name(OperationType operation_type, u8 binary)
{
    char *operation;
    switch (operation_type)
    {
        case Op_mov: { operation = "mov "; } break;
        case Op_add: { operation = "add "; } break;
        case Op_sub: { operation = "sub "; } break;
        case Op_cmp: { operation = "cmp "; } break;
        case Op_jmp: {
            switch (binary)
            {
                case OPCODE_JE:     { operation = "je"; } break;
                case OPCODE_JL:     { operation = "jl"; } break;
                case OPCODE_JLE:    { operation = "jle"; } break;
                case OPCODE_JB:     { operation = "jb"; } break;
                case OPCODE_JBE:    { operation = "jbe"; } break;
                case OPCODE_JP:     { operation = "jp"; } break;
                case OPCODE_JO:     { operation = "jo"; } break;
                case OPCODE_JS:     { operation = "js"; } break;
                case OPCODE_JNE:    { operation = "jne"; } break;
                case OPCODE_JNL:    { operation = "jnl"; } break;
                case OPCODE_JNLE:   { operation = "jnle"; } break;
                case OPCODE_JNB:    { operation = "jnb"; } break;
                case OPCODE_JNBE:   { operation = "jnbe"; } break;
                case OPCODE_JNP:    { operation = "jnp"; } break;
                case OPCODE_JNO:    { operation = "jno"; } break;
                case OPCODE_JNS:    { operation = "jns"; } break;
                case OPCODE_LOOP:   { operation = "loop"; } break;
                case OPCODE_LOOPZ:  { operation = "loopz"; } break;
                case OPCODE_LOOPNZ: { operation = "loopnz"; } break;
                case OPCODE_JCXZ:   { operation = "jcxz"; } break;
            }
        } break;
    }
    
    return operation;
}

void print_instruction(Instruction *instruction_, FileContent *file_content)
{
    Instruction instruction = *instruction_;
    
    printf(get_opcode_name(instruction.operation_type, instruction.binary));
    
    if (instruction.operation_type == Op_jmp) {
        // TODO: based on the increment place a label in the assembly and use that name here (it does not work with the number directly).
        s8 increment = get_next_byte(file_content);
        printf(" %d", increment);
        return;
    }
    
    if (instruction.flags & WORD_BYTE_TEXT_REQUIRED) {
        if (instruction.w) {
            printf("word ");
        } else {
            printf("byte ");
        }
    }
    
    
    if (instruction.flags & IMMEDIATE_ACCUMULATOR) {
        printf("%s", instruction.dest_register.name);
    }
    
    if (instruction.flags & REG_SOURCE_DEST && instruction.dest_register.type != Register_none) {
        printf("%s, ", instruction.dest_register.name);
    } else if (instruction.mod == MOD_REGISTER_MODE) {
        printf("%s", instruction.dest_register.name);
    }
    
    if (instruction.flags & DISPLACEMENT) {
        print_memory_address_and_displacement(instruction);
    }
    
    if (instruction.flags & IMMEDIATE) {
        u16 number = instruction.value;
        printf("%s, %d", instruction.dest_register.name, number);
    } else if (instruction.flags & ACCUMULATOR_ADDRESS) {
        u16 number = instruction.value;
        if (instruction.d) {
            printf("%s, [%d]", instruction.dest_register.name, number);
        } else {
            printf("[%d], %s", number, instruction.source_register.name);
        }
    } else if (instruction.flags & REG_SOURCE_DEST) {
        if (instruction.source_register.type != Register_none) {
            if (instruction.flags & DISPLACEMENT &&
                instruction.displacement_address.first_displacement.type != Register_none) {
                printf(", ");
            }
            
            printf("%s", instruction.source_register.name);
        }
    } else {
        printf(", %d", instruction.value);
    }
}

void print_instructions(Instruction *instructions, u32 count, FileContent *file_content)
{
    if (count)
    {
        printf("bits 16\n");
        for (int i = 0; i < count; ++i) {
            Instruction instruction = instructions[i];
            print_instruction(&instruction, file_content);
            printf("\n");
        }
    }
}

bool is_opcode_jump(u8 opcode)
{
    bool is_jump = ((opcode == OPCODE_JE) ||
                    (opcode == OPCODE_JL) ||
                    (opcode == OPCODE_JLE) ||
                    (opcode == OPCODE_JB) ||
                    (opcode == OPCODE_JBE) ||
                    (opcode == OPCODE_JP) ||
                    (opcode == OPCODE_JO) ||
                    (opcode == OPCODE_JS) ||
                    (opcode == OPCODE_JNE) ||
                    (opcode == OPCODE_JNL) ||
                    (opcode == OPCODE_JNLE) ||
                    (opcode == OPCODE_JNB) ||
                    (opcode == OPCODE_JNBE) ||
                    (opcode == OPCODE_JNP) ||
                    (opcode == OPCODE_JNO) ||
                    (opcode == OPCODE_JNS) ||
                    (opcode == OPCODE_LOOP) ||
                    (opcode == OPCODE_LOOPZ) ||
                    (opcode == OPCODE_LOOPNZ) ||
                    (opcode == OPCODE_JCXZ));
    
    return is_jump;
}

void set_source_and_dest_registers(Instruction *instruction, FileContent *file_content)
{
    RegisterDefinition reg_register = get_register_definition(instruction->w, instruction->reg);
    RegisterDefinition rm_register  = get_register_definition(instruction->w, instruction->rm);
    
    if (instruction->flags & HAS_DATA) {
        u16 number = (instruction->s == 0 && instruction->w == 1) 
            ? get_next_word(file_content) : (u16)get_next_byte(file_content);
        instruction->value = number;
    }
    
    if (instruction->flags & REG_SOURCE_DEST) {
        if (instruction->d) {
            instruction->dest_register = reg_register;
        } else {
            instruction->source_register = reg_register;
        }
    } else if (instruction->flags & IMMEDIATE ||
               instruction->flags & IMMEDIATE_ACCUMULATOR) {
        instruction->dest_register = reg_register;
    } else if (instruction->flags & ACCUMULATOR_ADDRESS) {
        u16 number = get_next_word(file_content);
        instruction->value = number;
        if (instruction->d) {
            instruction->dest_register = reg_register;
        } else {
            instruction->source_register = reg_register;
        }
    }
    
    if (instruction->mod == MOD_REGISTER_MODE) {
        instruction->dest_register = rm_register;
    }
}

void calculate_displacement(Instruction *instruction, FileContent *file_content)
{
    if (!(instruction->flags & DISPLACEMENT)) {
        return;
    }
    
    DisplacementAddress displacement = {};
    
    if (instruction->mod != MOD_REGISTER_MODE)
    {
        switch (instruction->rm)
        {
            case 0b000: {
                displacement.first_displacement = registers_definitions[3][1];
                displacement.second_displacement = registers_definitions[6][1];
            } break;
            
            case 0b001: {
                displacement.first_displacement = registers_definitions[3][1];
                displacement.second_displacement = registers_definitions[7][1];
            } break;
            
            case 0b010: {
                displacement.first_displacement = registers_definitions[5][1];
                displacement.second_displacement = registers_definitions[6][1];
            } break;
            
            case 0b011: {
                displacement.first_displacement = registers_definitions[5][1];
                displacement.second_displacement = registers_definitions[7][1];
            } break;
            
            case 0b100: {
                displacement.first_displacement = registers_definitions[6][1];
            } break;
            
            case 0b101: {
                displacement.first_displacement = registers_definitions[7][1];
            } break;
            
            case 0b110: {
                if (instruction->mod != MOD_MEMORY_MODE) {
                    displacement.first_displacement = registers_definitions[5][1];
                }
            } break;
            
            case 0b111: {
                displacement.first_displacement = registers_definitions[3][1];
            } break;
        }
    }
    
    switch (instruction->mod)
    {
        case MOD_MEMORY_MODE: {
            if (instruction->rm == 0b110) {
                displacement.offset = (s16)get_next_word(file_content);
            }
        } break;
        
        case MOD_MEMORY_MODE_8_BIT_DISPLACEMENT: {
            displacement.offset = (s8)get_next_byte(file_content);
        } break;
        
        case MOD_MEMORY_MODE_16_BIT_DISPLACEMENT: {
            displacement.offset = (s16)get_next_word(file_content);
        } break;
        
        case MOD_REGISTER_MODE: {
            // No displacement
        } break;
    }
    
    instruction->displacement_address = displacement;
}

u32 decode_asm_8086(FileContent *file_content, Instruction *instructions)
{
    u32 count = 0;
    while (file_content->size_remaining)
    {
        u8 first_byte = get_next_byte(file_content);
        
        Instruction instruction = {};
        instruction.binary = first_byte;
        
        if (((first_byte >> 2) & 0b111111) == OPCODE_MOV_REGISTER_MEMORY_TO_OR_FROM_REGISTER)
        {
            u8 second_byte = get_next_byte(file_content);
            
            instruction.operation_type = Op_mov;
            instruction.w = first_byte & 1;
            instruction.d = first_byte & 2;
            instruction.mod = ((second_byte >> 6) & 0b11);
            instruction.rm = second_byte & 0b111;
            instruction.reg = ((second_byte >> 3) & 0b111);
            instruction.flags = REG_SOURCE_DEST;
            if (instruction.mod != MOD_REGISTER_MODE) {
                instruction.flags |= DISPLACEMENT;
            }
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_IMMEDIATE_TO_REGISTER_OR_MEMORY)
        {
            u8 second_byte = get_next_byte(file_content);
            
            instruction.operation_type = Op_mov;
            instruction.w = first_byte & 1;
            instruction.mod = (second_byte >> 6) & 0b11;
            instruction.rm = second_byte & 0b111;
            instruction.flags = WORD_BYTE_TEXT_REQUIRED | DISPLACEMENT | HAS_DATA;
            
            // TODO: I need to store the value in the next byte (or word). I did this already, just look it up
        }
        else if (((first_byte >> 4) & 0b1111) == OPCODE_MOV_IMMEDIATE_TO_REGISTER)
        {
            instruction.operation_type = Op_mov;
            instruction.w = (first_byte >> 3) & 1;
            instruction.reg = first_byte & 0b111;
            instruction.flags = IMMEDIATE | HAS_DATA;
        }
        else if (((first_byte >> 1) & 0b1111111) == OPCODE_MOV_MEMORY_TO_ACCUMULATOR ||
                 ((first_byte >> 1) & 0b1111111) == OPCODE_MOV_ACCUMULATOR_TO_MEMORY)
        {
            instruction.operation_type = Op_mov;
            instruction.w = first_byte & 1;
            instruction.reg = 0b000;
            instruction.d = ((first_byte >> 1) & 1) ^ 1;
            instruction.flags = ACCUMULATOR_ADDRESS;
        }
        else if (((first_byte >> 2) & 0b111111) == OPCODE_ARITHMETIC_IMMEDIATE_TO_REGISTER_OR_MEMORY)
        {
            u8 second_byte = get_next_byte(file_content);
            
            instruction.s = (first_byte >> 1) & 1;
            instruction.w = first_byte & 1;
            instruction.mod = (second_byte >> 6) & 0b11;
            instruction.rm = second_byte & 0b111;
            instruction.operation_type = arithmetic_operations[(second_byte >> 3) & 0b111];
            instruction.flags = WORD_BYTE_TEXT_REQUIRED | DISPLACEMENT | HAS_DATA;
        }
        else if ((((first_byte >> 2) & 0b111111) == OPCODE_ADD_REGISTER_OR_MEMORY) ||
                 (((first_byte >> 2) & 0b111111) == OPCODE_SUB_REGISTER_OR_MEMORY) ||
                 (((first_byte >> 2) & 0b111111) == OPCODE_CMP_REGISTER_OR_MEMORY))
        {
            u8 second_byte = get_next_byte(file_content);
            
            instruction.d = (first_byte >> 1) & 1;
            instruction.w = first_byte & 1;
            instruction.mod = (second_byte >> 6) & 0b11;
            instruction.reg = (second_byte >> 3) & 0b111;
            instruction.rm = second_byte & 0b111;
            instruction.operation_type = arithmetic_operations[(first_byte >> 3) & 0b111];
            instruction.flags = REG_SOURCE_DEST | DISPLACEMENT;
        }
        else if ((((first_byte >> 1) & 0b1111111) == OPCODE_ADD_IMMEDIATE_TO_ACCUMULATOR) ||
                 (((first_byte >> 1) & 0b1111111) == OPCODE_SUB_IMMEDIATE_FROM_ACCUMULATOR) ||
                 (((first_byte >> 1) & 0b1111111) == OPCODE_CMP_IMMEDIATE_WITH_ACCUMULATOR))
        {
            instruction.w = first_byte & 1;
            instruction.reg = 0b000; // ax || al register
            instruction.operation_type = arithmetic_operations[(first_byte >> 3) & 0b111];
            instruction.flags = IMMEDIATE_ACCUMULATOR | HAS_DATA;
        }
        else if (is_opcode_jump(first_byte))
        {
            instruction.operation_type = Op_jmp;
        }
        else
        {
            printf("ERROR: opcode given by first byte [%c%c%c%c%c%c%c%c] not implemented\n", 
                   BYTE_TO_BINARY(first_byte));
            
            count = 0;
            break;
        }
        
        calculate_displacement(&instruction, file_content);
        set_source_and_dest_registers(&instruction, file_content);
        
        instructions[count++] = instruction;
    }
    
    return count;
}

void print_usage(char *program_name)
{
    fprintf(stdout, "USAGE:  %s [flags] [compiled 8086 program]\n", program_name);
    fprintf(stdout, "    flags:\n");
    fprintf(stdout, "        nothing: print the dissasembly\n");
    fprintf(stdout, "        --sim: simulate the dissasembly \n");
}

void print_final_state(State state)
{
    printf("\n");
    printf("Final registers:\n");
    printf("\tax: 0x%04hhx (%d)\n", state.registers[0].value, state.registers[0].value);
    printf("\tbx: 0x%04hhx (%d)\n", state.registers[1].value, state.registers[1].value);
    printf("\tcx: 0x%04hhx (%d)\n", state.registers[2].value, state.registers[2].value);
    printf("\tdx: 0x%04hhx (%d)\n", state.registers[3].value, state.registers[3].value);
    printf("\tsp: 0x%04hhx (%d)\n", state.registers[4].value, state.registers[4].value);
    printf("\tbp: 0x%04hhx (%d)\n", state.registers[5].value, state.registers[5].value);
    printf("\tsi: 0x%04hhx (%d)\n", state.registers[6].value, state.registers[6].value);
    printf("\tdi: 0x%04hhx (%d)\n", state.registers[7].value, state.registers[7].value);
}

void simulate_instruction(State *state, Instruction instruction)
{
    if (instruction.source_register.type == Register_none) {
        state->registers[instruction.dest_register.type - 1].value = instruction.value;
    } else {
        state->registers[instruction.dest_register.type - 1].value =
            state->registers[instruction.source_register.type - 1].value;
    }
}

void simulate_asm_8086(Instruction *instructions, u32 count, FileContent *file_content)
{
    // initialize state
    State state = {};
    state.registers[0] = {Register_a, 0};
    state.registers[1] = {Register_b, 0};
    state.registers[2] = {Register_c, 0};
    state.registers[3] = {Register_d, 0};
    state.registers[4] = {Register_sp, 0};
    state.registers[5] = {Register_bp, 0};
    state.registers[6] = {Register_si, 0};
    state.registers[7] = {Register_di, 0};
    
    for (int i = 0; i < count; ++i) {
        Instruction instruction = instructions[i];
        simulate_instruction(&state, instruction);
    }
    
    print_instructions(instructions, count, file_content);
    
    print_final_state(state);
}

int main(int argc, char **argv)
{
    char *program_name = argv[0];
    if (argc < 2) {
        print_usage(program_name);
        return 1;
    }
    
    char *filename;
    char *flag;
    
    bool simulate = false;
    
    if (argc == 3) {
        flag = argv[1];
        
        if (!str_equals(flag, "--sim")) {
            print_usage(program_name);
            return 2;
        }
        
        filename = argv[2];
        simulate = true;
    } else {
        filename = argv[1];
    }
    
    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        u32 size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        u8 *memory = (u8 *)malloc(size);
        fread(memory, size, 1, file);
        
        fclose(file);
        
        FileContent file_content = {};
        file_content.memory         = memory;
        file_content.total_size     = size;
        file_content.size_remaining = size;
        
        
        Instruction instructions[256];
        u32 count = decode_asm_8086(&file_content, instructions);
        
        if (simulate) {
            simulate_asm_8086(instructions, count, &file_content);
        } else {
            print_instructions(instructions, count, &file_content);
        }
    }
    else
    {
        printf("ERROR: could not read file %s\n", filename);
    }
    
    return 0;
}

