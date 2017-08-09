#include "time_help.hpp"
#include "lcd.hpp"
#include "history.hpp"

#include "mbed.h"
#include "freetronicsLCDShield.h"
#include "USBHID.h"
#include "TSISensor.h"
#include "RoundRobin.hpp"

#if defined SERIAL_DEBUG
Serial pc(USBTX, USBRX); // tx, rx
#endif

#define HID_SEND_REPORT_LEN 64U
#define HID_RECV_REPORT_LEN HID_SEND_REPORT_LEN

USBHID hid(HID_SEND_REPORT_LEN, HID_RECV_REPORT_LEN, 0x1987U, 0x1100U);

freetronicsLCDShield lx(D8, D9, 
                        D4, D5, D6, D7, 
                        D10, A0); 

LCDIf lcdInterface( lx );

TSISensor   tsi;

Ticker buttonWatcher;
freetronicsLCDShield::ShieldButton lastButton = freetronicsLCDShield::None;

uint32_t currentMessageId = UINT32_MAX;

void displayMessage( const DisplayMessage_t* const p_msg )
{
    currentMessageId = p_msg->id;

    for( unsigned i = 0; i < MAX_ROW_COUNT; i++ )
    {
        lcdInterface.setString(i ,p_msg->lines[i].c_str());
    }

    lcdInterface.setScrollEffect( p_msg->scrollEffect );
    lcdInterface.setPulsing( !( p_msg->dismissed ) );
}

bool showMessageFromHistory( const Offset_t p_offset )
{
    const DisplayMessage_t* const msg = getMessage( currentMessageId, p_offset );
    bool retVal = false;
    if( msg != NULL )
    {
        displayMessage( msg );
        retVal = true;
    }
    return retVal;
}

void watchButtons( void )
{
    freetronicsLCDShield::ShieldButton currentButton = lx.pressedButton();
   
    if( tsi.readPercentage() )
    {
        DisplayMessage_t* msg = getMessage( currentMessageId, OFFSET_NONE );
        if( msg != NULL )
        {
            msg->dismissed = true;
        }
        lcdInterface.setPulsing(false);
        lcdInterface.resetSleepTimer();
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
            case freetronicsLCDShield::Right:
                showMessageFromHistory( OFFSET_AFTER );
                break;
            case freetronicsLCDShield::Left:
                showMessageFromHistory( OFFSET_BEFORE );
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

void showBanner( void )
{
    lcdInterface.setString( 0, "mbed LCD HID" );
    lcdInterface.setString( 1, "github.com/bright-tools/mbed_lcd_hid" );
    lcdInterface.setPulsing( false );
}

#define MESSAGE_ID_NEW_MESSAGE 0x10U
#define MESSAGE_ID_REMOVE_MESSAGE 0x20U

void handleRemoveMessage( const uint8_t* const p_data )
{
    uint32_t id = (uint32_t)p_data[0] |
                  (uint32_t)p_data[1] |
                  (uint32_t)p_data[2] |
                  (uint32_t)p_data[3];

#if defined SERIAL_DEBUG
    pc.printf("RemoveMessage, id is %04x\r\n",id);
#endif

    if( id == UINT32_MAX )
    {
        removeAllMessages();
        showBanner();
        currentMessageId = UINT32_MAX;
    }
    else
    {
        /* Have we removed the message currently displayed? */
        if( id == currentMessageId )
        {
#if defined SERIAL_DEBUG
            pc.printf("That's the current message!\r\n");
#endif
            if( !showMessageFromHistory( OFFSET_BEFORE ) )
            {
                if( !showMessageFromHistory( OFFSET_AFTER ) )
                {
                    showBanner();
                }
            }
        }
    
        removeMessage( id );
    }
}

void handleNewMessage( const uint8_t* const p_data )
{
    uint32_t id = (uint32_t)p_data[0] |
                  (uint32_t)p_data[1] |
                  (uint32_t)p_data[2] |
                  (uint32_t)p_data[3];
    unsigned row = p_data[4];
    char strBuffer[ MAX_ROW_LEN + 1 ];
    DisplayMessage_t newMessage;
    DisplayMessage_t* msgPtr;

#if defined SERIAL_DEBUG
    pc.printf("NewMessage, id is %04x row is %d\r\n",id,row);
#if 0
    for( unsigned i = 0; i < 60 ; i++ )
    {
        pc.printf("%02x ",p_data[i]);
    }
    pc.printf("\r\n");
#endif
#endif

    if( row < MAX_ROW_COUNT )
    {
        if( msgPtr = getMessage( id, OFFSET_NONE ))
        {
#if defined SERIAL_DEBUG
            pc.printf("Found an existing message\r\n");
#endif
            newMessage = *msgPtr;
        }

        strncpy( strBuffer, (const char*)(&(p_data[ 14 ])), MAX_ROW_LEN );
        strBuffer[ MAX_ROW_LEN ] = 0;
    
        newMessage.id = id;
        newMessage.dismissed = false;
        newMessage.lines[row] = strBuffer;
        newMessage.scrollEffect = (LCDIf::ScrollEffect_t)(p_data[4]);
        addMessage( &newMessage );

        displayMessage( &newMessage );
        lcdInterface.resetSleepTimer();
    }
}

void handleReport( HID_REPORT* recv_report )
{
#if defined SERIAL_DEBUG
    pc.printf("Message received with message type %02x\r\n",recv_report->data[0]);
#endif

    switch( recv_report->data[0] )
    {
        case MESSAGE_ID_NEW_MESSAGE:
            handleNewMessage( &(recv_report->data[1]) );
            break;
        case MESSAGE_ID_REMOVE_MESSAGE:
            handleRemoveMessage( &(recv_report->data[1]) );
            break;
    }
}

int main() {
#if defined SERIAL_DEBUG
    pc.printf("Hello");
#endif

    RoundRobin::instance()->SetBaseRate( BASE_RATE );
    RoundRobin::instance()->addTask( LCD_REFRESH_MULTIPLE, lcdRefresh );
    RoundRobin::instance()->addTask( BUTTON_WATCH_MULTIPLE, watchButtons );
    
#if 0
    DisplayMessage_t test;
    test.id = 1;
    test.lines[0] = "Hello";
    test.lines[1] = "There";
    addMessage( &test );
    test.id = 2;
    test.lines[0] = "This is a test";
    test.lines[1] = "Of the interface history";
    addMessage( &test );
#endif

    showBanner();

    while(true) {
        HID_REPORT recv_report;

#if defined SERIAL_DEBUG
        pc.printf("+");
#endif

        //try to read a msg
        while(hid.readNB(&recv_report)) {
            handleReport( &recv_report );
        }

    HID_REPORT send_report;


            send_report.length = 64;
    //Send the report
    hid.send(&send_report);

    //Fill the report
    for (unsigned i = 0; i < send_report.length; i++)
    {
        send_report.data[i] = rand() & 0xff;
    }

#if defined SERIAL_DEBUG
        pc.printf("Button State: %f %d\r\n",lx.readButton(),lx.pressedButton());
#endif
        wait(1);
    }
}
