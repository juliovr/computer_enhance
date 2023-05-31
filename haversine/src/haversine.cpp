#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <intrin.h>
#include <float.h>
#include <stdint.h>

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

f32 random_value(f32 min, f32 max)
{
    u64 clocks = __rdtsc();
    u64 r = ((u64)rand() ^ ((u32)max));
    f32 random = 1000000000*(min + ((max - min) * ((f32)r / (f32)clocks)));
    
    return random;
}

struct Pair {
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
};

void generate_json(u32 n)
{
    FILE *file = fopen("haversine.json", "w");
    if (file)
    {
        srand((unsigned int)time(NULL));
        
        fprintf(file, "{\n");
        fprintf(file, "    \"pairs\": [\n");
        for (u32 i = 0; i < n; ++i) {
            Pair pair = {};
            pair.x0 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            pair.y0 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            pair.x1 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            pair.y1 = random_value(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
            
            fprintf(file, "        { \"x0\": %.20f, \"y0\": %.20f, \"x1\": %.20f, \"y1\": %.20f }", pair.x0, pair.y0, pair.x1, pair.y1);
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

#if 0
void parse_json()
{
    FILE *file = fopen("haversine.json", "r");
    if (file)
    {
        fprintf(stdout, "loaded\n");
        
        fclose(file);
    }
}
#endif

int main(int argc, char** argv)
{
    
#if 1
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s [number of entries to generate]\n", argv[0]);
        return 1;
    }
    
    u32 n = atoi(argv[1]);
    generate_json(n);
#endif
    
    //parse_json();
    
    return 0;
}
