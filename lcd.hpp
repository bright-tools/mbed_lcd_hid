#if !defined LCD_HPP
#define      LCD_HPP

#include "mbed.h"
#include "freetronicsLCDShield.h"

#include <cstddef>

#define MAX_ROW_DISPLAY 16
#define MAX_ROW_COUNT    2
#define MAX_ROW_LEN 40
#define SCROLL_PAUSE 10
#define RESET_PAUSE 10

class LCDIf
{
    public:
        typedef enum
        {
            ScrollEffectDefault,
            ScrollEffectBackToStart,
            ScrollEffectWrap,
            ScrollEffectWrapWithSpace,
            ScrollEffectReverseAtEnd
        } ScrollEffect_t;
    protected:
        bool newData;
        unsigned lcdPos;
        unsigned resetPauseCtr;
        unsigned scrollPauseCtr;
        uint32_t sleepTimer;
        size_t maxDataLen;
        bool pulsing;
        bool forwardDirectionScroll;
        unsigned pulseVal;
        float currentBacklight;
        /* Length is +1 to allow for terminator character */
        char lcdData[2][MAX_ROW_LEN+1];
        ScrollEffect_t currentScrollEffect;
        freetronicsLCDShield& m_shield;

        float calcPulseBackLight( void );
        void setBackLight( void );

        bool DoScrollWrap( void );
        bool DoScrollBackToStart( void );
        bool DoScrollReverseAtEnd( void );
        void doShift( void );
    public:

        LCDIf( freetronicsLCDShield& p_shield );
        virtual ~LCDIf();

        void resetSleepTimer( void );
        void decrementSleepTimer( void );

        void incrementBackLight( void );
        void decrementBackLight( void );

        void setString( unsigned p_row, const char* const p_data );

        void stopPulsing( void );

        void refresh( void );
};

#endif
