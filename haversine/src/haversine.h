#ifndef HAVERSINE_H
#define HAVERSINE_H


#define F32_MIN -FLT_MAX
#define F32_MAX FLT_MAX

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef char  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define MIN_RANDOM_VALUE 0.0f
#define MAX_RANDOM_VALUE 100.0f

#define MAX_RAND 24568.0f

//
// Lexer
//

struct Token;

struct Tokenizer {
    char *at;
    u32 line;

    bool parsing;

    Token *first;
    Token *last;
};

#define FOREACH_TOKEN_TYPE(GENERATION_TYPE)     \
    GENERATION_TYPE(TOKEN_TYPE_UNKNOWN)         \
    GENERATION_TYPE(TOKEN_TYPE_OPEN_BRACKET)    \
    GENERATION_TYPE(TOKEN_TYPE_CLOSE_BRACKET)   \
    GENERATION_TYPE(TOKEN_TYPE_OPEN_BRACE)      \
    GENERATION_TYPE(TOKEN_TYPE_CLOSE_BRACE)     \
    GENERATION_TYPE(TOKEN_TYPE_COLON)           \
    GENERATION_TYPE(TOKEN_TYPE_COMMA)           \
    GENERATION_TYPE(TOKEN_TYPE_STRING)          \
    GENERATION_TYPE(TOKEN_TYPE_NUMBER)          \
    GENERATION_TYPE(TOKEN_TYPE_BOOLEAN)         \
    GENERATION_TYPE(TOKEN_TYPE_NULL)            \
    GENERATION_TYPE(TOKEN_TYPE_END_OF_STREAM)

#define GENERATE_ENUM(VALUE) VALUE,
#define GENERATE_STRING(VALUE) #VALUE,

enum Token_type {
/*    TOKEN_TYPE_UNKNOWN,
    
    TOKEN_TYPE_OPEN_BRACKET,
    TOKEN_TYPE_CLOSE_BRACKET,
    TOKEN_TYPE_OPEN_BRACE,
    TOKEN_TYPE_CLOSE_BRACE,
    TOKEN_TYPE_COLON,
    TOKEN_TYPE_COMMA,
    //TOKEN_TYPE_VALUE,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_BOOLEAN,
    TOKEN_TYPE_NULL,
    
    TOKEN_TYPE_END_OF_STREAM,
*/
    FOREACH_TOKEN_TYPE(GENERATE_ENUM)
};

const static char * token_types[] {
    FOREACH_TOKEN_TYPE(GENERATE_STRING)
};


struct Buffer {
    int size;
    u8 *data;
};

struct Token {
    Token_type type;

    Buffer buffer;
    
    Token *next;
};


//
// Parser
//

enum Json_value_type {
    JSON_VALUE_TYPE_OBJECT,
    JSON_VALUE_TYPE_ARRAY,
    JSON_VALUE_TYPE_STRING,
    JSON_VALUE_TYPE_NUMBER,
    JSON_VALUE_TYPE_BOOLEAN,
    JSON_VALUE_TYPE_NULL,
};


struct Json_element {
    Buffer name;
    Buffer value;

    Json_element *first;
    Json_element *next_sibling;
};

Json_element * parse_element(Tokenizer *tokenizer, Buffer name, Token token_value);
Json_element * parse_object(Tokenizer *tokenizer);
Json_element * parse_array(Tokenizer *tokenizer);

inline void add_token(Tokenizer *tokenizer, Token *token)
{
    Token *new_token = (Token *)malloc(sizeof(Token));
    *new_token = *token;
    
    if (!tokenizer->first) {
        tokenizer->first = new_token;
        tokenizer->last = new_token;
        
        tokenizer->first->next = tokenizer->last;
    } else {
        tokenizer->last->next = new_token;
        tokenizer->last = new_token;
    }
}

inline Json_element * get(Json_element *json, char *name)
{
    Json_element *element = json->first;
    while (element) {
        if (strncmp(element->name.data, name, element->name.size) == 0) {
            break;
        }

        element = element->next_sibling;
    }

    return element;
}

#endif //HAVERSINE_H
