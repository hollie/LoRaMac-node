#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "utilities.h"

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand( ) % ( max - min + 1 ) + min;
}

void memcpy1( uint8_t *dst, uint8_t *src, uint16_t size )
{
    while( size-- )
    {
        *dst++ = *src++;
    }
}

void memset1( uint8_t *dst, uint8_t value, uint16_t size )
{
    while( size-- )
    {
        *dst++ = value;
    }
}

int8_t Nibble2HexChar( uint8_t a )
{
	if( a < 10 )
    {
		return '0' + a;
	}
    else if( a < 16 )
    {
		return 'A' + ( a - 10 );
	}
    else
    {
		return '?';
	}
}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
int __io_putchar( int c )
#else /* __GNUC__ */
int fputc( int c, FILE *stream )
#endif
{
#if (__CORTEX_M == 0x00)
	// Use LE UART or other
	return( c );
#else
	return( ITM_SendChar( c ) );
#endif
}
