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
#include <stdbool.h>
#include <stdio.h>

CY_ISR_PROTO(ISR_UART_rx_handler);
CY_ISR_PROTO(ISR_Timer);
void handleByteReceived(uint8_t byteReceived);
void decreaseSpeed(void);
void increaseSpeed(void);
void driveForwards(void);
void driveBackwards(void);
void switchDriveState(void);
void stop(void);
void drive(void);

typedef enum
{
    WAVE_DRIVE,
    HALF_STEP_DRIVE
}DRIVE_STATE;

typedef enum
{
    A1, 
    A1A2, 
    A2, 
    A2B1, 
    B1, 
    B1B2, 
    B2,
    B2A1
}STEP_STATE;

typedef enum
{
    FORWARDS,
    BACKWARDS
}DIRECTION_STATE;

static DRIVE_STATE driveState = WAVE_DRIVE;
static STEP_STATE stepState = A1;
static DIRECTION_STATE directionState = FORWARDS;
static bool stepBool = true;


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    isr_uart_rx_StartEx(ISR_UART_rx_handler);
    UART_1_Start();
    
    UART_1_PutString("DC-Motor-PWM application started\r\n");
    UART_1_PutString("0: Stop\r\n");
    UART_1_PutString("1: Drive forwards\r\n");
    UART_1_PutString("2: Drive backwards\r\n");
    UART_1_PutString("q: Decrease speed\r\n");
    UART_1_PutString("w: Increase speed\r\n");
    UART_1_PutString("a: Select drive state\r\n");
    
    //driveState = WAVE_DRIVE;
    //stepState = A1;
    //directionState = FORWARDS;
    //stepBool = true;
    Pin_A1_Write(1); // set initial position
    
    isr_timer_StartEx(ISR_Timer);
    Timer_1_Start();
    
    // char timerBuffer[256]; 

    for(;;)
    {
        /* Place your application code here. */
        drive();
        //snprintf(timerBuffer, sizeof(timerBuffer), "Timer period: %d \n\r", Timer_1_ReadCounter());
        //UART_1_PutString(timerBuffer);
    }
}

CY_ISR(ISR_UART_rx_handler)
{
    uint8_t bytesToRead = UART_1_GetRxBufferSize();
    while (bytesToRead > 0)
    {
        uint8_t byteReceived = UART_1_ReadRxData();
        // UART_1_WriteTxData(byteReceived); // echo back
        
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
        case 'a':
        {
            switchDriveState();
        }
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
    uint16_t period = Timer_1_ReadPeriod() * 1.5f;
    period = period >= 255 ? 255 : period; 
    /*
    // For debugging
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Bytes read: %d\r\n", period);
    UART_1_PutString(buffer);
    */
    Timer_1_WritePeriod((uint8)period);
    
}

void increaseSpeed()
{
    UART_1_PutString("\rIncreasing speed          ");
    uint16_t period = Timer_1_ReadPeriod() * 0.75f;
    period = period <= 2 ? 2 : period; 
    /*
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Bytes read: %d\r\n", period);
    UART_1_PutString(buffer);
    */
    Timer_1_WritePeriod((uint8)period);
    
}

void driveForwards()
{
    UART_1_PutString("\rSet direction: forwards   ");
    Timer_1_Start();
    directionState = FORWARDS;
}

void driveBackwards()
{
    UART_1_PutString("\rSet direction: backwards  ");
    Timer_1_Start();
    directionState = BACKWARDS;
}

void stop()
{
    UART_1_PutString("\rStop                      ");
    Timer_1_Sleep();
    //stepBool = false;
}

void switchDriveState()
{
    if (driveState == WAVE_DRIVE)
    {
        UART_1_PutString("\rHalf Step Drive                ");
        driveState = HALF_STEP_DRIVE;
    }
    else
    {
        UART_1_PutString("\rWave Drive                     ");
        driveState = WAVE_DRIVE;
    }
}

void drive()
{
    if (stepBool && driveState == WAVE_DRIVE)
    {
        if (directionState == FORWARDS)
        {
            switch (stepState)
            {
                case A1:
                    Pin_A1_Write(0);
                    Pin_A2_Write(1);
                    stepState = A2;
                    break;
                case A2:
                    Pin_A2_Write(0);
                    Pin_B1_Write(1);
                    stepState = B1;
                    break;
                case B1:
                    Pin_B1_Write(0);
                    Pin_B2_Write(1);
                    stepState = B2;
                    break;
                case B2:
                    Pin_B2_Write(0);
                    Pin_A1_Write(1);
                    stepState = A1;
                    break;
                default:
                    // failsafe hvis vi kommer fra half step drive
                    Pin_B1_Write(0);
                    Pin_B2_Write(0);
                    Pin_A2_Write(0);
                    Pin_A1_Write(1);
                    stepState = A1;
                    break;
            }
        }
        else
        {
            switch (stepState)
            {
                case A1:
                    Pin_A1_Write(0);
                    Pin_B2_Write(1);
                    stepState = B2;
                    break;
                case A2:
                    Pin_A2_Write(0);
                    Pin_A1_Write(1);
                    stepState = A1;
                    break;
                case B1:
                    Pin_B1_Write(0);
                    Pin_A2_Write(1);
                    stepState = A2;
                    break;
                case B2:
                    Pin_B2_Write(0);
                    Pin_B1_Write(1);
                    stepState = B1;
                    break;
                default:
                    // failsafe hvis vi kommer fra half step drive
                    Pin_B1_Write(0);
                    Pin_B2_Write(0);
                    Pin_A2_Write(0);
                    Pin_A1_Write(1);
                    stepState = A1;
                    break;
            }
        }
    }
    
    else if (stepBool && driveState == HALF_STEP_DRIVE)
    {
        if (directionState == FORWARDS)
        {
            switch (stepState)
            {
                case A1:
                    Pin_A2_Write(1);
                    stepState = A1A2;
                    break;
                case A1A2:
                    Pin_A1_Write(0);
                    stepState = A2;
                    break;
                case A2:
                    Pin_B1_Write(1);
                    stepState = A2B1;
                    break;
                case A2B1:
                    Pin_A2_Write(0);
                    stepState = B1;
                    break;
                case B1:
                    Pin_B2_Write(1);
                    stepState = B1B2;
                    break;
                case B1B2:
                    Pin_B1_Write(0);
                    stepState = B2;
                    break;
                case B2:
                    Pin_A1_Write(1);
                    stepState = B2A1;
                    break;
                case B2A1:
                    Pin_B2_Write(0);
                    stepState = A1;
                    break;
            }    
        }
        else
        {
            switch (stepState)
            {
                case A1:
                    Pin_B2_Write(1);
                    stepState = B2A1;
                    break;
                case A1A2:
                    Pin_A2_Write(0);
                    stepState = A1;
                    break;
                case A2:
                    Pin_A1_Write(1);
                    stepState = A1A2;
                    break;
                case A2B1:
                    Pin_B1_Write(0);
                    stepState = A2;
                    break;
                case B1:
                    Pin_A2_Write(1);
                    stepState = A2B1;
                    break;
                case B1B2:
                    Pin_B2_Write(0);
                    stepState = B1;
                    break;
                case B2:
                    Pin_B1_Write(1);
                    stepState = B1B2;
                    break;
                case B2A1:
                    Pin_A1_Write(0);
                    stepState = B2;
                    break;
            }
        }
    }
    
    stepBool = false;
    
}

CY_ISR(ISR_Timer)
{
    stepBool = true;
}

/* [] END OF FILE */
