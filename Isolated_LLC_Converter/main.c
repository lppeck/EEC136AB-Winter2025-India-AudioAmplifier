/* 
© [2025] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
 */
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/adc/adc_interface.h"
#include "mcc_generated_files/adc/adc1.h"
#include "mcc_generated_files/pwm_hs/pwm_hs_interface.h"
#include "mcc_generated_files/pwm_hs/pwm.h"
#include "mcc_generated_files/dma/dma_interface.h"
#include "mcc_generated_files/dma/dma.h"
#include "mcc_generated_files/timer/timer_interface.h"
#include "mcc_generated_files/timer/sccp1.h"

#include <stdio.h>
#include <stdlib.h>

/*
    Main application
 */

#define NUM_SAMPLES_TO_AVG 10
#define ADC_ARRAY_SIZE 256

volatile uint16_t max = 0;
volatile uint16_t min = 10000;

volatile uint16_t (*dma_buffer) = (volatile uint16_t *)0x6000;

volatile uint16_t data[ADC_ARRAY_SIZE];

volatile uint32_t avg_array[ADC_ARRAY_SIZE - NUM_SAMPLES_TO_AVG];
volatile uint32_t adc_cb_count = 0;
volatile uint32_t cmp_cb_count = 0;


void getADCAverage()
{
    ADC1_IndividualChannelInterruptDisable(0);
    
    //Ensure PWMs are off, half bridge is at a constant state, and the tank has settled
    //Grab a ton of samples and average them.
    ADC1_IndividualChannelInterruptEnable(0);
}

void DMA_Channel0Callback(enum DMA_EVENT event)
{
    if(event == DMA_TRANSFER_DONE)
    {
        SCCP1_Timer_Stop();
        LATD = 0b0000011000000000;
        for(int i = 0; i < ADC_ARRAY_SIZE; i++)
        {
            data[i] = dma_buffer[i];
        }
        SCCP1_Timer_Start();
    }
}

/*void ADC1_ChannelCallback(const enum ADC1_CHANNEL channel, uint16_t adcVal)
{
    uint32_t count = (adc_cb_count % ADC_ARRAY_SIZE);
    data[count] = adcVal;
    adc_cb_count++;
    
    LATD = (0b0000011000000000 & adc_cb_count);
    //max = (adcVal > max) ? adcVal : max;
    //min = (adcVal < min) ? adcVal : min;
    if(count == 0)
    {
        //ADC1_Disable();
        avg_array[0] = data[0];
        for(int i = 1; i < NUM_SAMPLES_TO_AVG; i++)
        {
            avg_array[0] += data[i];
        }
        
        for(int i = 1; i < (ADC_ARRAY_SIZE - NUM_SAMPLES_TO_AVG); i++)
        {
            avg_array[i] = data[i + NUM_SAMPLES_TO_AVG] + avg_array[i-1] - data[i - 1];
        }
        //ADC1_Enable();
    }
    ADC1_ChannelSoftwareTriggerEnable(0);
}*/

int main(void)
{
    
    SYSTEM_Initialize();
    
    LATD = 0b0000000000000000;
    
    DMA_ChannelEnable(DMA_CHANNEL_0);
    
    ADC1_Enable();
    
    SCCP1_Timer_Start();
    
    PWM_Enable();
    
    while(1)
    {
        
        
    }
}

