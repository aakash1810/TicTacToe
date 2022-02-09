/**************************************************************************
********* Pic code **********************************************************		
**************************************************************************/

		//>>> Lab - 4 >>>
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	File Name:	ELNC6007ASLab4.c
	Author:		ASolanki
	Date:		10/12/2020
	Modified:	Name or None
	c Fanshawe College, 2020

	Description: *** Creating a game board for TicTacToe using multiplexing, LEDS on breadborad,
					 changing the status, reseting game board, reciving serial string from mbed 
					 and sending through serial port 2.***
	
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

// Preprocessor >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// Libraries >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include "pragmas.h"
#include <p18f45k22.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <usart.h>

// Constants >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define TRUE		1
#define	FALSE		0
#define ROW		    3
#define COL		    3
#define MAXTURN    	9
#define SIZE		2
#define BUFSIZE		30
#define MYADDY		202
#define CONTROLLER	1 
#define MAXTURNTIME	30
#define PRINTTIME	6
#define RESET		0
#define	ENABLED		1
#define	LOW		    0
#define	HIGH		1
#define TIMER INTCONbits.TMR0IF  
#define ONESEC 		0xBDC 
#define RC2FLAG	PIR3bits.RC2IF
#define RC2BUFF	RCREG2
#define TOKENSIZE	    10
#define CMDSTMT     	0
#define ADDYTO      	1
#define ADDYFROM    	2
#define GBROW       	3
#define GBCOL       	4
#define GBCHAR      	5
#define BUFFER	    	30

#define LED1ON TRISCbits.TRISC1=0		// for led 1[0][0]
#define LED1OFF TRISCbits.TRISC1=1
#define LED1RON	LATCbits.LATC1=0
#define LED1GON	LATCbits.LATC1=1

#define LED2ON TRISCbits.TRISC2=0		// for led 2 [1][0]
#define LED2OFF TRISCbits.TRISC2=1
#define LED2RON	LATCbits.LATC2=0
#define LED2GON	LATCbits.LATC2=1

#define LED3ON TRISCbits.TRISC3=0		// for led 3 [2][0] 
#define LED3OFF TRISCbits.TRISC3=1
#define LED3RON	LATCbits.LATC3=0
#define LED3GON	LATCbits.LATC3=1

#define LED4ON TRISDbits.TRISD0=0		// for led 4 [0][1]
#define LED4OFF TRISDbits.TRISD0=1
#define LED4RON	LATDbits.LATD0=0
#define LED4GON	LATDbits.LATD0=1

#define LED5ON TRISDbits.TRISD1=0		// for led 5 [1][1]
#define LED5OFF TRISDbits.TRISD1=1
#define LED5RON	LATDbits.LATD1=0
#define LED5GON	LATDbits.LATD1=1

#define LED6ON TRISDbits.TRISD2=0		// for led 6 [2][1]
#define LED6OFF TRISDbits.TRISD2=1
#define LED6RON	LATDbits.LATD2=0
#define LED6GON	LATDbits.LATD2=1

#define LED7ON TRISDbits.TRISD3=0 		// for led 7 [0][2]
#define LED7OFF TRIDCbits.TRISD3=1
#define LED7RON	LATDbits.LATD3=0
#define LED7GON	LATDbits.LATD3=1

#define LED8ON TRISCbits.TRISC4=0		// for led 8 [1][2]
#define LED8OFF TRISCbits.TRISC4=1
#define LED8RON	LATCbits.LATC4=0
#define LED8GON	LATCbits.LATC4=1

#define LED9ON TRISCbits.TRISC5=0		// for led 9 [2][2]
#define LED9OFF TRISCbits.TRISC5=1
#define LED9RON	LATCbits.LATC5=0
#define LED9GON	LATCbits.LATC5=1

// Global Variables >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

char plypmt[] = {"PLYPMT"};
char gamrst[] = {"GAMRST"};
char *tokens[TOKENSIZE];
char buffer[BUFFER];
char sentenceReady=FALSE;
char *ptr = buffer;
char row = 0;
char col = 0;
char pbState = TRUE;
char lastState = FALSE;
char press = 0;
int size = 0;
char hold = 0;
char reset = 0;
char sec = 0, min = 1;
unsigned char turnTime = MAXTURNTIME;
char place = 1;
char buf[50];
char printSen = FALSE;
char senPrint = FALSE;
int printTime = 0;

enum gameStatus{Ready, Playing, Won,Draw, Reset};

typedef struct gameBoard
{
	char ttt[3][3];
	char player;
	unsigned char turn;
	char winFlag;
}gameBoard_t;

typedef struct tttSys
{
	unsigned char address;
	gameBoard_t game;
	unsigned char select;
	unsigned char time[SIZE];
	enum gameStatus status; 
}tttSys_t;

tttSys_t ttt202;

// Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void isr();
#pragma code vectorHighInterrupt = 0x008
void vectorHighInterrupt()
{
	_asm
	GOTO isr
	_endasm
}
#pragma code

/*** set_osc_p18f45k22_4MHz: ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Sets the internal Oscillator of the Pic 18F45K22 to 4MHz.
Input: 		None
Returns:	None
**********************************************************************************/
void set_osc_p18f45k22_4MHz(void)
{
	OSCCON =  0x52;						// Sleep on slp cmd, HFINT 4MHz, INT OSC Blk
	OSCCON2 = 0x04;						// PLL No, CLK from OSC, MF off, Sec OSC off, Pri OSC
	OSCTUNE = 0x80;						// PLL disabled, Default factory freq tuning
	
	while (OSCCONbits.HFIOFS != 1); 	// wait for osc to become stable
}
//eo: set_osc_p18f45k22_4MHz:: ***************************************************

/*** portConfig ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Configering the input port A and output ports C and D.
Input: 		None
Returns:	None
**********************************************************************************/
void portConfig(void)
{
	// port A (RA4, RA5, RA6, RA7) as an input
	
	ANSELA = 0x00;		// port A as digital
	LATA = 0x00;		// set pushbuttons off on initialization
	TRISA = 0xF0;		// set upper bits as output 1111 0000

	// port C (RC1, RC2, RC3, RC4, RC5) as a digital output

	ANSELC = 0x00;		// port C as digital
	LATC = 0x00;		// set LEDs off on initialization
	TRISC = 0xFF;		//  Set LEDs in high impedence mode

	// port D (RD0, RD1, RD2, RD3) as a digital output

	ANSELD = 0x00;		//  port D as digital
	LATD = 0x00;		// set LEDs off on initialization
	TRISD = 0xFF;		// Set LEDs in high impedence mode
}
//eo: portConfig:: ***************************************************

/*** configSerialPort1 ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Configaring serial port for print the game board.
Input: 		None
Returns:	None
**********************************************************************************/
void configSerialPort1(void)
{
	SPBRG1  = 0x19;		// Sync = 0, BRGH = 1. BRG16 = 0, baudrate 9600, Fosc = 4Mhz, Decimal 25
	TXSTA1  = 0x26;		// Transmit status 0010 0110
	RCSTA1  = 0x90;		// Serial port enable, enable reciver 1001 0000 
	BAUDCON1= 0x40;		// Baudrate at 9600
	

}
//eo: configSerialPort1:: ***************************************************

/*** configSerialPort2 ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Configaring serial port 2 for the sending and receiving the string.
Input: 		None
Returns:	None
**********************************************************************************/
void configSerialPort2(void)
{
	SPBRG2  = 0x19;		// Sync = 0, BRGH = 1. BRG16 = 0, baudrate 9600, Fosc = 4Mhz, Decimal 25
	TXSTA2  = 0x26;		// Transmit status 0010 0110
	RCSTA2  = 0x90;		// Serial port enable, enable reciver 1001 0000 
	BAUDCON2= 0x40;		// Baudrate at 9600
}
//eo: configSerialPort2:: ***************************************************

/*** resetTMRo ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Configering Timer 0 to 1 second refresh.
Input: 		None
Returns:	None
**********************************************************************************/
void resetTMR0(int count)
{
	TIMER=FALSE;		
	TMR0H=count>>8;		//  upper nibble 0x0B
	TMR0L=count;		// Lower nibble 0xDC
}
void configTMR0(int count)
{
	resetTMR0(ONESEC);
	T0CON=0x93;			// TMR0 is on, PSV = 16
}
//eo: resetTMR0:: ***************************************************

/*** tttSys ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Defining the values of TicTacToe data structure.
Input: 		None
Returns:	None
**********************************************************************************/
void tttSys(void)
{
	ttt202.address = 202;
	for (row = 0; row < ROW; row++)
	{
		for (col = 0; col < COL; col++)
		{
			ttt202.game.ttt[row][col] = 0;
		}
	}
	ttt202.game.player = 'X';
	ttt202.game.turn = FALSE;
	ttt202.game.winFlag = FALSE;
	ttt202.select = 0;
	for(size = 0; size <= SIZE; size++)
	{
		ttt202.time[size] = 0x00;
	}
	ttt202.status = Ready;
	row=0;
	col=0;
}
//eo: tttSys:: ***************************************************

/*** calculateChecksum ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Oprating the LEDs to turn on for X or O
Input: 		charecter pointer array.
Returns:	returns checksm value.
**********************************************************************************/
char calculateChecksum(char *ptr)
{
	char checksum =0;
	while(*ptr)
	{
		checksum += *ptr;
		ptr++;
	}
	return checksum;
}//eo: calculateChecksum:: ******************************************

/*** gameEnd ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Generating the game end sentence for mbed.
Input: 		None
Returns:	None
**********************************************************************************/
void gameEnd(void)
{
	sprintf(buf, "$GAMEND,%i,%i,%c,%04i\0", CONTROLLER, MYADDY, ttt202.game.player, printTime);
	sprintf(buf, "%s,%i^", buf, calculateChecksum(buf));
}
//eo: gameEnd:: ******************************************

/*** updateDisplay ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Creates a game board for TicTacToe.
Input: 		None
Returns:	None
**********************************************************************************/
void updateDisplay(void)
{
	char stop=TRUE;
	
	printf("\033[2J\033[H");
	
	printf("\t\t------Tic Tac TOE--------\n\r");
	if(ttt202.status == Ready)
	{
		printf("TicTacToe%03d\t\tGame Time:%02d:%02d\t\tStatus:Ready\n\r",ttt202.address, ttt202.time[ 1 ], ttt202.time[ 0 ]);
	}
	if(ttt202.status == Playing)
	{
		printf("TicTacToe%03d\t\tGame Time:%02d:%02d\t\tStatus:Playing\n\r",ttt202.address, ttt202.time[ 1 ], ttt202.time[ 0 ]);
	}
	if(ttt202.status == Won)
	{
		printf("TicTacToe%03d\t\tGame Time:%02d:%02d\t\tStatus:Won\n\r",ttt202.address, ttt202.time[ 1 ], ttt202.time[ 0 ]);
	}
	if(ttt202.status == Draw)
	{
		printf("TicTacToe%03d\t\tGame Time:%02d:%02d\t\tStatus:Draw\n\r",ttt202.address, ttt202.time[ 1 ], ttt202.time[ 0 ]);
	}
	if(ttt202.status == Reset)
	{
		printf("TicTacToe%03d\t\tGame Time:%02d:%02d\t\tStatus:Reset\n\r",ttt202.address, ttt202.time[ 1 ], ttt202.time[ 0 ]);
	}	
	printf("Player:%c\t\tTurn Count:%d\n\r",ttt202.game.player, ttt202.game.turn);
	if(press == 0)
	{ 
		printf("Row\n\r");
	}
	else if(press==1)
	{
		printf("Col\n\r");
	}
	//printf("Row(or Column)\n\r");
	printf("Select:%d\t\tTurn Time:%02d\n\r",ttt202.select, turnTime);
	printf("row=%d\tcol=%d\n\r",row,col);
	printf("\t\t\t\tcolumn\n\r");
	printf("\n\t\t\t    0\t  1\t 2\n\n\r");
	printf("\n\t\t\t 0   %c   |   %c |  %c  \n\r", ttt202.game.ttt[0][0], ttt202.game.ttt[0][1], ttt202.game.ttt[0][2]);
	printf("\n\t\tR\t --------------------\n\r");
	printf("\n\t\tO\t1   %c   |   %c |  %c  \n\r", ttt202.game.ttt[1][0], ttt202.game.ttt[1][1], ttt202.game.ttt[1][2]);
	printf("\n\t\tW\t --------------------\n\r");
	printf("\n\t\t\t 2   %c   |   %c |  %c  \n\r",ttt202.game.ttt[2][0], ttt202.game.ttt[2][1], ttt202.game.ttt[2][2]);
	

	printTime = ttt202.time[1];
	printTime = ttt202.time[1] * 100;
	printTime += ttt202.time[0];
	//printf("%04i", printTime);

	if(ttt202.game.winFlag && ttt202.game.player == 'X' && stop)
	{	
		printf("Player O won\n\r");
		printf("To Restart game hold the Enter and Mode for 3 seconds\n\r");
		gameEnd();
		puts2USART(buf);
		stop=0;
	}
	else if (ttt202.game.winFlag && ttt202.game.player == 'O' && stop)
	{
		printf("Player X won\n\r");
		printf("To Restart game hold the Enter and Mode for 3 seconds\n\r");
		gameEnd();
		puts2USART(buf);
		stop=0;
	}
	if(ttt202.game.turn == MAXTURN && !ttt202.game.winFlag && stop)
	{
		printf("It's a draw\n\r");
		printf("To Restart game hold the Enter and Mode for 3 seconds\n\r");
		ttt202.game.player='N';
		ttt202.status = Draw;
		gameEnd();
		press = 3;
		puts2USART(buf);
		stop=0;
	} 
}
//eo: updateDisplay:: ***************************************************

/*--- sentence ------------------------------------------------------------
Author:		ASolanki
Date:		19/11/2020
Modified:	Name or None
Desc:		printing the string after every placement.
Input: 		Type and purpose of input arguments
Returns:	Type and purpose of returning argument
------------------------------------------------------------------------------*/
void sentence(void)
{
	sprintf(buf, "$PLYPMT,%i,%i,%i,%i,%c\0", CONTROLLER, MYADDY, row, col, ttt202.game.player);
	sprintf(buf, "%s,%i^\0", buf, calculateChecksum(buf));

}// eo sentence::  **************************************************



// --- Win Check ---
/*--- winCheck ------------------------------------------------------------
Author:		ASolanki
Date:		19/11/2020
Modified:	Name or None
Desc:		Checking whether someone is winning or not.
Input: 		Type and purpose of input arguments
Returns:	Type and purpose of returning argument
------------------------------------------------------------------------------*/
void winCheck(void)
{
	if ((ttt202.game.ttt[0][0] & ttt202.game.ttt[0][1] & ttt202.game.ttt[0][2]) == 'X' ||
		(ttt202.game.ttt[1][0] & ttt202.game.ttt[1][1] & ttt202.game.ttt[1][2]) == 'X' ||
		(ttt202.game.ttt[2][0] & ttt202.game.ttt[2][1] & ttt202.game.ttt[2][2]) == 'X' ||
		(ttt202.game.ttt[0][0] & ttt202.game.ttt[1][0] & ttt202.game.ttt[2][0]) == 'X' ||
		(ttt202.game.ttt[0][1] & ttt202.game.ttt[1][1] & ttt202.game.ttt[2][1]) == 'X' ||
		(ttt202.game.ttt[0][2] & ttt202.game.ttt[1][2] & ttt202.game.ttt[2][2]) == 'X' ||
		(ttt202.game.ttt[0][0] & ttt202.game.ttt[1][1] & ttt202.game.ttt[2][2]) == 'X' ||
		(ttt202.game.ttt[0][2] & ttt202.game.ttt[1][1] & ttt202.game.ttt[2][0]) == 'X' ||
		(ttt202.game.ttt[0][0] & ttt202.game.ttt[0][1] & ttt202.game.ttt[0][2]) == 'O' ||
		(ttt202.game.ttt[1][0] & ttt202.game.ttt[1][1] & ttt202.game.ttt[1][2]) == 'O' ||
		(ttt202.game.ttt[2][0] & ttt202.game.ttt[2][1] & ttt202.game.ttt[2][2]) == 'O' ||
		(ttt202.game.ttt[0][0] & ttt202.game.ttt[1][0] & ttt202.game.ttt[2][0]) == 'O' ||
		(ttt202.game.ttt[0][1] & ttt202.game.ttt[1][1] & ttt202.game.ttt[2][1]) == 'O' ||
		(ttt202.game.ttt[0][2] & ttt202.game.ttt[1][2] & ttt202.game.ttt[2][2]) == 'O' ||
		(ttt202.game.ttt[0][0] & ttt202.game.ttt[1][1] & ttt202.game.ttt[2][2]) == 'O' ||
		(ttt202.game.ttt[0][2] & ttt202.game.ttt[1][1] & ttt202.game.ttt[2][0]) == 'O')
	{
		ttt202.game.winFlag = TRUE;
		ttt202.status = Won;
		printSen = FALSE;		
	}
	else
	{
		ttt202.game.winFlag == FALSE;
		//ttt202.status = Draw;
	}
}	
// eo winCheck::  **************************************************

/*** ledStart ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Oprating the LEDs to turn on for X or O
Input: 		None
Returns:	None
**********************************************************************************/
void ledStart(void)
{
	if(ttt202.game.ttt[0][0])				// led1 on for [0][0]
	{
		if(ttt202.game.ttt[0][0] == 'X')
		{
			LED1ON;
			LED1RON;
		}
		else if(ttt202.game.ttt[0][0] == 'O')
		{
			LED1ON;
			LED1GON;
		}
	}
	if(ttt202.game.ttt[1][0])				// led2 on for [1][0]
	{
		if(ttt202.game.ttt[1][0] == 'X')
		{
			LED2ON;
			LED2RON;
		}
		else if(ttt202.game.ttt[1][0] == 'O')
		{
			LED2ON;
			LED2GON;
		}
	}
	if(ttt202.game.ttt[2][0])				// led3 on for [2][0]
	{
		if(ttt202.game.ttt[2][0] == 'X')
		{
			LED3ON;
			LED3RON;
		}
		else if(ttt202.game.ttt[2][0] == 'O')
		{
			LED3ON;
			LED3GON;
		}
	}
	if(ttt202.game.ttt[0][1])				// led4 on for [0][1]
	{
		if(ttt202.game.ttt[0][1] == 'X')
		{
			LED4ON;
			LED4RON;
		}
		else if(ttt202.game.ttt[0][1] == 'O')
		{
			LED4ON;
			LED4GON;
		}
	}
	if(ttt202.game.ttt[1][1])				// led5 on for [1][1]
	{
		if(ttt202.game.ttt[1][1] == 'X')
		{
			LED5ON;
			LED5RON;
		}
		else if(ttt202.game.ttt[1][1] == 'O')
		{
			LED5ON;
			LED5GON;
		}
	}
	if(ttt202.game.ttt[2][1])				// led6 on for [2][1]
	{
		if(ttt202.game.ttt[2][1] == 'X')
		{
			LED6ON;
			LED6RON;
		}
		else if(ttt202.game.ttt[2][1] == 'O')
		{
			LED6ON;
			LED6GON;
		}
	}
	if(ttt202.game.ttt[0][2])				// led7 on for [0][2]
	{
		if(ttt202.game.ttt[0][2] == 'X')
		{
			LED7ON;
			LED7RON;
		}
		else if(ttt202.game.ttt[0][2] == 'O')
		{
			LED7ON;
			LED7GON;
		}
	}
	if(ttt202.game.ttt[1][2])				// led8 on for [1][2] 
	{
		if(ttt202.game.ttt[1][2] == 'X')
		{
			LED8ON;
			LED8RON;
		}
		else if(ttt202.game.ttt[1][2] == 'O')
		{
			LED8ON;
			LED8GON;
		}
	}
	if(ttt202.game.ttt[2][2])				// led9 on for [2][2]
	{
		if(ttt202.game.ttt[2][2] == 'X')
		{
			LED9ON;
			LED9RON;
		}
		else if(ttt202.game.ttt[2][2] == 'O')
		{
			LED9ON;
			LED9GON;
		}
	}
}
//eo: ledStart:: ***************************************************

/*** autoPlacePlayer ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Oprating the LEDs to turn on for X or O
Input: 		None
Returns:	None
**********************************************************************************/
void autoPlacePlayer(void)
{
	for (row = 0; row < ROW && place == 1; row++)
	{
		for (col = 0; col < COL && place == 1; col++)
		{
			
			if (ttt202.game.ttt[row][col] != 'X' && ttt202.game.ttt[row][col] != 'O' && !ttt202.game.winFlag)  // check if the location is occupied or not
			{
				if (ttt202.game.player == 'X')                // if player is true than takes X
				{
					ttt202.game.ttt[row][col] = 'X';
					sentence();
					ttt202.game.player='O';
					ledStart();
				}
				else if(ttt202.game.player == 'O')           // if player is false than takes O
				{
					ttt202.game.ttt[row][col] = 'O';
					sentence();
					ttt202.game.player='X';
					ledStart();
				}
				winCheck();                                  // check anyone is winning or not
				ttt202.game.turn++;							 // increament the turn count
				place = 0;
			}
		}
	}
	puts2USART(buf);
	row--;
	col--;
	place = 1;
	turnTime = MAXTURNTIME;
	printSen = TRUE;
}
//eo: autoPlacePlayer:: ****************************************** 

/*** gameReset ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Reset the game board.
Input: 		None
Returns:	None
**********************************************************************************/
void gameReset(void)
{
	ttt202.status = Reset;
	tttSys();
	TRISC = 0xFF;
	TRISD = 0xFF;
	turnTime = MAXTURNTIME;
	printSen = TRUE;
}
// eo gameReset::*****************************************************

#pragma interrupt isr 
/*** configInterrupt ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Configering interrupt to recieve string from mbed.
Input: 		None
Returns:	None
**********************************************************************************/
void configInterrupt(void)
{
	PIR3bits.RC2IF = RESET;
	PIE3bits.RC2IE = ENABLED;
	//IPR3bits.RC2IP = LOW;

	//INTCONbits.TMR0IF = RESET;
	//INTCONbits.TMR0IE = ENABLED;
	//INTCON2bits.TMR0IP = LOW;
	
	RCONbits.IPEN = 0;  // Priority levels off.

	INTCON |= 0xC0;
}
// eo configInterrupt::*****************************************************

/*** getAByte ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Collecting the sentence from mbed.
Input: 		None
Returns:	None
**********************************************************************************/
void getAByte(char *ptr)
{
    char hold=0;
    while(hold!='^')
    {
        if(RC2FLAG)
        {
            hold=RC2BUFF;
	    if(hold=='$')
            {
	        ptr=buffer;
	    }
            *ptr = hold;
            ptr++;
        }
    }
    *ptr=0x00;
    sentenceReady=TRUE;
}
// eo getAByte::*****************************************************


/*** validateSentence ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Validating the sentence and removing the checksum.
Input: 		None
Returns:	None
**********************************************************************************/
char validateSentence(char *ptr)
{
    char checksum = 0, recvdcs = 0;
    unsigned char count = strlen(ptr);
    char checksumFlag = FALSE;
    while(checksumFlag == FALSE)
    {
        if(*(ptr+count) == '^')
        {
            *(ptr+count) = 0;    
        }    
        if(*(ptr+count) == ',')
        {
            *(ptr+count) = 0;
            recvdcs = atoi(ptr+count+1);
            checksumFlag = TRUE;    
        }
        count--;
    } 
    checksum = calculateChecksum(ptr);
    if(checksum == recvdcs)
    {
        return TRUE;    
    }   
    else
    {
        return FALSE;    
    }
}
// eo validateSentance::*****************************************************


/*** parseSentence ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Dividing the string into pieces.
Input: 		None
Returns:	None
**********************************************************************************/
void parseSentence(char *ptr)
{
	unsigned char tokenCounter = 0;
	while(*ptr)
	{
		if(*ptr == '$' || *ptr == ',')
		{
			*ptr = 0x00;
			tokens[tokenCounter] = ptr+1;
			tokenCounter++;
		}
		ptr++;
	}
}
//eo: parseSentence:: ***************************************************

/*** executeSentence ******************************************************
Author:		ASolanki
Date:		10/12/2020		
Modified:	ASolanki
Desc:		Executing the sentence for the placement.
Input: 		None
Returns:	None
**********************************************************************************/
void executeSentence()
{
    if(atoi(tokens[ADDYTO]) == MYADDY) 
    {
        if(atoi(tokens[ADDYFROM]) == CONTROLLER)
        {
            if(strcmp(tokens[CMDSTMT],plypmt) == 44)
            {
                ttt202.game.ttt[atoi(tokens[GBROW])][atoi(tokens[GBCOL])] = *tokens[GBCHAR]; 
                ttt202.game.turn++;
		winCheck();
		if(*tokens[GBCHAR] == 'X')
		{
			ttt202.game.player = 'O';
		}
		else
		{
			ttt202.game.player = 'X';
		}
            }
	    if(strcmp(tokens[CMDSTMT],gamrst) == 44)
	    {
		gameReset();
	    }    
        }    
    }   
}
//eo: executeSentence:: ***************************************************

#pragma interrupt isr

/*** isr ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Oprating the LEDs to turn on for X or O
Input: 		None
Returns:	None
**********************************************************************************/
void isr()
{
	if(RC2FLAG)
	{
		getAByte(buffer);
	}
	
	INTCON |= 0xC0;
}
//eo: isr:: ***************************************************

/*** intializeSystem ******************************************************
Author:		ASolanki
Date:		19/11/2020		
Modified:	ASolanki
Desc:		Initialize the TICTacToe system
Input: 		None
Returns:	None
**********************************************************************************/
void initializeSystem(void)
{
	set_osc_p18f45k22_4MHz();
	portConfig();
	configSerialPort1();
	configSerialPort2();
	configTMR0(ONESEC);
	configInterrupt();
}	
//eo: initializeSystem:: ***************************************************

/*>>> MAIN: FUNCTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
void main(void)
{
	initializeSystem();
	tttSys();
	printf("System Ready...\n\r");
	while(1)
	{
		if(TIMER)
		{
			resetTMR0(ONESEC);
			if(!ttt202.game.winFlag && ttt202.game.turn != MAXTURN)
			{
				ttt202.status = Playing;	
				ttt202.time[sec]++;
				if(ttt202.time[sec] >= 60)
				{
					ttt202.time[min]++;
					ttt202.time[sec] = 0;
				}
				turnTime--;
				if(turnTime == 0)
				{
					turnTime = MAXTURNTIME;
					autoPlacePlayer();
				}
				if(printSen == TRUE)
				{		
					senPrint++;
					if(senPrint == PRINTTIME)
					{
						senPrint = 0;
						printSen = FALSE;
					}
				}		
			}
			updateDisplay();
		}// eo TIMER
		if(sentenceReady)
		{
			sentenceReady = FALSE;
			if(validateSentence)
			{
				parseSentence(buffer);
				executeSentence();
				ledStart();
				turnTime = MAXTURNTIME;
			}
		}// eo sentenceReady
		
	}// eo while
}// eo main::
