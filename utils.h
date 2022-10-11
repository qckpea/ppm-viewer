#include <stdint.h>
#include <assert.h>

#define uint8 uint8_t
#define uint16 uint16_t 
#define uint32 uint32_t 
#define uint64 uint64_t 
#define int8 int8_t 
#define int16 int16_t 
#define int32 int32_t 
#define int64 int64_t 

inline uint32
SafeTruncateUInt64(uint64 value)
{
    assert(value <= 0xFFFFFFFF);
    uint32 result = (uint32)value;
    return(result);
}