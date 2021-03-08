/*_____ I N C L U D E S ____________________________________________________*/
#include "ML51.h"

#include	"project_config.h"



/*_____ D E C L A R A T I O N S ____________________________________________*/
volatile uint8_t u8TH0_Tmp = 0;
volatile uint8_t u8TL0_Tmp = 0;

//UART 0
//bit BIT_TMP;
//bit BIT_UART;
//bit uart0_receive_flag=0;
//unsigned char uart0_receive_data;

/*_____ D E F I N I T I O N S ______________________________________________*/
#define SYS_CLOCK 								(24000000ul)

/*_____ M A C R O S ________________________________________________________*/

volatile uint32_t BitFlag = 0;

uint32_t counter_tick = 0;

/*_____ F U N C T I O N S __________________________________________________*/


void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , Enable);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("compare_buffer finish \r\n");	
		set_flag(flag_error , Disable);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void delay(uint16_t dly)
{
/*
	delay(100) : 14.84 us
	delay(200) : 29.37 us
	delay(300) : 43.97 us
	delay(400) : 58.5 us	
	delay(500) : 73.13 us	
	
	delay(1500) : 0.218 ms (218 us)
	delay(2000) : 0.291 ms (291 us)	
*/

	while( dly--);
}


void send_UARTString(uint8_t* Data)
{
	#if 1
	uint16_t i = 0;

	while (Data[i] != '\0')
	{
		#if 1
		SBUF = Data[i++];
		#else
		UART_Send_Data(UART0,Data[i++]);		
		#endif
	}

	#endif

	#if 0
	uint16_t i = 0;
	
	for(i = 0;i< (strlen(Data)) ;i++ )
	{
		UART_Send_Data(UART0,Data[i]);
	}
	#endif

	#if 0
    while(*Data)  
    {  
        UART_Send_Data(UART0, (unsigned char) *Data++);  
    } 
	#endif
}

void send_UARTASCII(uint16_t Temp)
{
    uint8_t print_buf[16];
    uint16_t i = 15, j;

    *(print_buf + i) = '\0';
    j = (uint16_t)Temp >> 31;
    if(j)
        (uint16_t) Temp = ~(uint16_t)Temp + 1;
    do
    {
        i--;
        *(print_buf + i) = '0' + (uint16_t)Temp % 10;
        (uint16_t)Temp = (uint16_t)Temp / 10;
    }
    while((uint16_t)Temp != 0);
    if(j)
    {
        i--;
        *(print_buf + i) = '-';
    }
    send_UARTString(print_buf + i);
}


void GPIO_Init(void)
{
	P03_PUSHPULL_MODE;		

//	P30_PUSHPULL_MODE;	
}

// 0x7C00 : 31K
void Get_FlashAddr_Vaule(unsigned int u16_addr)
{
	#define SN_Size (8)	
	
    unsigned char u8Count;
	unsigned char xdata SNBuffer[SN_Size];
	
    set_CHPCON_IAPEN;
    IAPAL = LOBYTE(u16_addr);
    IAPAH = HIBYTE(u16_addr);
    IAPCN = BYTE_READ_APROM;
    for(u8Count = 0 ; u8Count < SN_Size; u8Count++)
    {   
        IAPFD = 0x00;
        set_IAPTRG_IAPGO;
        SNBuffer[u8Count] = IAPFD ;
        IAPAL++;
    } 
    clr_CHPCON_IAPEN;
	
	printf("Value (0x%BX , 0x%BX) : " , LOBYTE(u16_addr) , HIBYTE(u16_addr) );

    for(u8Count = 0 ; u8Count < (SN_Size -1) ; u8Count++)
	{
		printf(" 0x%BX" , SNBuffer[u8Count]);
	}
	printf(" 0x%BX \r\n" , SNBuffer[SN_Size -1]);	

}

void Timer0_IRQHandler(void)
{

	
	tick_counter();

	if ((get_tick() % 1000) == 0)
	{
//		P03 ^= 1;
		Get_FlashAddr_Vaule(0x7C00);

	}

	if ((get_tick() % 50) == 0)
	{

	}		
}

void Timer0_ISR(void) interrupt 1        // Vector @  0x0B
{
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    clr_TCON_TF0;
	
	Timer0_IRQHandler();
}

void Timer0_Init(void)
{
	uint16_t res = 0;

	ENABLE_TIMER0_MODE1;
    TIMER0_FSYS_DIV12;
	
	u8TH0_Tmp = HIBYTE(TIMER_DIV12_VALUE_1ms_FOSC_240000);
	u8TL0_Tmp = LOBYTE(TIMER_DIV12_VALUE_1ms_FOSC_240000); 

    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;

    ENABLE_TIMER0_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR0;                                  //Timer0 run
}


//void Serial_ISR (void) interrupt 4 
//{
//    _push_(SFRS);

//    if (RI)
//    {   
//      uart0_receive_flag = 1;
//      uart0_receive_data = SBUF;
//      clr_SCON_RI;                                         // Clear RI (Receive Interrupt).
//    }
//    if  (TI)
//    {
//      if(!BIT_UART)
//      {
//          TI = 0;
//      }
//    }

//    _pop_(SFRS);	
//}


void UART0_Init(void)
{
	MFP_P31_UART0_TXD;                              // UART0 TXD use P1.6
	P31_QUASI_MODE;                                  // set P1.6 as Quasi mode for UART0 trasnfer
	UART_Open(SYS_CLOCK,UART0_Timer3,115200);        // Open UART0 use timer1 as baudrate generate and baud rate = 115200
	ENABLE_UART0_PRINTF;


	printf("UART0_Init\r\n");
}

void SYS_Init(void)
{
//	FsysSelect(FSYS_HIRC);

    ALL_GPIO_QUASI_MODE;
//    ENABLE_GLOBAL_INTERRUPT;                // global enable bit	
}

void main (void) 
{
    SYS_Init();

    UART0_Init();
	GPIO_Init();
	Timer0_Init();


		
    while(1)
    {

		
    }
}



