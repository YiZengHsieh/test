#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <compat/deprecated.h>


#define NRF_SB	1
#define NRF_ESB	2
#define NRF_PL	3

//#define SENDER
#define RFMODE	NRF_SB

//--------------------------------------------                                            
//定義宣告   
//--------------------------------------------
#define uint unsigned int
#define uchar unsigned char     

//--------------------------------------------
//副程式宣告     
//--------------------------------------------     

void USART_Init( unsigned int ubrr);

void USART_TXD( unsigned char Data );

/** The address of the radio. Parameter to the radio init */
//static const uint8_t address[RF_ADDRESS_LENGTH] = {0x22,0x33,0x44};

//-----------------------------------------------------------------------------------------------------------

/** Function that initialises everything. Calls @b system_init () which is
 * hardware dependant, and @b device_boot_msg () from @b system.c.
 * It implementes a simple statemachine to handle the input from the user on 
 * the evaluation board. With two clicks, the user can choose between 
 * primary transmitter mode (PTX) and primary reciever mode (PRX), and between
 * the functionality levels ShockBurst (sb), Enchanced ShockBurst, 
 * and Enhanced ShockBurst with Bidirectional data (pl).
 */  

uchar g_k1=0x00;
uchar g_k2=0x00;
   
uchar g_k3=0x00;
uchar g_k4=0x00;

uchar o_k1=0x00;
uchar o_k2=0x00;
   
uchar o_k3=0x00;
uchar o_k4=0x00;

uchar p_i=0x00;

uchar p_r=0x00;



uchar accr[3][2];


volatile int adcr[3][3];

//光遮斷器
void photointerrupter(void)
{
    if( (PINB & (1<<PINB3)) == 0 )	  // check bus for HIGH, othwise we have a short circuit
      p_i=0x01;//USART_TXD( 0x01 );
	else
	  p_i=0x02;//USART_TXD( 0x02 );
}

//光敏電阻Photoresistor
void Photoresistor(void)
{
    if( (PINB & (1<<PINB1)) == 0 )	  // check bus for HIGH, othwise we have a short circuit
      p_r=0x03;//USART_TXD( 0x03 );
	else
	  p_r=0x04;//USART_TXD( 0x04 );
}



//溫度感測器
uchar w1_reset(void)  
{
    PORTB &= ~(1<<PORTB2);               // One-wire LOW
    DDRB |= 1<<DDB2;
  
    _delay_us( 480 );			  // 480 us
  
    DDRB &= ~(1<<DDB2);               
    _delay_us( 65 );               // Wait 15 + 50 us
    uchar err = PINB & (1<<PINB2);		  // Sample bus
  
    _delay_us( 480 - 65 );         // wait until cycle is gone 
  
    if( (PINB & (1<<PINB2)) == 0 )	  // check bus for HIGH, othwise we have a short circuit
      err = 1;
	  
	  return err;
} 

uchar w1_bit_read( void )
{
    PORTB &= ~(1<<PORTB2);               // One-wire LOW
    DDRB |= 1<<DDB2;
    
    _delay_us( 1 );                // 1 us time 
    DDRB &= ~(1<<DDB2);                // One-wire HIGH and use as input

    _delay_us( 14 );               // 14 us 
    uchar value = PINB & (1<<PINB2);    // sample bus
    _delay_us( 45 );               // 45 us 

    return value;
}
void w1_bit_write( uchar b )
{
    PORTB &= ~(1<<PORTB2);               // One-wire LOW
    DDRB |= 1<<DDB2;
    _delay_us( 10 );               // 10 us 

    if ( b ) 
    {
        DDRB &= ~(1<<DDB2);           // One-wire input    
        _delay_us( 5 + 45 );       // in total 60 us
    }
    else
    {
        _delay_us( 5 + 45 );       // 60 us in total
        DDRB &= ~(1<<DDB2);           // One-wire HIGH and use as input    
    }
}
int w1_read( uchar bits)
{
    uchar i = bits;
    int  value = 1 << (bits-1);
    int  b = 0;

    do
    {
       b >>= 1;
       if( w1_bit_read() ) b |= value;
    
    } while( --i );

    return b;
}
uchar w1_byte_rd( void )
{
   return (uchar) w1_read(8);
}


void w1_byte_wr( uchar b )
{
  uchar i = 8;
  do
  {
    w1_bit_write( b & 1 );
    b >>= 1;

  } while( --i );
}
/****************************************************************************
 * Start measurement
 ****************************************************************************/
 void start_meas( void )
 {
   if( PINB & 1<< PINB2 )
   {
     w1_byte_wr( 0xEE );
   }
 }


/****************************************************************************
 * Display measurement result
 ****************************************************************************/
 void read_meas( void )
 {
   //char s[20];
   int t;
   int r;
   unsigned int c;

   long temperature;

   w1_reset();
   w1_byte_wr( 0xAA );			// read temp
   t = w1_byte_rd();			// low byte
   
   w1_reset();
   w1_byte_wr( 0xA0 );          // read remaining count
   r = w1_read(9);

   w1_reset();
   w1_byte_wr( 0x41 );          // load counter

   w1_reset();
   w1_byte_wr( 0xA0 );          // read slope counter
   c = w1_read(9);

   

   if ( t > 0x80 ) 
   {
      t = t-256;
   }

   temperature   = 10 * t - 5;
   temperature  += ( 10 * ( c - r )) / c ;

   t = temperature / 10;
   r = temperature % 10;
   
   uchar k1=t>>8;
   g_k1=k1;
   uchar k2=t-(k1<<8);
   g_k2=k2;
   uchar k3=r>>8;
   g_k3=k3;
   uchar k4=r-(k3<<8);
   g_k4=k4;
   
   o_k1=g_k1;
   o_k2=g_k2;   
   o_k3=g_k3;
   o_k4=g_k4;
   //USART_TXD( 0xFF );
   //USART_TXD( k1 );
   //USART_TXD( k2 );
   //USART_TXD( k3 );
   //USART_TXD( k4 );
   //USART_TXD( 0xFF );
   
   
 }

//加速度計

void ADC_Init( void )
{
	ADCSRA=( 1<<ADEN )|( 1<<ADPS2 )|( 1<<ADPS1 )|( 0<<ADPS0 );	//divison factor = 64
	ADMUX=( 1<<REFS1 )|( 1<<REFS0 );	//internal 2.56V
	sbi(ADCSRA, ADSC);	// Start conversion
	while (ADCSRA & (1<<ADSC));	// First dummy conversion
	sbi(ADCSRA, ADIF);
	//ADEN:enable ADC
	//ADPS:divison factor=64
}

void ADC_Read( void )
{
	ADMUX &= 0xE0;//應該要0xF0,因為永遠都是右對齊
	ADMUX |= (1<<MUX2)|(1<<MUX0);	//PORT5(ADC5) X-axis
	
	//應該要 ADCSRA |= ( 1<<ADIF )|( 1<<ADSC );
	//跟晶片說我要開始做A/D並且取直是從有interupte flag的地方開始取
	ADCSRA |= ( 1<<ADIF );
	ADCSRA |= ( 1<<ADSC );
	while( ( ADCSRA&( 1<<ADIF ) )==0);//ADIF:adc conversion finish

	//value=ADCL+2^8*ADCH
	adcr[0][1] = ADCL;
	adcr[0][1] |= ( ADCH<<8 );//X axis acceleration
	_delay_ms(3);

	
	ADMUX &= 0xE0;
	ADMUX |= (1<<MUX2)|(1<<MUX1);	//ADC6	Y-axis
	ADCSRA |= ( 1<<ADIF );
	ADCSRA |= ( 1<<ADSC );
	while( ( ADCSRA&( 1<<ADIF ) )==0 );
	adcr[1][1] = ADCL;
	adcr[1][1] |= ( ADCH<<8 );//Y axis acceleration
	_delay_ms(3);

	ADMUX &= 0xE0;
	ADMUX |= (1<<MUX2)|(1<<MUX1)|(1<<MUX0);
	ADCSRA |= ( 1<<ADIF );
	ADCSRA |= ( 1<<ADSC );
	while( ( ADCSRA&( 1<<ADIF ) )==0 );
	adcr[2][1] = ADCL;
	adcr[2][1] |= ( ADCH<<8 );//Z axis acceleration
	_delay_ms(3);
}

void Data_Shift( void )
{
	adcr[0][2] = adcr[0][1];//shift x-axis data
	adcr[1][2] = adcr[1][1];//shift y-axis data
	adcr[2][2] = adcr[2][1];//shift z-axis data
}


#define FOSC 8000000// Clock Speed
#define BAUD 38400
#define MYUBRR FOSC/16/BAUD-1

void USART_Init( unsigned int ubrr)
{
/* Set baud rate */
UBRRH = (unsigned char)(ubrr>>8);
UBRRL = (unsigned char)ubrr;
/* Enable receiver and transmitter */
UCSRB = (1<<RXEN)|(1<<TXEN);
/* Set frame format: 8data, 2stop bit */
UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);

}







void USART_TXD(unsigned char data) 
{
	while ( !( UCSRA & (1<<UDRE)) ) ;	// Wait for empty transmit buffer 
	UDR = data;	 // Put data into buffer, sends the data 

	
}

int main(void) 
{
	uint8_t i;
	uchar status; 
	
	//光遮斷器
	PORTB &= ~(1<<PORTB3);               // One-wire LOW
	DDRB &= ~(1<<DDB3); 
	
	
	//光敏電阻Photoresistor
	PORTB &= ~(1<<PORTB1);               // One-wire LOW
	DDRB &= ~(1<<DDB1); 
	
	USART_Init(MYUBRR);//初始UART
	

    _delay_ms(100); 
	
	
	/*ADC*/
	
	ADC_Init();
	for( i = 0; i < 5; i++ )
	{
		ADC_Read();
		Data_Shift();
		
	}
	
	
		PORTB |= 1<< PB2;
        DDRB |= 1<< PB2;

        w1_reset(); 
        start_meas();
        
        for ( i=0; i < 250; i++)
        {
            _delay_us(10000);
            
            w1_reset();

            w1_byte_wr( 0xAC );
            status = w1_byte_rd();

           // Temperature conversion finished ?
            
            if ( status & 0x80)
            {
              //LED off
               USART_TXD( 0xAA ); 
               PORTB &= ~(1<< PB2);
               //break;
			   i=250;
            }
        }
        
       // read measurement value and 
        
        read_meas();
	
	
	uchar flag=0x00;
	
	int count=0;
	
	while (1)
	{
	
	
		flag=0x00;
		
	if(count%10==0)
	{
		PORTB |= 1<< PB2;
        DDRB |= 1<< PB2;

        w1_reset(); 
        start_meas();
		
        count=0;
        
        for(int i=0;i<250;i++)
        {   
            
            _delay_us(10000);
			
			w1_reset();

            w1_byte_wr( 0xAC );
            status = w1_byte_rd();

           // Temperature conversion finished ?
            
            if ( status & 0x80)
            {
              //LED off
               //USART_TXD( 0xAA ); 
               PORTB &= ~(1<< PB2);
               
			   flag=0x01;
			   i=250;
			   
            }
        }
		
	}
	
     count++;	
       // read measurement value and 
     
        
		if(flag==0x01)
		read_meas();
		else
		{
		   g_k1=o_k1;
           g_k2=o_k2;   
           g_k3=o_k3;
           g_k4=o_k4;
		
        }
        
		
		
		
       
	
	    photointerrupter();
		
		//_delay_ms( 100 );
		
		Photoresistor();
		
		//_delay_ms( 100 );
		
		
		ADC_Read();
		
		//_delay_ms( 100 );
		
		for( unsigned char k=0; k<3; k++ )
		{
			
			
			accr[k][0]=adcr[k][1]>>8; 
			accr[k][1]=adcr[k][1];
			
		}
	
	    USART_TXD( 0xAA );
        USART_TXD( 0xFF );
        USART_TXD( g_k1 );
        USART_TXD( g_k2 );
        USART_TXD( g_k3 );
        USART_TXD( g_k4 );
        USART_TXD( 0xFF ); 
		USART_TXD( p_i );
		USART_TXD( p_r );
		USART_TXD( 0xFF );
		USART_TXD( 0x00 );
		USART_TXD( accr[0][0] );
		USART_TXD( accr[0][1] );
		USART_TXD( 0xFF );
		USART_TXD( 0x01 );
		USART_TXD( accr[1][0] );
		USART_TXD( accr[1][1] );
		USART_TXD( 0xFF );
		USART_TXD( 0x02 );
		USART_TXD( accr[2][0] );
		USART_TXD( accr[2][1] );
	
	
	_delay_ms(100);

	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------------------
