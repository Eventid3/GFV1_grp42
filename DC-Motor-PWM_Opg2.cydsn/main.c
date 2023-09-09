/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdint.h>

CY_ISR_PROTO(ISR_UART_rx_handler);
void handleByteReceived(uint8_t byteReceived);
void decreaseSpeed(void);
void increaseSpeed(void);
void driveForwards(void);
void driveBackwards(void);
void stop(void);

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    isr_uart_rx_StartEx(ISR_UART_rx_handler);
    UART_1_Start();
    PWM_1_Start();
    
    
    UART_1_PutString("DC-Motor-PWM application started\r\n");
    UART_1_PutString("0: Stop\r\n");
    UART_1_PutString("1: Drive forwards\r\n");
    UART_1_PutString("2: Drive backwards\r\n");
    UART_1_PutString("q: Decrease speed\r\n");
    UART_1_PutString("w: Increase speed\r\n");

    for(;;)
    {
        /* Place your application code here. */
    }
}

CY_ISR(ISR_UART_rx_handler)
{
    uint8_t bytesToRead = UART_1_GetRxBufferSize();
    while (bytesToRead > 0)
    {
        uint8_t byteReceived = UART_1_ReadRxData();
        UART_1_WriteTxData(byteReceived); // echo back
        
        handleByteReceived(byteReceived);
        
        bytesToRead--;
    }
}

void handleByteReceived(uint8_t byteReceived)
{
    switch(byteReceived)
    {
        case 'q' :
        {
            decreaseSpeed();
        }
        break;
        case 'w' :
        {
            increaseSpeed();
        }
        break;
        case '1' :
        {
            driveForwards();
        }
        break;
        case '2' :
        {
            driveBackwards();
        }
        break;
        case '0' :
        {
            stop();
            
        }
        break;
        default :
        {
            // nothing
        }
        break;
    }
}

void decreaseSpeed()
{
    UART_1_PutString("\rDecreasing speed          ");
    int8_t cmpValue = PWM_1_ReadCompare() - 10;
    cmpValue = cmpValue <= 0 ? 0 : cmpValue; // cap cmpValue at 0
    PWM_1_WriteCompare(cmpValue);
    
}

void increaseSpeed()
{
    UART_1_PutString("\rIncreasing speed          ");
    uint8_t cmpValue = PWM_1_ReadCompare() + 10;
    cmpValue = cmpValue >= 100 ? 100 : cmpValue; // cap cmpValue at 100
    PWM_1_WriteCompare(cmpValue);
}

void driveForwards()
{
    UART_1_PutString("\rSet direction: forwards   ");
    Pin_IN2_Write(0);
    Pin_IN1_Write(1);
    UART_1_PutChar(Pin_IN1_Read());
    PWM_1_Start();
}

void driveBackwards()
{
    UART_1_PutString("\rSet direction: backwards  ");
    Pin_IN1_Write(0);
    Pin_IN2_Write(1);
    UART_1_PutChar(Pin_IN2_Read());
    PWM_1_Start();
}

void stop()
{
    UART_1_PutString("\rStop                      ");
    PWM_1_Sleep();
}

/* [] END OF FILE */
