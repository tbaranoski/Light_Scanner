/*
 * Project5.c
 *
 * Created: 11/20/2019 9:45:21 PM
 * Author : Taylor Baranoski
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "U0_LCD_Driver.h"	
#include "ADCSampleCode.h"

//Globals
//Mode 0: FTL - Follow The Light
//Mode 1: ATL - Avoid The Light
int mode;
int isrCount = 0;
int ADCValues[10];
int ADCValuesLocal[3];


const int DEGREES_180 = 77; //Max time for a duty cycle that will function

///////////////////////////////////////////////////////////////////////
//Input: n/a
//Return: n/a
//Description: Sets up Servo to fast PWM, prescalor (256), and timer0;
void InitializeServo(){
	
	//Set PB4 as output
	DDRB |= (1 << 4); //Set PB4 to output
	
	//Set Wave generation Mode
	TCCR0A |= (1 << WGM01);
	TCCR0A |= (1 << WGM00);
	
	//Set PreScaling of clock to 256
	TCCR0A |= (1 <<CS02);
	TCCR0A |= (0 <<CS01);
	TCCR0A |= (0 <<CS00);
	
	//Set to Non-Inverted Mode
	TCCR0A |= (1 << COM0A1);
	TCCR0A |= (0 << COM0A0);
	
	//Set Timer Start
	TCNT0 = 0;
}

//Input: n/a
//Return: n/a
//Description: Sets B6 (UP), B7(DOWN), and E3(RIGHT) as inputs
InitializeJoyStick(){
	
	//Initialize UP and DOWN and RIGHT as inputs
	DDRB &= ~0b11000000;
	DDRE &= ~0b00001000;
	
	//Enable Pull UP resistors
	PORTB |= 0b11000000;
	PORTE |= 0b00001000;
}


//Input: n/a
//Return: n/a
//Description: Sets UP LCD and print initial option
InitializeLCD(){	
	LCD_Init();

	//Print Initial Option
	lcd_puts("FTL");
}

//Input: n/a
//Return: n/a
//Description: Sets UP interrupts for RIGHT, UP, and DOWN button presses.
InitializeInterrupts(){

	sei();

	//Enable Interrupts on bits 0-15
	EIMSK |= (1 << PCIE0);	
	EIMSK |= (1 << PCIE1);	

	//Mask pins B6, B7, and E3 to allow Interrupts to trigger upon toggling for these pins
	PCMSK1 |= (1 << PCINT14);
	PCMSK1 |= (1 << PCINT15);
	PCMSK0 |= (1 << PCINT3);
}

//Input: n/a
//Return: n/a
//Description: Performs Full Sweep
int FullSweep(){
	
	int tempADC;
	char tempADCChar[6];
	
	//Turn Interrupts off
	cli();
	
	int pAngle;
	int startTime = 15;
	int i;
	int arrayIndex = 0;

	//Keep Moving Servo 20 degrees for 0 degrees -160 degrees
	for(i = startTime; i <= 77; i += 7){
		
		//Set Output Compare Register to i
		OCR0A = i;
		_delay_ms(3000);
		
		//Get ADC value
		tempADC = ADCAquire();
		
		//Convert that int to char
		sprintf(tempADCChar, "%d", tempADC);
		
		//Print ADC value
		lcd_puts(tempADCChar);
		
		//Save ADC value in int array
		ADCValues[arrayIndex] = tempADC;
		arrayIndex++;
		
	}
	//Move the servo to final position at 180 degrees
	OCR0A = DEGREES_180;
	_delay_ms(3000);
	
	//Get ADC value
	tempADC = ADCAquire();
	
	//Convert that int to char
	sprintf(tempADCChar, "%d", tempADC);
	
	//Print ADC value
	lcd_puts(tempADCChar);
	
	//Save ADC value in int array
	ADCValues[9] = tempADC;
	
	//Enable Interrupts
	sei();
}

//Input: n /a
//Return: n/a
//Description: Prints ADC values at each position
void printADCValues(){
	int tempADC;
	char tempADCChar[6];

	//Print all the ADC[]
	int i;
	for(i = 0; i < 10; i++){
		_delay_ms(6000);
		
		//Print ADC
		//Get ADC value
		tempADC = ADCValues[i];
		
		//Convert that int to char
		sprintf(tempADCChar, "%d", tempADC);
		
		//Print ADC value
		lcd_puts(tempADCChar);
	}
}


//Input: n /a
//Return: return the index of the lowest ADC value in array
//Description: Finds lowest ADC
findLowestADC(){
	int index;
	int value = 2000;
	
	int i;
	//Loop through array
	for(i = 0; i < 10; i++){
		
		if(ADCValues[i] <= value){
			value = ADCValues[i];
			index = i;
		}
	}
	return index;
}

//Input: n /a
//Return: return the index of the highest ADC value in array
//Description: Finds highest ADC
findHighestADC(){
	int index;
	int value = 0;
	
	int i;
	//Loop through array
	for(i = 0; i < 10; i++){
		
		if(ADCValues[i] >= value){
			value = ADCValues[i];
			index = i;
		}
	}
	return index;
}

//Input: index of array
//Return: Angle
//Description:converts index to angle of servo
int IndexToAngle(int index){
	int angle = index *20;
	return angle;
}

//Input: index of angle to start at
//Return: n/a
//Description: Performs local sweep at primary angle
LocalSweep(int primaryIndex){
	int i;
	int arrayIndex = 0;
	int tempADC;
	char tempADCChar[6];
	
	//if primary Index == 0
	if(primaryIndex == 0){
		arrayIndex = 1;
		for(i = 15; i <= 19; i += 4){
			
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 1
	if(primaryIndex == 1){
		for(i = 18; i <= 26; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 2
	if(primaryIndex == 2){
		for(i = 25; i <= 33; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 3
	if(primaryIndex == 3){
		for(i = 32; i <= 40; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 4
	if(primaryIndex == 4){
		for(i = 38; i <= 46; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 5
	if(primaryIndex == 5){
		for(i = 45; i <= 53; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 6
	if(primaryIndex == 6){
		for(i = 52; i <= 60; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 7
	if(primaryIndex == 7){
		for(i = 59; i <= 67; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	//if primary Index == 8
	if(primaryIndex == 8){
		for(i = 66; i <= 74; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
	//if primary Index == 9
	if(primaryIndex == 9){
		for(i = 73; i <= 77; i += 4){
			OCR0A = i;
			_delay_ms(5000);
			
			//Get ADC value
			tempADC = ADCAquire();
			
			//Convert that int to char
			sprintf(tempADCChar, "%d", tempADC);
			
			//Print ADC value
			lcd_puts(tempADCChar);
			
			ADCValuesLocal[arrayIndex] = tempADC;
			arrayIndex++;
		}
	}
	
}

//Input: the primary Index of ADC values
//Return: lowest ADC value's index
//Description:returns the index of lowest local ADC
int FindLowestADCLocal(int primaryIndex){
	int i;
	int num = 1000;
	int index;
	
	//If primary Index is 0
	if(primaryIndex == 0){
		for(i = 1; i < 3; i++){
			if (ADCValuesLocal[i] < num){
				num = ADCValuesLocal[i];
				index = i;
			}
		}
	}
	
	//If primaryIndex is 1-8
	if(primaryIndex >= 1 && primaryIndex <= 8){
		for(i = 0; i < 3; i++){
			if (ADCValuesLocal[i] < num){
				num = ADCValuesLocal[i];
				index = i;
			}
		}
	}
	
	//If primary Index is 9
	if(primaryIndex == 9){
		for(i = 0; i < 2; i++){
			if (ADCValuesLocal[i] < num){
				num = ADCValuesLocal[i];
				index = i;
			}
		}
	}
	
	//return index
	return index;
	
}

//Input: the primary Index of ADC values
//Return: lowest ADC value's index
//Description:returns the index of lowest local ADC
int FindHighestADCLocal(int primaryIndex){
	int i;
	int num = 0;
	int index;
	
	//If primary Index is 0
	if(primaryIndex == 0){
		for(i = 1; i < 3; i++){
			if (ADCValuesLocal[i] > num){
				num = ADCValuesLocal[i];
				index = i;
			}
		}
	}
	
	//If primaryIndex is 1-8
	if(primaryIndex >= 1 && primaryIndex <= 8){
		for(i = 0; i < 3; i++){
			if (ADCValuesLocal[i] > num){
				num = ADCValuesLocal[i];
				index = i;
			}
		}
	}
	
	//If primary Index is 9
	if(primaryIndex == 9){
		for(i = 0; i < 2; i++){
			if (ADCValuesLocal[i] > num){
				num = ADCValuesLocal[i];
				index = i;
			}
		}
	}
	
	//return index
	return index;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int main(void)
{
	InitializeServo();
	//Initial Servo Value
	OCR0A = 15;
	InitializeJoyStick();
	InitializeLCD();
	InitializeInterrupts();
	ADCSetup();

	//Initialize Modes
	//Mode 0: FTL - Follow The Light
	//Mode 1: ATL - Avoid The Light
	mode = 0;
	
	//Infinite Loop
	while(1 == 1){
			
	}
	return 0;
}

//Input: primaryAngle form full sweep and index of local sweep value
//Return: final angle
//Description: Finds the exact angle by taking the primary angle and index of local values.
int IndexToLocalAngle(int primaryAngle, int ADCIndex){
	
	int temp;
	
	//If -10 deg
	if(ADCIndex == 0){
		temp = -10;
	}
	
	//If 0 deg
	if(ADCIndex == 1){
		temp = 0;
	}
	
	//If +10 deg
	if(ADCIndex == 2){
		temp = 10;
	}
	
	//return primary angle
	return (primaryAngle + temp);
}

//////////////////////////////////////////////////////////////////////
//Input: Occurs when E3 toggles (RIGHT)
//Return: n/a
//Description: Performs appropriate mode (FTL or ATL).
ISR(PCINT0_vect){
	int primaryAngle;
	int localAngle;
	int ADCIndex;
	int primaryIndex;
	char primaryAngleChar[6];
	char localAngleChar[6];
	
	isrCount++;
	
	//Every other ISR cal call this
	if((isrCount % 2) == 0){
		//Call Full Sweep
		FullSweep();
		
		//Test Print all the ADC[]
		//printADCValues();
		
		//IF FTL find lowest ADC
		if(mode == 0){
			primaryIndex = findLowestADC();
			primaryAngle = IndexToAngle(primaryIndex);
			
			//Print Primary Angle to screen
	
			sprintf(primaryAngleChar, "%d", primaryAngle);
			LCD_Init();
			_delay_ms(5000);
			lcd_puts(primaryAngleChar);
			_delay_ms(7000);
			
			////////////////////////////////////////////////////////////
			//Perform Local Sweep
			LocalSweep(primaryIndex);
			ADCIndex = FindLowestADCLocal(primaryIndex);
			localAngle = IndexToLocalAngle(primaryAngle, ADCIndex);
			
			//Print Primary Angle to screen
			
			sprintf(localAngleChar, "%d", localAngle);
			LCD_Init();
			_delay_ms(5000);
			lcd_puts(localAngleChar);
			_delay_ms(7000);
		}
		
		//If ATL find highest ADC
		if(mode == 1){
			primaryIndex = findHighestADC();
			primaryAngle = IndexToAngle(primaryIndex);
			
			//Print Primary Angle to screen
			
			sprintf(primaryAngleChar, "%d", primaryAngle);
			LCD_Init();
			_delay_ms(5000);
			lcd_puts(primaryAngleChar);
			_delay_ms(7000);
			
			////////////////////////////////////////////////////////////
			//Perform Local Sweep
			LocalSweep(primaryIndex);
			ADCIndex = FindHighestADCLocal(primaryIndex);
			localAngle = IndexToLocalAngle(primaryAngle, ADCIndex);
			
			//Print Primary Angle to screen
			
			sprintf(localAngleChar, "%d", localAngle);
			LCD_Init();
			_delay_ms(5000);
			lcd_puts(localAngleChar);
			_delay_ms(7000);
			
		}
		
	}
				
}

//Input: Occurs when B6 or B7 toggles. (UP/DOWN)
//Return: n/a
//Description: Switches between Follow the Light and Avoid the Light Modes
ISR(PCINT1_vect){
	isrCount++;
	
	if((isrCount % 2) == 0){
	
		//Toggle the mode
		if(mode == 0){
			mode = 1;
			lcd_puts("ATL");
		}
		else{
			mode = 0;
			lcd_puts("FTL");
		}
	} //end of if
}
