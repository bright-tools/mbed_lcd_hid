#include "mbed.h"
#include "freetronicsLCDShield.h"
#include "USBHID.h"

#if defined SERIAL_DEBUG
Serial pc(USBTX, USBRX); // tx, rx
#endif

#define MAX_ROW_DISPLAY 16
#define MAX_ROW_LEN 40
#define SCROLL_PAUSE 10

USBHID hid(MAX_ROW_LEN + 2, MAX_ROW_LEN + 2, 0x1987, 0x1100);

HID_REPORT recv_report;
HID_REPORT send_report;

freetronicsLCDShield lx(D8, D9, 
                        D4, D5, D6, D7, 
                        D10, A0); 

Ticker lcdUpdate;
char lcdData[2][MAX_ROW_LEN+1] = { { 0 }, { 0 } };
size_t maxDataLen;
bool newData = false;
unsigned lcdPos = 0;
bool pulsing = false;
unsigned scrollPauseCtr = 0;

void lcdRefresh( void )
{
    if( newData )
    {
        size_t i;

        lcdPos = 0;
        scrollPauseCtr = 0;
        lx.cls();
        lx.setCursorPosition(0,0);
        lx.printf(lcdData[0]);
        maxDataLen = strnlen( lcdData[0], MAX_ROW_LEN );
        lx.setCursorPosition(1,0);
        lx.printf(lcdData[1]);
        i = strnlen( lcdData[1], MAX_ROW_LEN );
        if( i > maxDataLen )
        {
            maxDataLen = i;
        }
        newData = false;
        pulsing = true;
    }
    else if( maxDataLen > MAX_ROW_DISPLAY )
    {
        if( scrollPauseCtr < SCROLL_PAUSE )
        {
            scrollPauseCtr++;
        } 
        else 
        {
            lx.shift(true);
            lcdPos = (lcdPos + 1) % MAX_ROW_LEN;

            /* Is it a short string and we've not got to the position where the
             * display is wrapping back round? */
            if(( lcdPos > maxDataLen) && ( lcdPos < (MAX_ROW_LEN-MAX_ROW_DISPLAY)))
            {
                /* Return home so that the user isn't waiting for the display to
                 * scroll through a load of empty space */
                lx.home();
                lcdPos = 0;
                scrollPauseCtr = 0;
            }
        }
    }
}

Ticker buttonWatcher;
freetronicsLCDShield::ShieldButton lastButton = freetronicsLCDShield::None;
float currentBacklight = 0.2;
unsigned pulseVal = 0;
void watchButtons( void )
{
    freetronicsLCDShield::ShieldButton currentButton = lx.pressedButton();
    float backLightVal;
    
    if( lastButton == freetronicsLCDShield::None )
    {
        switch( currentButton ) {
        case freetronicsLCDShield::Up:
            if( currentBacklight < 1.0 )
            {
                currentBacklight += 0.1;
            }
            break;
        case freetronicsLCDShield::Down:
            if( currentBacklight > 0.0 )
            {
                currentBacklight -= 0.1;
            }
            break;
        case freetronicsLCDShield::Left:
            pulsing = false;
            break;
        }
    }
    
    backLightVal = currentBacklight; 
    if( pulsing )
    {
        backLightVal = 0.8* sin(pulseVal * 3.14159265 / 180);
        if( backLightVal < 0 )
        {
            backLightVal = -backLightVal;
        }
        if( backLightVal < 0.05 )
        {
            backLightVal = 0.05;
        }
        pulseVal = (pulseVal + 5) % 360;
    }
    else
    {
        pulseVal = 0;
    }

    lx.setBackLight(backLightVal);
    lastButton = currentButton;
}

int main() {
#if defined SERIAL_DEBUG
    pc.printf("Hello");
#endif

    lcdUpdate.attach( lcdRefresh, 0.2 );
    buttonWatcher.attach( watchButtons, 0.1 );

    lx.setBackLight(currentBacklight);
    lx.setCursor(false);

    memcpy(lcdData[0],"* Hello * This is a very long str"/*ing for"*/,MAX_ROW_LEN);
    newData = true;

    while(1) {
#if defined SERIAL_DEBUG
        pc.printf("+");
#endif
        wait(1);

    send_report.length = MAX_ROW_LEN + 2;
    //Send the report
    hid.send(&send_report);

    //Fill the report
    for (int i = 0; i < send_report.length; i++)
    {
        send_report.data[i] = rand() & 0xff;
    }

    //try to read a msg
    while(hid.readNB(&recv_report)) {
        unsigned destRow = 0;
        if( recv_report.data[0] == 1 )
        {
            destRow = 1;
        }
#if defined SERIAL_DEBUG
        pc.printf("Message received for %d\r\n",destRow);
#endif
        memcpy(lcdData[destRow],&(recv_report.data[1]),MAX_ROW_LEN);
        lcdData[destRow][MAX_ROW_LEN] = 0;
        newData = true;
    }

#if defined SERIAL_DEBUG
        pc.printf("Button State: %f %d\r\n",lx.readButton(),lx.pressedButton());
#endif
    }
}
