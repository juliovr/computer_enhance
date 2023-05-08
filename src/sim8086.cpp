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

void print_instruction(Instruction *instruction_)
{
    Instruction instruction = *instruction_;
    
    printf(get_opcode_name(instruction.operation_type, instruction.binary));
    
    if (instruction.operation_type == Op_jmp) {
        printf(" $%d", ((s8)instruction.value + 2)); // Explicit ip value (instead of label) should not consider the 2 bytes used in this instruction.
        
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
    } else if ((instruction.flags & SEGMENT) && instruction.source_register.type != Register_none) {
        printf(", %s", instruction.source_register.name);
    } else {
        printf(", %d", instruction.value);
    }
}

void print_instructions(Instruction *instructions, u32 count)
{
    if (count)
    {
        printf("bits 16\n");
        for (int i = 0; i < count; ++i) {
            Instruction instruction = instructions[i];
            print_instruction(&instruction);
            
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
        u16 number;
        if (instruction->w == 1) {
            if (instruction->s == 0)  {
                number = get_next_word(file_content);
                instruction->bytes_used += 2;
            } else {
                number = (u16)get_next_byte(file_content);
                if (number < 0) {
                    // Sign-extended
                    number |= 0xFF00;
                }
                
                instruction->bytes_used += 1;
            }
        } else {
            number = (u16)get_next_byte(file_content);
            instruction->bytes_used += 1;
        }
        
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
        instruction->bytes_used += 2
            ;
        instruction->value = number;
        if (instruction->d) {
            instruction->dest_register = reg_register;
        } else {
            instruction->source_register = reg_register;
        }
    }
    
    if (instruction->mod == MOD_REGISTER_MODE) {
        if (instruction->flags & SEGMENT) {
            RegisterDefinition segment_register = get_segment_register_definition(instruction->sr);
            if (instruction->d) {
                instruction->dest_register = rm_register;
                instruction->source_register = segment_register;
            } else {
                instruction->dest_register = segment_register;
                instruction->source_register = rm_register;
            }
        } else {
            instruction->dest_register = rm_register;
        }
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
                instruction->bytes_used += 2;
            }
        } break;
        
        case MOD_MEMORY_MODE_8_BIT_DISPLACEMENT: {
            displacement.offset = (s8)get_next_byte(file_content);
            instruction->bytes_used += 1;
        } break;
        
        case MOD_MEMORY_MODE_16_BIT_DISPLACEMENT: {
            displacement.offset = (s16)get_next_word(file_content);
            instruction->bytes_used += 2;
        } break;
        
        case MOD_REGISTER_MODE: {
            // No displacement
        } break;
    }
    
    instruction->displacement_address = displacement;
}

u32 decode_asm_8086(FileContent *file_content, Instruction *instructions)
{
    u32 instruction_count = 0;
    while (file_content->size_remaining)
    {
        Instruction instruction = {};
        
        u8 first_byte = get_next_byte(file_content);
        instruction.bytes_used += 1;
        
        instruction.binary = first_byte;
        
        if (((first_byte >> 2) & 0b111111) == OPCODE_MOV_REGISTER_MEMORY_TO_OR_FROM_REGISTER)
        {
            u8 second_byte = get_next_byte(file_content);
            instruction.bytes_used += 1;
            
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
            instruction.bytes_used += 1;
            
            instruction.operation_type = Op_mov;
            instruction.w = first_byte & 1;
            instruction.mod = (second_byte >> 6) & 0b11;
            instruction.rm = second_byte & 0b111;
            instruction.flags = WORD_BYTE_TEXT_REQUIRED | DISPLACEMENT | HAS_DATA;
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
        else if ((first_byte == OPCODE_MOV_REGISTER_OR_MEMORY_TO_SEGMENT_REGISTER) ||
                 (first_byte == OPCODE_MOV_SEGMENT_REGISTER_TO_REGISTER_OR_MEMORY))
        {
            u8 second_byte = get_next_byte(file_content);
            instruction.bytes_used += 1;
            
            instruction.operation_type = Op_mov;
            instruction.w = 1;
            instruction.mod = (second_byte >> 6) & 0b11;
            instruction.sr = (second_byte >> 3) & 0b111;
            instruction.rm = second_byte & 0b111;
            instruction.d = ((first_byte >> 1) & 1) ^ 1;
            instruction.flags = SEGMENT;
        }
        else if (((first_byte >> 2) & 0b111111) == OPCODE_ARITHMETIC_IMMEDIATE_TO_REGISTER_OR_MEMORY)
        {
            u8 second_byte = get_next_byte(file_content);
            instruction.bytes_used += 1;
            
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
            instruction.bytes_used += 1;
            
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
            u8 second_byte = get_next_byte(file_content);
            instruction.bytes_used += 1;
            
            instruction.operation_type = Op_jmp;
            instruction.value = second_byte;
        }
        else
        {
            printf("ERROR: opcode given by first byte [%c%c%c%c%c%c%c%c] not implemented\n", 
                   BYTE_TO_BINARY(first_byte));
            
            instruction_count = 0;
            break;
        }
        
        calculate_displacement(&instruction, file_content);
        set_source_and_dest_registers(&instruction, file_content);
        
        instructions[instruction_count++] = instruction;
    }
    
    return instruction_count;
}

void print_usage(char *program_name)
{
    fprintf(stdout, "USAGE:  %s [flags] [compiled 8086 program]\n", program_name);
    fprintf(stdout, "    flags:\n");
    fprintf(stdout, "        nothing: print the dissasembly\n");
    fprintf(stdout, "        --sim: simulate the instructions\n");
}

void print_final_state(State state)
{
    printf("\n");
    printf("Final registers:\n");
    printf("\tax: 0x%04hx (%d)\n", state.registers[0].value, state.registers[0].value);
    printf("\tbx: 0x%04hx (%d)\n", state.registers[1].value, state.registers[1].value);
    printf("\tcx: 0x%04hx (%d)\n", state.registers[2].value, state.registers[2].value);
    printf("\tdx: 0x%04hx (%d)\n", state.registers[3].value, state.registers[3].value);
    printf("\tsp: 0x%04hx (%d)\n", state.registers[4].value, state.registers[4].value);
    printf("\tbp: 0x%04hx (%d)\n", state.registers[5].value, state.registers[5].value);
    printf("\tsi: 0x%04hx (%d)\n", state.registers[6].value, state.registers[6].value);
    printf("\tdi: 0x%04hx (%d)\n", state.registers[7].value, state.registers[7].value);
    printf("\tes: 0x%04hx (%d)\n", state.registers[8].value, state.registers[8].value);
    printf("\tcs: 0x%04hx (%d)\n", state.registers[9].value, state.registers[9].value);
    printf("\tss: 0x%04hx (%d)\n", state.registers[10].value, state.registers[10].value);
    printf("\tds: 0x%04hx (%d)\n", state.registers[11].value, state.registers[11].value);
    printf("\tip: 0x%04hx (%d)\n", state.registers[12].value, state.registers[12].value);
    printf("\n");
    
    printf("    Flags: ");
    if (state.carry_flag)           { printf("C"); }
    if (state.parity_flag)          { printf("P"); }
    if (state.auxiliary_carry_flag) { printf("A"); }
    if (state.zero_flag)            { printf("Z"); }
    if (state.sign_flag)            { printf("S"); }
    if (state.overflow_flag)        { printf("O"); }
}

s8 simulate_instruction(State *state, Instruction *instruction)
{
    s8 instructions_to_move = 1;
    
    state->ip_register.value += instruction->bytes_used;
    
    // Calculate memory displacement
    s16 displacement = instruction->displacement_address.offset;
    if (instruction->displacement_address.first_displacement.type != Register_none) {
        displacement += state->registers[instruction->displacement_address.first_displacement.type - 1].value;
    }
    
    if (instruction->displacement_address.second_displacement.type != Register_none) {
        displacement += state->registers[instruction->displacement_address.second_displacement.type - 1].value;
    }
    
    
    switch (instruction->operation_type)
    {
        case Op_mov: {
            if (instruction->source_register.type == Register_none)
            {
                if (instruction->dest_register.type == Register_none) {
                    // To memory
                    *(state->memory + displacement) = (instruction->value & 0xFF);
                    if (instruction->w) {
                        *(state->memory + displacement + 1) = (instruction->value >> 8);
                    }
                } else if ((instruction->dest_register.bytes >> 3) & 1) {
                    if (displacement) {
                        state->registers[instruction->dest_register.type - 1].value = *((u16 *)(state->memory + displacement));
                    } else {
                        state->registers[instruction->dest_register.type - 1].value = instruction->value;
                    }
                } else {
                    u16 value;
                    if (instruction->dest_register.bytes & 0b0100) {
                        // High bits
                        state->registers[instruction->dest_register.type - 1].value &= 0x00FF;
                        value = (instruction->value << 8);
                    } else {
                        // Lower bits
                        state->registers[instruction->dest_register.type - 1].value &= 0xFF00;
                        value = (u8)instruction->value;
                    }
                    
                    state->registers[instruction->dest_register.type - 1].value |= value;
                }
            }
            else
            {
                u16 source_register_value = state->registers[instruction->source_register.type - 1].value;
                if (instruction->dest_register.type == Register_none) {
                    // To memory
                    *(state->memory + displacement) = (source_register_value & 0xFF);
                    if (instruction->w) {
                        *(state->memory + displacement + 1) = (source_register_value >> 8);
                    }
                } else if ((instruction->source_register.bytes >> 3) & 1) {
                    if (displacement) {
                        state->registers[instruction->dest_register.type - 1].value = *((u16 *)(state->memory + displacement));
                    } else {
                        state->registers[instruction->dest_register.type - 1].value = source_register_value;
                    }
                    
                } else {
                    u16 value;
                    if (instruction->source_register.bytes & 0b0100) {
                        // High bits
                        value = (source_register_value >> 8);
                    } else {
                        // Lower bits
                        value = (source_register_value & 0xFF);
                    }
                    
                    if (instruction->dest_register.bytes & 0b0100) {
                        // High bits
                        state->registers[instruction->dest_register.type - 1].value &= 0x00FF;
                        state->registers[instruction->dest_register.type - 1].value |= (value << 8);
                    } else {
                        // Lower bits
                        state->registers[instruction->dest_register.type - 1].value &= 0xFF00;
                        state->registers[instruction->dest_register.type - 1].value |= (value & 0xFF);
                    }
                }
            }
            
            state->parity_flag = false;
            state->zero_flag = false;
            state->sign_flag = false;
            state->overflow_flag = false;
            state->carry_flag = false;
            state->auxiliary_carry_flag = false;
        } break;
        
        case Op_add: {
            u16 new_value;
            u16 prev_value = state->registers[instruction->dest_register.type - 1].value;
            u16 source_value;
            if (instruction->source_register.type == Register_none) {
                source_value = instruction->value;
            } else {
                source_value = state->registers[instruction->source_register.type - 1].value;
            }
            
            
            if (displacement) {
                if (instruction->dest_register.type == Register_none) {
                    *((u16 *)(state->memory + displacement)) += source_value;
                    new_value = *((u16 *)(state->memory + displacement));
                } else {
                    state->registers[instruction->dest_register.type - 1].value += *((u16 *)(state->memory + displacement));
                    new_value = state->registers[instruction->dest_register.type - 1].value;
                }
            } else {
                state->registers[instruction->dest_register.type - 1].value += source_value;
                new_value = state->registers[instruction->dest_register.type - 1].value;
            }
            
            state->parity_flag = (count_one_bits((u8)new_value) % 2 == 0);
            state->zero_flag = (new_value == 0);
            state->sign_flag = ((new_value >> 15) & 1);
            state->overflow_flag = (((prev_value & 0x8000) == (source_value & 0x8000)) && 
                                    ((new_value & 0x8000) != (prev_value & 0x8000)));
            state->carry_flag = (MAX_U16 - prev_value) < source_value;
            state->auxiliary_carry_flag = ((((prev_value >> 3) & 1) != ((new_value >> 3) & 1)) &&
                                           (((prev_value >> 4) & 1) != ((new_value >> 4) & 1)));
        } break;
        
        case Op_sub: {
            u16 prev_value = state->registers[instruction->dest_register.type - 1].value;
            u16 source_value;
            if (instruction->source_register.type == Register_none) {
                source_value = instruction->value;
            } else {
                source_value = state->registers[instruction->source_register.type - 1].value;
            }
            
            state->registers[instruction->dest_register.type - 1].value -= source_value;
            
            u16 new_value = state->registers[instruction->dest_register.type - 1].value;
            state->parity_flag = (count_one_bits((u8)new_value) % 2 == 0);
            state->zero_flag = (new_value == 0);
            state->sign_flag = ((new_value >> 15) & 1);
            state->overflow_flag = (((prev_value & 0x8000) != (source_value & 0x8000)) && 
                                    ((new_value & 0x8000) != (prev_value & 0x8000)));
            state->carry_flag = prev_value < source_value;
            state->auxiliary_carry_flag = ((((prev_value >> 3) & 1) != ((new_value >> 3) & 1)) &&
                                           (((prev_value >> 4) & 1) != ((new_value >> 4) & 1)));
        } break;
        
        case Op_cmp: {
            u16 prev_value = state->registers[instruction->dest_register.type - 1].value;
            u16 source_value;
            if (instruction->source_register.type == Register_none) {
                source_value = instruction->value;
            } else {
                source_value = state->registers[instruction->source_register.type - 1].value;
            }
            
            u16 new_value = prev_value - source_value;
            
            state->parity_flag = (count_one_bits((u8)new_value) % 2 == 0);
            state->zero_flag = (new_value == 0);
            state->sign_flag = ((new_value >> 15) & 1);
            state->overflow_flag = (((prev_value & 0x8000) != (source_value & 0x8000)) && 
                                    ((new_value & 0x8000) != (prev_value & 0x8000)));
            state->carry_flag = prev_value < source_value;
            state->auxiliary_carry_flag = ((((prev_value >> 3) & 1) != ((new_value >> 3) & 1)) &&
                                           (((prev_value >> 4) & 1) != ((new_value >> 4) & 1)));
        } break;
        
        case Op_jmp: {
            bool should_jump = false;
            switch (instruction->binary)
            {
                case OPCODE_JE:     { should_jump = state->zero_flag; } break;
                case OPCODE_JL:     { should_jump = false; } break;
                case OPCODE_JLE:    { should_jump = false; } break;
                case OPCODE_JB:     { should_jump = state->carry_flag; } break;
                case OPCODE_JBE:    { should_jump = false; } break;
                case OPCODE_JP:     { should_jump = state->parity_flag; } break;
                case OPCODE_JO:     { should_jump = false; } break;
                case OPCODE_JS:     { should_jump = false; } break;
                case OPCODE_JNE:    { should_jump = !state->zero_flag; } break;
                case OPCODE_JNL:    { should_jump = false; } break;
                case OPCODE_JNLE:   { should_jump = false; } break;
                case OPCODE_JNB:    { should_jump = false; } break;
                case OPCODE_JNBE:   { should_jump = false; } break;
                case OPCODE_JNP:    { should_jump = false; } break;
                case OPCODE_JNO:    { should_jump = false; } break;
                case OPCODE_JNS:    { should_jump = false; } break;
                case OPCODE_LOOP:   { should_jump = ((s16)--state->registers[2].value > 0); } break;
                case OPCODE_LOOPZ:  { should_jump = false; } break;
                case OPCODE_LOOPNZ: { should_jump = !state->zero_flag; } break;
                case OPCODE_JCXZ:   { should_jump = false; } break;
            }
            
            if (should_jump) {
                if (instruction->binary == OPCODE_LOOPNZ) {
                    if (--state->registers[2].value == 0) {
                        break;
                    }
                }
                
                state->ip_register.value += (s8)instruction->value;
                instructions_to_move = 0;
                
                if ((s8)instruction->value < 0) {
                    s8 bytes_remaining = -instruction->bytes_used - (s8)instruction->value;
                    
                    while (bytes_remaining) {
                        --instruction;
                        --instructions_to_move;
                        bytes_remaining -= instruction->bytes_used;
                    }
                } else {
                    s8 bytes_remaining = instruction->bytes_used + (s8)instruction->value;
                    
                    while (bytes_remaining) {
                        bytes_remaining -= instruction->bytes_used;
                        ++instruction;
                        ++instructions_to_move;
                    }
                }
            }
        } break;
        
    }
    
    return instructions_to_move;
}

void simulate_asm_8086(Instruction *instructions, u32 count)
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
    state.registers[8] = {Register_es, 0};
    state.registers[9] = {Register_cs, 0};
    state.registers[10] = {Register_ss, 0};
    state.registers[11] = {Register_ds, 0};
    state.ip_register = {Register_ip, 0};
    state.memory = (u8 *)malloc(MEGABYTES(1));
    
    for (int instruction_index = 0; instruction_index < count;) {
        Instruction *instruction = instructions + instruction_index;
        s8 instructions_to_move = simulate_instruction(&state, instruction);
        instruction_index += instructions_to_move;
    }
    
    print_instructions(instructions, count);
    
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
            simulate_asm_8086(instructions, count);
        } else {
            print_instructions(instructions, count);
        }
    }
    else
    {
        printf("ERROR: could not read file %s\n", filename);
    }
    
    return 0;
}

