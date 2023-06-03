#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <intrin.h>
#include <float.h>
#include <stdint.h>
#include <string.h>

#include "haversine.h"

//
// See: https://www.json.org/json-en.html
//

f32 random_value(f32 min, f32 max)
{
    u64 clocks = __rdtsc();
    u64 r = ((u64)rand() ^ ((u32)max));
    f32 random = 1000000000*(min + ((max - min) * ((f32)r / (f32)clocks)));
    
    return random;
}

void generate_json(u32 n)
{
    FILE *file = fopen("haversine.json", "w");
    if (file)
    {
        srand((unsigned int)time(NULL));
        
        fprintf(file, "{\n");
        fprintf(file, "    \"pairs\": [\n");
        for (u32 i = 0; i < n; ++i) {
            f32 x0 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            f32 y0 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            f32 x1 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            f32 y1 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            
            fprintf(file, "        { \"x0\": %.20f, \"y0\": %.20f, \"x1\": %.20f, \"y1\": %.20f }", x0, y0, x1, y1);
            if (i < (n - 1)) {
                fprintf(file, ",");
            }
            
            fprintf(file, "\n");
        }
        
        fprintf(file, "    ]\n");
        fprintf(file, "}");
        
        fclose(file);
    }
}

char * read_entire_file(char *filename)
{
    char *result = 0;
    
    FILE *file = fopen(filename, "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result = (char *)malloc(file_size + 1);
        fread(result, file_size, 1, file);
        result[file_size] = '\0';
        
        fclose(file);
    }
    
    return result;
}

inline bool is_end_of_line(char c)
{
    bool result = (c == '\n' ||
                   c == '\r');
    
    return result;
}

inline bool is_whitespace(char c)
{
    bool result = (c == ' ' ||
                   c == '\t' ||
                   c == '\f' ||
                   is_end_of_line(c));
    
    return result;
}

inline void eat_all_whitespaces(Tokenizer *tokenizer)
{
    while (is_whitespace(tokenizer->at[0])) {
        if (is_end_of_line(tokenizer->at[0])) {
            ++tokenizer->line;
        }
        
        ++tokenizer->at;
    }
}

inline bool is_alpha(char c)
{
    bool result = ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
    
    return result;
}

inline bool is_number(char c)
{
    bool result = (c >= '0' && c <= '9');
    
    return result;
}

Token get_token(Tokenizer *tokenizer, bool advance_tokenizer)
{
    eat_all_whitespaces(tokenizer);
    
    char *original_at = tokenizer->at;
    
    Token token = {};
    token.text_length = 1;
    token.text = tokenizer->at;
    
    char c = tokenizer->at[0];
    ++tokenizer->at;
    
    switch(c)
    {
        case '{': { token.type = TOKEN_TYPE_OPEN_BRACE; } break;
        case '}': { token.type = TOKEN_TYPE_CLOSE_BRACE; } break;
        case '[': { token.type = TOKEN_TYPE_OPEN_BRACKET; } break;
        case ']': { token.type = TOKEN_TYPE_CLOSE_BRACKET; } break;
        case ':': { token.type = TOKEN_TYPE_COLON; } break;
        case ',': { token.type = TOKEN_TYPE_COMMA; } break;
        
        case '"': {
            token.type = TOKEN_TYPE_STRING;
            
            token.text = tokenizer->at;
            while (tokenizer->at[0] && tokenizer->at[0] != '"') {
                ++tokenizer->at;
            }
            
            token.text_length = (u32)(tokenizer->at - token.text);
            
            ++tokenizer->at; // Skip last double quotes
        } break;
        
        case '\0': { token.type = TOKEN_TYPE_END_OF_STREAM; } break;
        
        default: {
            // Parse literal value
            
            if (is_alpha(c)) {
                token.text = --tokenizer->at;
                while (tokenizer->at[0] && (is_alpha(c) &&
                                            tokenizer->at[0] != '}' && 
                                            tokenizer->at[0] != ']' && 
                                            tokenizer->at[0] != ',' && 
                                            !is_whitespace(tokenizer->at[0]))) {
                    
                    ++tokenizer->at;
                }
                
                token.text_length = (u32)(tokenizer->at - token.text);
                
                if (strncmp(token.text, "true", 4) == 0 ||
                    strncmp(token.text, "false", 5) == 0) {
                    token.type = TOKEN_TYPE_BOOLEAN;
                } else if (strncmp(token.text, "null", 4) == 0) {
                    token.type = TOKEN_TYPE_NULL;
                } else {
                    fprintf(stderr, "Unrecognized literal value %.*s\n", token.text_length, token.text);
                }
                
            } else if (is_number(c)) {
                token.text = --tokenizer->at;
                
                // TODO: Fix this. Now it works with correct decimal values, but it does not work with values like 1.e1
                bool could_be_number = true;
                bool could_be_dot = true;
                bool could_be_e = true;
                
                while (tokenizer->at[0])
                {
                    if (tokenizer->at[0] != '}' && 
                        tokenizer->at[0] != ']' && 
                        tokenizer->at[0] != ',' && 
                        !is_whitespace(tokenizer->at[0]))
                    {
                        if (could_be_number && is_number(tokenizer->at[0])) {
                            ++tokenizer->at;
                            if (!could_be_dot) {
                                could_be_e = true;
                            }
                            if (!could_be_e) {
                                could_be_dot = true;
                            }
                        } else if (could_be_dot && tokenizer->at[0] == '.') {
                            ++tokenizer->at;
                            could_be_dot = false;
                        } else if (could_be_e && ((tokenizer->at[0] == 'e') ||
                                                  (tokenizer->at[0] == 'E'))) {
                            ++tokenizer->at;
                            could_be_dot = false;
                            could_be_e = false;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                
                token.text_length = (u32)(tokenizer->at - token.text);
                token.type = TOKEN_TYPE_NUMBER;
                
            } else {
                fprintf(stderr, "Unrecognized literal value %.*s\n", token.text_length, token.text);
            }
            
        } break;
    }
    
    
    if (!advance_tokenizer) {
        tokenizer->at = original_at;
    }
    
    return token;
}

inline bool require_token(Tokenizer *tokenizer, Token_type expected_type)
{
    Token token = get_token(tokenizer);
    bool result = token.type == expected_type;
    
    return result;
}

void parse_member(Tokenizer *tokenizer)
{
    Token string_token = get_token(tokenizer);
    if (string_token.type != TOKEN_TYPE_STRING) {
        fprintf(stderr, "Missing '\"' at line %d\n", tokenizer->line);
        return;
    }
    
    if (!require_token(tokenizer, TOKEN_TYPE_COLON)) {
        fprintf(stderr, "Missing ':' at line %d\n", tokenizer->line);
        return;
    }
    
    parse_element(tokenizer);
}

void parse_members(Tokenizer *tokenizer)
{
    parse_member(tokenizer);
    
    Token token = get_token(tokenizer, false);
    if (token.type == TOKEN_TYPE_COMMA) {
        get_token(tokenizer);
        parse_members(tokenizer);
    }
}

void parse_object(Tokenizer *tokenizer)
{
    parse_members(tokenizer);
    
    if (!require_token(tokenizer, TOKEN_TYPE_CLOSE_BRACE)) {
        fprintf(stderr, "Expected } at line %d\n", tokenizer->line);
        return;
    }
}

void parse_array(Tokenizer *tokenizer)
{
    Token array = get_token(tokenizer, false);
    
    parse_elements(tokenizer);
    
    if (!require_token(tokenizer, TOKEN_TYPE_CLOSE_BRACKET)) {
        fprintf(stderr, "Expected ] at line %d\n", tokenizer->line);
        return;
    }
}

void parse_elements(Tokenizer *tokenizer)
{
    parse_element(tokenizer);
    
    Token token = get_token(tokenizer, false);
    if (token.type == TOKEN_TYPE_COMMA) {
        get_token(tokenizer);
        parse_elements(tokenizer);
    }
}

bool parse_element(Tokenizer *tokenizer)
{
    bool continue_parsing = true;
    
    Token token = get_token(tokenizer);
    switch(token.type)
    {
        case TOKEN_TYPE_OPEN_BRACE: {
            parse_object(tokenizer);
        } break;
        
        case TOKEN_TYPE_OPEN_BRACKET: {
            parse_array(tokenizer);
        } break;
        
        // Literal values string, number, boolean and null are skiped here
        
        case TOKEN_TYPE_END_OF_STREAM: {
            continue_parsing = false;
        } break;
    }
    
    return continue_parsing;
}

void parse_json(char *json_content)
{
    Tokenizer tokenizer = {};
    tokenizer.at = json_content;
    tokenizer.line = 1;
    
    bool parsing = true;
    while (parsing) {
        parsing = parse_element(&tokenizer);
    }
}

int main(int argc, char** argv)
{
#if 0
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s [number of entries to generate]\n", argv[0]);
        return 1;
    }
    
    u32 n = atoi(argv[1]);
    generate_json(n);
#endif
    
    //char *filename = "haversine.json";
    char *filename = "test.json";
    char *json_content = read_entire_file(filename);
    if (json_content) {
        printf("Length = %zd\n", strlen(json_content));
        printf("Parsing %s...\n", filename);
        parse_json(json_content);
        printf("Done\n");
    } else {
        fprintf(stderr, "ERROR: Could not open file %s\n", filename);
    }
    
    return 0;
}
