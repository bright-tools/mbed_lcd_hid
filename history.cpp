#include "history.hpp"

#include <list>

#define MAX_HISTORY_SIZE 10U

#if defined SERIAL_DEBUG
extern Serial pc;
#endif

std::list<DisplayMessage_t> messageHistory;

bool removeMessage( const uint32_t p_id )
{
    bool retVal = false;

    for( std::list<DisplayMessage_t>::iterator i = messageHistory.begin();
         i != messageHistory.end();
         i++ )
    {
        if( i->id == p_id )
        {
            messageHistory.erase( i );
            retVal = true;
            break;
        }
    }

    return retVal;
}

void addMessage( const DisplayMessage_t* const p_message )
{
    /* If the message is already in the history, remove it */
    removeMessage( p_message->id );

    /* Add the message to the back of the history list */
    messageHistory.push_back( *p_message );

    /* Prevent the history from growing too long */
    if( messageHistory.size() > MAX_HISTORY_SIZE )
    {
        messageHistory.pop_front();
    }
#if defined SERIAL_DEBUG
    pc.printf("Message history is size: %d\r\n",messageHistory.size());
#endif
}

DisplayMessage_t* getMessage( uint32_t p_id, Offset_t p_offset )
{
    DisplayMessage_t* retVal = NULL;

    if( p_id == UINT32_MAX )
    {
        if( messageHistory.size() > 0 )
        {
            retVal = &(*(messageHistory.begin()));
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        for( std::list<DisplayMessage_t>::iterator i = messageHistory.begin();
             i != messageHistory.end();
             i++ )
        {
            if( p_id == i->id )
            {
                switch( p_offset )
                {
                    case OFFSET_NONE:
                        retVal = &(*i);
                        break;
                    case OFFSET_BEFORE:
                        if( i != messageHistory.begin() )
                        {
                            i--;
                            retVal = &(*i);
                        }
                        break;
                    case OFFSET_AFTER:
                        i++;
                        if( i != messageHistory.end() )
                        {
                            retVal = &(*i);
                        }
                        break;
                }
                break;
            }
        }
    }

#if defined SERIAL_DEBUG
    if( retVal != NULL )
    {
        pc.printf("Returning %d\r\n",retVal->id);
    }
    else
    {
        pc.printf("Returning NULL\r\n");
    }
#endif

    return retVal;
}
