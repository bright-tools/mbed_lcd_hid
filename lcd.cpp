#include "lcd.hpp"
#include "time_help.hpp"

LCDIf::LCDIf( freetronicsLCDShield& p_shield ) : newData( false ), lcdPos( 0 ), 
                                                 resetPauseCtr( 0 ),
                                                 scrollPauseCtr( 0 ), sleepTimer( 0 ),
                                                 maxDataLen( 0 ), pulsing( false ),
                                                 forwardDirectionScroll( true ),
                                                 pulseVal( 0 ), currentBacklight( 0.2 ),
                                                 currentScrollEffect( ScrollEffectReverseAtEnd ),
                                                 m_shield( p_shield )
{
    memset( lcdData, 0, sizeof( lcdData ));

    m_shield.setBackLight(currentBacklight);
    m_shield.setCursor(false);
}

LCDIf::~LCDIf( void )
{
}

#define SLEEP_TIME       SECONDS_IN_MINUTES( 5 )

void LCDIf::resetSleepTimer( void )
{
    sleepTimer = SLEEP_TIME / LCD_REFRESH_RATE;
}

void LCDIf::decrementSleepTimer( void )
{
    sleepTimer--;
}

bool LCDIf::DoScrollWrap( void )
{
    bool doScroll = false;

    /* Is it a short string and we've not got to the position where the
     * display is wrapping back round? */
    if(( lcdPos > maxDataLen) && ( lcdPos < (MAX_ROW_LEN-MAX_ROW_DISPLAY)))
    {
        const unsigned shifts = (MAX_ROW_LEN-MAX_ROW_DISPLAY+1) - lcdPos;
        for( unsigned i = 0; i < shifts; i++ )
        {
            doShift();
        }
    }
    else
    {
        doScroll = true;
    }

    return doScroll;
}

bool LCDIf::DoScrollBackToStart( void )
{
    bool doScroll = false;

    if( lcdPos >= (maxDataLen-MAX_ROW_DISPLAY))
    {
        if( resetPauseCtr < RESET_PAUSE )
        {
            resetPauseCtr++;
        } 
        else
        {
            /* Return home so that the user isn't waiting for the display to
             * scroll through a load of empty space */
            m_shield.home();
            lcdPos = 0;
            scrollPauseCtr = 0;
            resetPauseCtr = 0;
        }
    }
    else
    {
        doScroll = true;
    }

    return doScroll;
}

bool LCDIf::DoScrollReverseAtEnd( void )
{
    bool doScroll = false;

    if( forwardDirectionScroll )
    {
        if( lcdPos >= (maxDataLen-MAX_ROW_DISPLAY))
        {
            forwardDirectionScroll = false;
        }
        else
        {
            doScroll = true;
        }
    }
    else
    {
        m_shield.shift(false);
        lcdPos--;
        if( lcdPos == 0 )
        {
            forwardDirectionScroll = true;
        }
    }

    return doScroll;
}

void LCDIf::refresh( void )
{
    if( newData )
    {
        size_t i;
        
        if( currentScrollEffect == ScrollEffectWrapWithSpace )
        {
            lcdData[0][MAX_ROW_LEN-1] = lcdData[1][MAX_ROW_LEN-1] = ' ';
        }

        lcdPos = 0;
        scrollPauseCtr = 0;
        m_shield.cls();
        m_shield.setCursorPosition(0,0);
        m_shield.printf(lcdData[0]);
        maxDataLen = strnlen( lcdData[0], MAX_ROW_LEN );
        m_shield.setCursorPosition(1,0);
        m_shield.printf(lcdData[1]);
        i = strnlen( lcdData[1], MAX_ROW_LEN );

        if( i > maxDataLen )
        {
            maxDataLen = i;
        }
        newData = false;
        forwardDirectionScroll = true;
        resetSleepTimer();
    }
    /* Is it a string that needs to be scrolled? */
    else if( maxDataLen > MAX_ROW_DISPLAY )
    {
        if( scrollPauseCtr < SCROLL_PAUSE )
        {
            scrollPauseCtr++;
        } 
        else 
        {
            bool doScroll = false;
            switch( currentScrollEffect )
            {
                case ScrollEffectDefault:
                case ScrollEffectBackToStart:
                    doScroll = DoScrollBackToStart();
                    break;
                case ScrollEffectWrapWithSpace:
                case ScrollEffectWrap:
                    doScroll = DoScrollWrap();
                    break;
                case ScrollEffectReverseAtEnd:
                    doScroll = DoScrollReverseAtEnd();
                    break;
            }

            if( doScroll )
            {
                doShift();
            }
        }
    }

    setBackLight();
}

void LCDIf::doShift( void )
{
    m_shield.shift(true);
    lcdPos = (lcdPos + 1) % MAX_ROW_LEN;
}

float LCDIf::calcPulseBackLight( void )
{
    float retVal = 0.8* sin(pulseVal * 3.14159265 / 180);
    if( retVal < 0 )
    {
        retVal = -retVal;
    }
    if( retVal < 0.05 )
    {
        retVal = 0.05;
    }
    pulseVal = (pulseVal + 5) % 360;

    return retVal;
}

void LCDIf::setBackLight( void )
{
    float backLightVal;
    backLightVal = currentBacklight; 
    if( pulsing )
    {
        backLightVal = calcPulseBackLight();
    }
    else
    {
        pulseVal = 0;

        if( sleepTimer > 0 )
        {
            decrementSleepTimer();
        }
        else
        {
            backLightVal = 0;
        }
    }

    m_shield.setBackLight(backLightVal);
}

#define BACKLIGHT_STEP_SIZE 0.1F

void LCDIf::incrementBackLight( void )
{
    if( currentBacklight < 1.0 )
    {
        currentBacklight += BACKLIGHT_STEP_SIZE;
    }
}

void LCDIf::decrementBackLight( void )
{
    if( currentBacklight > BACKLIGHT_STEP_SIZE )
    {
        currentBacklight -= BACKLIGHT_STEP_SIZE;
    }
    else
    {
        currentBacklight = 0;
    }
}

void LCDIf::setPulsing( const bool p_pulse )
{
    pulsing = p_pulse;
}
        
void LCDIf::setString( unsigned p_row, const char* const p_data )
{
    strncpy(lcdData[p_row],p_data,MAX_ROW_LEN);
    newData = true;
}
