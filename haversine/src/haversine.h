#ifndef HAVERSINE_H
#define HAVERSINE_H


#define F32_MIN -FLT_MAX
#define F32_MAX FLT_MAX

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
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

struct Tokenizer {
    char *at;
    u32 line;
};

enum Token_type {
    TOKEN_TYPE_UNKNOWN,
    
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
};

struct Token {
    Token_type type;
    
    u32 text_length;
    char *text;
    
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

// TODO: here access the distinct value types: object, array, string, number, boolean and null

struct Json_object;
struct Json_object_member;
struct Json_value;


struct Json_object_member {
    char *name;
    Json_value *value;
};

struct Json_object {
    u32 member_count;
    Json_object_member *members;
};

struct Json_value {
    Json_value_type type;
    union {
        Json_object *object;
        void *array;
        void *string;
        void *number;
        void *boolean;
        void *null;
    };
};

Token get_token(Tokenizer *tokenizer, bool advance_tokenizer = true);
bool require_token(Tokenizer *tokenizer, Token_type expected_type);
void parse_json(char *json_content);
void parse_elements(Tokenizer *tokenizer);
bool parse_element(Tokenizer *tokenizer);
void parse_object(Tokenizer *tokenizer);
void parse_array(Tokenizer *tokenizer);
void parse_members(Tokenizer *tokenizer);
void parse_member(Tokenizer *tokenizer);

#endif //HAVERSINE_H
