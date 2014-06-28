#include <string.h>
#include <math.h>
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"

/*!
 * When set to 1 the application uses the Over-the-Air activation procedure
 * When set to 0 the application uses the Personalization activation procedure
 */
#define OVER_THE_AIR_ACTIVATION		0

/*!
 * Mote device IEEE EUI
 */
uint8_t DevEui[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#if( OVER_THE_AIR_ACTIVATION != 0 )
	#define OVER_THE_AIR_ACTIVATION_DUTYCYCLE	10000000  // 10 [s] value in us

	/*!
	 * Application IEEE EUI
	 */
	uint8_t AppEui[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	/*!
	 * AES encryption/decryption cipher application key
	 */
	uint8_t AppKey[] = 
	{ 
		0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
		0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};

#else
	/*!
	 * AES encryption/decryption cipher network session key
	 */
	uint8_t NwkSKey[] = 
	{ 
		0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
		0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};

	/*!
	 * AES encryption/decryption cipher application session key
	 */
	uint8_t AppSKey[] = 
	{ 
		0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
		0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
	};

	/*!
	 * Device address
	 */
	uint32_t DevAddr;

#endif

/*!
 * Indicates if the MAC layer has already joined a network.
 */
bool IsNetworkJoined = false;

/*!
 * Defines the application data transmission duty cycle
 */
#define APP_TX_DUTYCYCLE		5000000  // 5 [s] value in us
#define APP_TX_DUTYCYCLE_RND	1000000  // 1 [s] value in us

/*!
 * User application data buffer size
 */
#define APP_DATA_SIZE	16

/*!
 * User application data
 */
uint8_t AppData[APP_DATA_SIZE];

/*!
 * Defines the application data transmission duty cycle
 */
uint32_t TxDutyCycleTime;

TimerEvent_t TxNextPacketTimer;

#if( OVER_THE_AIR_ACTIVATION != 0 )
	/*!
	 * Defines the join request timer
	 */
	TimerEvent_t JoinReqTimer;
#endif

/*!
 * Indicates if a new packet can be sent
 */
bool TxNextPacket = true;
bool TxDone = false;
bool RxDone = false;
bool TxAckReceived = false;

bool AppLedStateOn = false;

LoRaMacEvent_t prvLoRaMacEvents;

TimerEvent_t Led1Timer;
volatile bool Led1TimerEvent = false;

TimerEvent_t Led2Timer;
volatile bool Led2TimerEvent = false;

TimerEvent_t Led4Timer;
volatile bool Led4TimerEvent = false;

#if( OVER_THE_AIR_ACTIVATION != 0 )

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnJoinReqTimerEvent( void )
{
	TxNextPacket = true;
}

#endif

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnTxNextPacketTimerEvent( void )
{
	TxNextPacket = true;
}

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnLed1TimerEvent( void )
{
	Led1TimerEvent = true;
}

/*!
 * \brief Function executed on Led 2 Timeout event
 */
void OnLed2TimerEvent( void )
{
	Led2TimerEvent = true;
}

/*!
 * \brief Function executed on Led 4 Timeout event
 */
void OnLed4TimerEvent( void )
{
	Led4TimerEvent = true;
}

/*!
 * \brief Function to be executed on MAC layer event
 */
void OnMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
	if( info->Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT )
	{
		TxDone = true;
	}
	if( flags->Bits.JoinAccept == 1 )
	{
#if( OVER_THE_AIR_ACTIVATION != 0 )
		TimerStop( &JoinReqTimer );
#endif
		IsNetworkJoined = true;
	}
	
	if( flags->Bits.Tx == 1 )
	{
		if( info->TxAckReceived == true )
		{
			TxAckReceived = true;
		}
		TxDone = true;
	}

	if( flags->Bits.Rx == 1 )
	{
		if( ( ( info->RxPort == 1 ) || ( info->RxPort == 2 ) ) && ( info->RxBufferSize > 0 ) )
		{
			AppLedStateOn = info->RxBuffer[0];
		}
		RxDone = true;
	}
}

/**
 * Main application entry point.
 */
int main( void )
{
	SystemCoreClockUpdate();
while(1);
	BoardInitPeriph( );
	BoardInitMcu( );

	// Initialize LoRaMac device unique ID
	BoardGetUniqueId( DevEui );
/*
while(1)
{
	uint8_t data;
	if (UartGetChar(&UartUsb, &data) == 0)
	{
		UartPutChar(&UartUsb, data);
	}
}
*/
	prvLoRaMacEvents.MacEvent = OnMacEvent;
	LoRaMacInit( &prvLoRaMacEvents );

	IsNetworkJoined = false;

#if( OVER_THE_AIR_ACTIVATION == 0 )
	// Random seed initialization
	srand( RAND_SEED );
	// Choose a random device address
	// NwkID = 0
	// NwkAddr rand [0, 33554431]
	DevAddr = randr( 0, 0x01FFFFFF );

	LoRaMacInitNwkIds( 0x000000, DevAddr, NwkSKey, AppSKey );
	IsNetworkJoined = true;
#else
	// Sends a JoinReq Command every 5 seconds until the network is joined
	TimerInit( &JoinReqTimer, OnJoinReqTimerEvent ); 
	TimerSetValue( &JoinReqTimer, OVER_THE_AIR_ACTIVATION_DUTYCYCLE );
#endif

	TxNextPacket = true;
	TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );

	TimerInit( &Led1Timer, OnLed1TimerEvent );
	TimerSetValue( &Led1Timer, 25000 );

	TimerInit( &Led2Timer, OnLed2TimerEvent );
	TimerSetValue( &Led2Timer, 25000 );

	TimerInit( &Led4Timer, OnLed4TimerEvent );
	TimerSetValue( &Led4Timer, 25000 );

	LoRaMacSetAdrOn( true );

	while( 1 )
	{
		while( IsNetworkJoined == false )
		{
#if( OVER_THE_AIR_ACTIVATION != 0 )
			if( TxNextPacket == true )
			{
				TxNextPacket = false;

				LoRaMacJoinReq( DevEui, AppEui, AppKey );

				// Relaunch timer for next trial
				TimerStart( &JoinReqTimer );
			}
			TimerLowPowerHandler( );
#endif
		}
		if( Led1TimerEvent == true )
		{
			Led1TimerEvent = false;
			LED_1_OFF();
		}
		if( Led2TimerEvent == true )
		{
			Led2TimerEvent = false;
			LED_2_OFF();
		}
		if( Led4TimerEvent == true )
		{
			Led4TimerEvent = false;
			// Switch LED 4 OFF
		}
		if( TxAckReceived == true )
		{
			TxAckReceived = false;
			LED_2_ON();
			TimerStart( &Led2Timer );
		}
		if( RxDone == true )
		{
			RxDone = false;
			if( AppLedStateOn == true )
			{
				LED_3_ON();
			}
			else
			{
				LED_3_OFF();
			}
		}
		
		if( TxDone == true )
		{
			TxDone = false;
			
			// Schedule next packet transmission
			TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
			TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
			TimerStart( &TxNextPacketTimer );
		}

		if( TxNextPacket == true )
		{
			TxNextPacket = false;

			LED_1_ON();
			TimerStart( &Led1Timer );

			AppData[0] = AppLedStateOn;
			AppData[5] = ( 0xAA55 >> 8 ) & 0xFF;
			AppData[6] = 0xAA55 & 0xFF;
			AppData[7] = 0xAA;
			AppData[8] = ( 0x22AA55 >> 16 ) & 0xFF;
			AppData[9] = ( 0x22AA55 >> 8 ) & 0xFF;
			AppData[10] = 0x22AA55 & 0xFF;
			AppData[11] = ( 0x3355AA >> 16 ) & 0xFF;
			AppData[12] = ( 0x3355AA >> 8 ) & 0xFF;
			AppData[13] = 0x3355AA & 0xFF;
			
			LoRaMacSendFrame( 2, AppData, APP_DATA_SIZE );
		}
		TimerLowPowerHandler( );
	}
}

