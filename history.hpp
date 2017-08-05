#if !defined HISTORY_HPP
#define      HISTORY_HPP

#include "lcd.hpp"

#include <limits>
#include <stdint.h>
#include <string>

typedef enum
{
    OFFSET_NONE,
    OFFSET_BEFORE,
    OFFSET_AFTER
} Offset_t;

typedef struct
{
    uint32_t    id;
    std::string lines[MAX_ROW_COUNT];
} DisplayMessage_t;

void              addMessage( const DisplayMessage_t* const p_message );
DisplayMessage_t* getMessage( uint32_t p_id, Offset_t p_offset );
bool              removeMessage( const uint32_t p_id );

#endif
