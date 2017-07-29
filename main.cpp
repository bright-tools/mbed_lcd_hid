#include "mbed.h"
#include "freetronicsLCDShield.h"
#include "USBHID.h"
#include "TSISensor.h"
#include "time_help.hpp"
#include "lcd.hpp"

#if defined SERIAL_DEBUG
Serial pc(USBTX, USBRX); // tx, rx
#endif


USBHID hid(MAX_ROW_LEN + 2, MAX_ROW_LEN + 2, 0x1987, 0x1100);

HID_REPORT recv_report;
HID_REPORT send_report;

freetronicsLCDShield lx(D8, D9, 
                        D4, D5, D6, D7, 
                        D10, A0); 

LCDIf lcdInterface( lx );

Ticker lcdUpdate;
TSISensor   tsi;

Ticker buttonWatcher;
freetronicsLCDShield::ShieldButton lastButton = freetronicsLCDShield::None;

void watchButtons( void )
{
    freetronicsLCDShield::ShieldButton currentButton = lx.pressedButton();
   
    if( tsi.readPercentage() )
    {
        lcdInterface.stopPulsing();
    }

    if( lastButton == freetronicsLCDShield::None )
    {
        if( currentButton != freetronicsLCDShield::None )
        {
            lcdInterface.resetSleepTimer();
        }

        switch( currentButton ) {
            case freetronicsLCDShield::Up:
                lcdInterface.incrementBackLight();
                break;
            case freetronicsLCDShield::Down:
                lcdInterface.decrementBackLight();
                break;
            default:
                break;
        }
    }

    lastButton = currentButton;
}

void lcdRefresh( void )
{
    lcdInterface.refresh();
}

int main() {
#if defined SERIAL_DEBUG
    pc.printf("Hello");
#endif

    EventQueue eventQueue;
    Thread lcdThread(osPriorityLow);
    lcdThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));

    lcdUpdate.attach( eventQueue.event(&lcdRefresh), LCD_REFRESH_RATE );
    buttonWatcher.attach( watchButtons, BUTTON_WATCH_RATE );

    lcdInterface.setString( 0, "* Hello * This is a very long str" );

    while(true) {
#if defined SERIAL_DEBUG
        pc.printf("+");
#endif

        send_report.length = MAX_ROW_LEN + 2;
        //Send the report
        hid.send(&send_report);

        //Fill the report
        for (unsigned i = 0; i < send_report.length; i++)
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
            lcdInterface.setString(destRow,(char*)(&(recv_report.data[1])));
        }

#if defined SERIAL_DEBUG
        pc.printf("Button State: %f %d\r\n",lx.readButton(),lx.pressedButton());
#endif
        wait(1);
    }
}
