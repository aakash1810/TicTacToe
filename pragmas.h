





//mcc18 -p18f45k22 --help-config<cr> 

#pragma config FOSC = INTIO67
#pragma config PLLCFG = OFF
#pragma config PRICLKEN = ON
#pragma config FCMEN = OFF
#pragma config IESO = OFF

#pragma config PWRTEN = OFF //#pragma config PWRTEN = ON 		// blanked for PICKIT2 debug function
#pragma config BOREN = ON
#pragma config BORV = 285 
 
#pragma config WDTEN = OFF

#pragma config PBADEN = OFF  //PORT B digital on reset

#pragma config LVP = OFF
#pragma config MCLRE = EXTMCLR

