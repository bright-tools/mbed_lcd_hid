#if !defined TIME_HELP_HPP
#define      TIME_HELP_HPP

#define SECONDS( _x ) _x ## F
#define SECONDS_IN_MINUTES( _x ) ( _x * 60 )

#define LCD_REFRESH_RATE SECONDS( 0.2 )
#define BUTTON_WATCH_RATE SECONDS( 0.1 )

#endif
