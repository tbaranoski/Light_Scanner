/*
 * ADCSampleCode.h
 *
 * Created: 11/21/2019 9:54:47 PM
 *  Author: Taylor Baranoski
 */ 
#include <avr/io.h>


#ifndef ADCSAMPLECODE_H_
#define ADCSAMPLECODE_H_


void ADCSetup();
void ADCStartConversion();
void ADCClearConversionFlag();
int ADCIsConversionCompleteFlagSet();
uint16_t ADCGet();
int ADCAquire();

#endif /* ADCSAMPLECODE_H_ */