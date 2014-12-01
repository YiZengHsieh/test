
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "nRF24L01.h"

#define NRF_SB	1
#define NRF_ESB	2
#define NRF_PL	3

//#define SENDER
#define RFMODE	NRF_SB

/** The address of the radio. Parameter to the radio init */
static const uint8_t address[RF_ADDRESS_LENGTH] = {0x22,0x33,0x44};

//-----------------------------------------------------------------------------------------------------------

/** Function that initialises everything. Calls @b system_init () which is
 * hardware dependant, and @b device_boot_msg () from @b system.c.
 * It implementes a simple statemachine to handle the input from the user on 
 * the evaluation board. With two clicks, the user can choose between 
 * primary transmitter mode (PTX) and primary reciever mode (PRX), and between
 * the functionality levels ShockBurst (sb), Enchanced ShockBurst, 
 * and Enhanced ShockBurst with Bidirectional data (pl).
 */  



volatile int adcr[3][3];

void ADC_Init( void )
{
	ADCSRA=( 1<<ADEN )|( 1<<ADPS2 )|( 1<<ADPS1 )|( 0<<ADPS0 );	//divison factor = 64
	ADMUX=( 1<<REFS1 )|( 1<<REFS0 );	//internal 1.1V
	sbi(ADCSRA, ADSC);	// Start conversion
	while (ADCSRA & (1<<ADSC));	// First dummy conversion
	sbi(ADCSRA, ADIF);
	//ADEN:enable ADC
	//ADPS:divison factor=64
}

void ADC_Read( void )
{
	ADMUX &= 0xE0;
	ADMUX |= (1<<MUX2)|(1<<MUX0);	//PORT5(ADC5) X-axis
	
	ADCSRA |= ( 1<<ADIF );
	ADCSRA |= ( 1<<ADSC );
	while( ( ADCSRA&( 1<<ADIF ) )==0);//ADIF:adc conversion finish
	adcr[0][1] = ADCL;
	adcr[0][1] |= ( ADCH<<8 );//X axis acceleration
	_delay_ms(10);

	
	ADMUX &= 0xE0;
	ADMUX |= (1<<MUX2)|(1<<MUX1);	//ADC6	Y-axis
	ADCSRA |= ( 1<<ADIF );
	ADCSRA |= ( 1<<ADSC );
	while( ( ADCSRA&( 1<<ADIF ) )==0 );
	adcr[1][1] = ADCL;
	adcr[1][1] |= ( ADCH<<8 );//Y axis acceleration
	_delay_ms(10);

	ADMUX &= 0xE0;
	ADMUX |= (1<<MUX2)|(1<<MUX1)|(1<<MUX0);
	ADCSRA |= ( 1<<ADIF );
	ADCSRA |= ( 1<<ADSC );
	while( ( ADCSRA&( 1<<ADIF ) )==0 );
	adcr[2][1] = ADCL;
	adcr[2][1] |= ( ADCH<<8 );//Z axis acceleration
	_delay_ms(10);
}

void Data_Shift( void )
{
	adcr[0][2] = adcr[0][1];//shift x-axis data
	adcr[1][2] = adcr[1][1];//shift y-axis data
	adcr[2][2] = adcr[2][1];//shift z-axis data
}



int main(void) 
{
	uint8_t i;
	
	// Set clock divider to F_CLK/1
#ifdef __AVR_ATmega168__
	CLKPR = 0x80;
	CLKPR = 0x00;
#endif

	Set_Delay_CPU_CLOCK(10000000UL);
	
	nrf_system_init();                  //Hardware dependant system initialisation
	UART_Init();

	// Init IO Ports
	sbi(LED1_DDR, LED1_BIT);
	sbi(LED2_DDR, LED2_BIT);
	sbi(LED3_DDR, LED3_BIT);
	
	cbi(USER_BTN_DDR, USER_BTN_BIT);
	sbi(USER_BTN_PORT, USER_BTN_BIT);

	LED_ALL_OFF();                  //Turn off all lights

	// Turn off the RF first
//	DELAY_MS(100);	// Delay power-on-reset for nrf24L01+
//	radio_off();

	DELAY_MS(100);	// Delay power-on-reset for nrf24L01+

	for (i=0; i<RF_PAYLOAD_LENGTH;i++)
		nrf_pload[i] = 0;
	
	/*ADC*/
	
	ADC_Init();
	for( i = 0; i < 5; i++ )
	{
		ADC_Read();
		Data_Shift();
		
	}
	

	while (true)
	{
	
#ifdef SENDER

		radio_operational_mode(HAL_NRF_PTX);

	#if RFMODE == NRF_SB
		//Start as PTX in ShockBurst
		radio_init(HAL_NRF_SB, address, rx_buff);
	#elif RFMODE == NRF_ESB
		//Start as PTX in Enhanced 
		radio_init(HAL_NRF_ESB, address, rx_buff);
	#elif RFMODE == NRF_PL
		// Start as PTX in Enhanced PL
		radio_init(HAL_NRF_PL, address, rx_buff);
	#endif

		// Send data from buffer to RF
		device_ptx_mode();

#else

		radio_operational_mode(HAL_NRF_PRX);

	#if RFMODE == NRF_SB
		// Start as PRX in ShockBurst 
		//radio_sb_init (address, HAL_NRF_PRX);
		radio_init(HAL_NRF_SB, address, rx_buff);
	#elif RFMODE == NRF_ESB
		// Start as PRX in Enhanced 
		radio_init(HAL_NRF_ESB, address, rx_buff);
	#elif RFMODE == NRF_PL
		//Start as PRX in Enhanced PL
		radio_init(HAL_NRF_PL, address, rx_buff);
	#endif

		// receive data from RF to buffer
		device_prx_mode();
	
#endif
		
		//ADC_Read();
		/*
		for( unsigned char k=0; k<3; k++ )
		{
			USART_Transmit( 0x55 );
			USART_Transmit( k );
			USART_Transmit( 2 );
			USART_Transmit( adcr[k][1]>>8 );
			USART_Transmit( adcr[k][1] );
			USART_Transmit( 0x04 );
			//USART_Transmit( drop_flag_back );
			_delay_ms( 10 );
		}
		*/
		
		
/*		for( unsigned char k=0; k<2; k++ )
		{
				rx_buff[k] = 0x55;
				rx_buff[k+1] = k;
				rx_buff[k+2] = 2;
				rx_buff[k+3] = adcr[k][1]>>8;
				rx_buff[k+4] = adcr[k][1];
				rx_buff[k+5] = 0x04;
				_delay_ms( 10 );
			
		}*/
		//Data_Shift();
		//_delay_ms( 50 );
		
		/*
		for( int j = 0; j< 10; j++ )
		{
			rx_buff[j] = j;
		}
		*/
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------
#ifdef SENDER

void device_ptx_mode(void)
{
	uint8_t i=0, j=0;
	while(true)
	{
		LED1_TOGGLE();
		//LED2_ON();
		
				

		// Wait till the packet is sent
		do
		{
			radio_irq ();
		} while((radio_get_status ()) == RF_BUSY);

#if RFMODE == NRF_PL || RFMODE == NRF_ESB
		// If ACK is recieved
		if (((radio_get_status ()) == RF_TX_DS) 
	#if RFMODE == NRF_PL
			|| ((radio_get_status ()) == RF_TX_AP)
	#endif
			)
		{
			LED2_TOGGLE();
		}
		else
			LED2_OFF();

	#if RFMODE == NRF_PL
		// If ACK payload was recieved, get the payload
		if (radio_get_status () == RF_TX_AP)
		{
			// Get the payload from the PRX and set LED1 accordingly
			for (i=0; i<RF_PAYLOAD_LENGTH; i++)
			{
				UART_PutChar(rx_buff[i]);
			}

			LED3_TOGGLE();
		}
		else
			LED3_OFF();

	#endif
#endif

		// Sleep 100ms
		//DELAY_MS(100);

		// Set up the payload according to the input button 1
		//nrf_pload[0] = 0x51;
		/*
		for( unsigned char k=0; k<2; k++ )
		{
				nrf_pload[k] = 0x55;
				nrf_pload[k+1] = k;
				nrf_pload[k+2] = 2;
				nrf_pload[k+3] = adcr[k][1]>>8;
				nrf_pload[k+4] = adcr[k][1];
				nrf_pload[k+5] = 0x04;
				//_delay_ms( 10 );
			
		}
		*/
		/*
		for( j = 0; j<5;j++ )
		{
			ADC_Read();
		// x
			//nrf_pload[0] = 0x55;
			//nrf_pload[1] = 0;
			//nrf_pload[2] = 2;
			nrf_pload[6*j+0] = adcr[0][1]>>8;
			nrf_pload[6*j+1] = adcr[0][1];
			//nrf_pload[5] = 0x04;
		// y	
			//nrf_pload[6] = 0x55;
			//nrf_pload[7] = 1;
			//nrf_pload[8] = 2;
			nrf_pload[6*j+2] = adcr[1][1]>>8;
			nrf_pload[6*j+3] = adcr[1][1];
			//nrf_pload[11] = 0x04;
		// z		
			//nrf_pload[12] = 0x55;
			//nrf_pload[13] = 2;
			//nrf_pload[14] = 2;
			nrf_pload[6*j+4] = adcr[2][1]>>8;
			nrf_pload[6*j+5] = adcr[2][1];
			//nrf_pload[17] = 0x04;
		}*/
		
		
		/* 0x55 */
		
		//_delay_ms(20);
		ADC_Read();

		nrf_pload[0] = 0x55;
		nrf_pload[1] = adcr[0][1]>>8;
		nrf_pload[2] = adcr[0][1];
		nrf_pload[3] = 0x55;
		nrf_pload[4] = adcr[1][1]>>8;
		nrf_pload[5] = adcr[1][1];
		nrf_pload[6] = 0x55;
		nrf_pload[7] = adcr[2][1]>>8;
		nrf_pload[8] = adcr[2][1];

		_delay_ms(20);
		
		ADC_Read();
		nrf_pload[9] = 0x55;
		nrf_pload[10] = adcr[0][1]>>8;
		nrf_pload[11] = adcr[0][1];
		nrf_pload[12] = 0x55;
		nrf_pload[13] = adcr[1][1]>>8;
		nrf_pload[14] = adcr[1][1];
		nrf_pload[15] = 0x55;
		nrf_pload[16] = adcr[2][1]>>8;
		nrf_pload[17] = adcr[2][1];

		_delay_ms(20);
		ADC_Read();
		nrf_pload[18] = 0x55;
		nrf_pload[19] = adcr[0][1]>>8;
		nrf_pload[20] = adcr[0][1];
		nrf_pload[21] = 0x55;
		nrf_pload[22] = adcr[1][1]>>8;
		nrf_pload[23] = adcr[1][1];
		nrf_pload[24] = 0x55;
		nrf_pload[25] = adcr[2][1]>>8;
		nrf_pload[26] = adcr[2][1];

/*
		if(USER_BTN_PRESSED())
		{
		nrf_pload[6] = 0x56;
		
		}
*/
/* 0x54 */
/*
		_delay_ms(53);
		nrf_pload[0] = 0x54;
		nrf_pload[1] = 0x02;
		nrf_pload[2] = 0xbc;
		nrf_pload[3] = 0x54;
		nrf_pload[4] = 0x02;
		nrf_pload[5] = 0x58;
		nrf_pload[6] = 0x54;
		nrf_pload[7] = 0x01;
		nrf_pload[8] = 0xF4;

_delay_ms(53);
		
		nrf_pload[9] = 0x54;
		nrf_pload[10] = 0x02;
		nrf_pload[11] = 0xC0;
		nrf_pload[12] = 0x54;
		nrf_pload[13] = 0x02;
		nrf_pload[14] = 0x01;
		nrf_pload[15] = 0x54;
		nrf_pload[16] = 0x01;
		nrf_pload[17] = 0xCC;

_delay_ms(53);
		
		nrf_pload[18] = 0x54;
		nrf_pload[19] = 0x02;
		nrf_pload[20] = 0xA1;
		nrf_pload[21] = 0x54;
		nrf_pload[22] = 0x02;
		nrf_pload[23] = 0x36;
		nrf_pload[24] = 0x54;
		nrf_pload[25] = 0x01;
		nrf_pload[26] = 0xD2;

		
		/* 0x53 */
		/*
		_delay_ms(55);
		nrf_pload[0] = 0x53;
		nrf_pload[1] = 0x01;
		nrf_pload[2] = 0xF4;
		nrf_pload[3] = 0x53;
		nrf_pload[4] = 0x02;
		nrf_pload[5] = 0x38;
		nrf_pload[6] = 0x53;
		nrf_pload[7] = 0x02;
		nrf_pload[8] = 0xbf;

_delay_ms(55);
		
		nrf_pload[9] = 0x53;
		nrf_pload[10] = 0x01;
		nrf_pload[11] = 0xF4;
		nrf_pload[12] = 0x53;
		nrf_pload[13] = 0x02;
		nrf_pload[14] = 0x38;
		nrf_pload[15] = 0x53;
		nrf_pload[16] = 0x02;
		nrf_pload[17] = 0xbf;

_delay_ms(55);
		
		nrf_pload[18] = 0x53;
		nrf_pload[19] = 0x01;
		nrf_pload[20] = 0xF4;
		nrf_pload[21] = 0x53;
		nrf_pload[22] = 0x02;
		nrf_pload[23] = 0x38;
		nrf_pload[24] = 0x53;
		nrf_pload[25] = 0x02;
		nrf_pload[26] = 0xbf;
		
		
		
			/* 0x52 */
		/*
		_delay_ms(50);
		nrf_pload[0] = 0x52;
		nrf_pload[1] = 0x02;
		nrf_pload[2] = 0xbc;
		nrf_pload[3] = 0x52;
		nrf_pload[4] = 0x02;
		nrf_pload[5] = 0x58;
		nrf_pload[6] = 0x52;
		nrf_pload[7] = 0x01;
		nrf_pload[8] = 0xF4;

_delay_ms(50);
		
		nrf_pload[9] = 0x52;
		nrf_pload[10] = 0x02;
		nrf_pload[11] = 0xbc;
		nrf_pload[12] = 0x52;
		nrf_pload[13] = 0x02;
		nrf_pload[14] = 0x50;
		nrf_pload[15] = 0x52;
		nrf_pload[16] = 0x01;
		nrf_pload[17] = 0xF0;

_delay_ms(50);
		
		nrf_pload[18] = 0x52;
		nrf_pload[19] = 0x02;
		nrf_pload[20] = 0xbF;
		nrf_pload[21] = 0x52;
		nrf_pload[22] = 0x02;
		nrf_pload[23] = 0x50;
		nrf_pload[24] = 0x52;
		nrf_pload[25] = 0x01;
		nrf_pload[26] = 0xF0;

		
		
		/*for (i=0; i<RF_PAYLOAD_LENGTH; i++)
		{
			UART_PutChar(nrf_pload[i]);
		}*/
		//_delay_ms(100);
			
		/*
		ADC_Read();
		nrf_pload[6] = adcr[0][1]>>8;
		nrf_pload[7] = adcr[0][1];
		nrf_pload[8] = adcr[1][1]>>8;
		nrf_pload[9] = adcr[1][1];
		nrf_pload[10] = adcr[2][1]>>8;
		nrf_pload[11] = adcr[2][1];
		*/
		//Send the packet
		radio_send_packet(nrf_pload, RF_PAYLOAD_LENGTH);           
	}
}

//-----------------------------------------------------------------------------------------------------------

#else
	//UART_PutChar(0x98);

//-----------------------------------------------------------------------------------------------------------

void device_prx_mode(void)
{
	uint8_t i=0, j=0;
	uint8_t loop=0;

	_RF_CE_HIGH();        // Set Chip Enable (CE) pin high to enable receiver
	DELAY_US(130);

	while(true)
	{ 
		LED1_TOGGLE();
		//UART_PutChar(0x36);
		
		
		
#if RFMODE == NRF_PL
		// Used in PL mode only
		// Setup and put the ACK payload on the TX FIFO
		nrf_pload[0] = 0x52;
		hal_nrf_write_ack_pload (0, nrf_pload, RF_PAYLOAD_LENGTH);
#endif

		// Run until either 110ms has elapsed 
		// OR there is data on the radio
		loop = 0;
		do
		{
			// Check IRQ, if data available, put the data from RX FIFO into buffer array
			radio_irq ();
			loop++;
			//DELAY_MS(1);
		} while ((radio_get_status () == RF_IDLE) && loop<IRQ_LOOP);

		//UART_PutChar(radio_get_status());
		
		if ((radio_get_status () == RF_RX_DR) || (radio_get_status () == RF_TX_AP))
		{
		  // Get the payload from the PTX and set LED accordingly
			LED2_TOGGLE();
			//UART_PutChar(0x37);
			/*
			for (i=0; i<RF_PAYLOAD_LENGTH; i++)
			{
				//UART_PutChar(rx_buff[i]);
				for( j = 0; j<5;j++ )
				{
					// x
					UART_PutChar( 0x55 );
					UART_PutChar( 0x00 );
					UART_PutChar( 0x02 );
					UART_PutChar( rx_buff[6*j+0] );
					UART_PutChar( rx_buff[6*j+1] );
					UART_PutChar( 0x04 );
					
					// y
					UART_PutChar( 0x55 );
					UART_PutChar( 0x01 );
					UART_PutChar( 0x02 );
					UART_PutChar( rx_buff[6*j+2] );
					UART_PutChar( rx_buff[6*j+3] );
					UART_PutChar( 0x04 );
					
					// z
					UART_PutChar( 0x55 );
					UART_PutChar( 0x02 );
					UART_PutChar( 0x02 );
					UART_PutChar( rx_buff[6*j+4] );
					UART_PutChar( rx_buff[6*j+5] );
					UART_PutChar( 0x04 );
					//DELAY_MS(10);
				}
			}*/
			UART_PutChar( rx_buff[0] );
			UART_PutChar( 0x00 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[1] );
			UART_PutChar( rx_buff[2] );
			//UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[3] );
			UART_PutChar( 0x01 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[4] );
			UART_PutChar( rx_buff[5] );
			//UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[6] );
			UART_PutChar( 0x02 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[7] );
			UART_PutChar( rx_buff[8] );
			//UART_PutChar( 0x04 );
			
			UART_PutChar( rx_buff[9] );
			UART_PutChar( 0x00 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[10] );
			UART_PutChar( rx_buff[11] );
			//UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[12] );
			UART_PutChar( 0x01 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[13] );
			UART_PutChar( rx_buff[14] );
			//UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[15] );
			UART_PutChar( 0x02 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[16] );
			UART_PutChar( rx_buff[17] );
			//UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[18] );
			UART_PutChar( 0x00 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[19] );
			UART_PutChar( rx_buff[20] );
			//UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[21] );
			UART_PutChar( 0x01 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[22] );
			UART_PutChar( rx_buff[23] );
			//UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[24] );
			UART_PutChar( 0x02 );
			//UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[25] );
			UART_PutChar( rx_buff[26] );
			//UART_PutChar( 0x04 );

/*
			UART_PutChar( rx_buff[0] );
			UART_PutChar( 0x00 );
			UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[18] );
			UART_PutChar( rx_buff[19] );
			UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[0] );
			UART_PutChar( 0x01 );
			UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[20] );
			UART_PutChar( rx_buff[21] );
			UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[0] );
			UART_PutChar( 0x02 );
			UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[22] );
			UART_PutChar( rx_buff[23] );
			UART_PutChar( 0x04 );


			UART_PutChar( rx_buff[0] );
			UART_PutChar( 0x00 );
			UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[24] );
			UART_PutChar( rx_buff[25] );
			UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[0] );
			UART_PutChar( 0x01 );
			UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[26] );
			UART_PutChar( rx_buff[27] );
			UART_PutChar( 0x04 );

			UART_PutChar( rx_buff[0] );
			UART_PutChar( 0x02 );
			UART_PutChar( 0x02 );
			UART_PutChar( rx_buff[28] );
			UART_PutChar( rx_buff[29] );
			UART_PutChar( 0x04 );
*/

		}
		else
		{
			LED2_OFF();
			//UART_PutChar(0x39);
		}

#if RFMODE == NRF_PL
		if ((radio_get_status ()) == RF_TX_DS)
		{
			// This block is used in PL mode only, TX_DS is activated after payload ACK is sent
			LED3_TOGGLE();
			//UART_PutChar(0x38);
		}
		else
			LED3_OFF();
#endif

		// Set radio status to idle
		radio_set_status (RF_IDLE);

	}
	_RF_CE_LOW();        // Set Chip Enable (CE) pin low to go to stanby I mode
}

#endif
//-----------------------------------------------------------------------------------------------------------
