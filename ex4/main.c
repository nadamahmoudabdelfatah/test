#include "tm4c123gh6pm_registers.h"

#define SYSCTL_RCC2_USERCC2_MASK     0x80000000  /* USERCC2 Bit MASK */
#define SYSCTL_RCC2_BYPASS2_MASK     0x00000800  /* PLL BYPASS2 Bit MASK */
#define SYSCTL_RCC_XTAL_MASK         0x000007C0  /* XTAL Bits MASK */
#define SYSCTL_RCC_XTAL_8MHZ         0x00000380  /* 8 MHz Crystal Value */
#define SYSCTL_RCC_XTAL_16MHZ        0x00000540  /* 16 MHz Crystal Value */
#define SYSCTL_RCC2_OSCSRC2_MASK     0x00000070  /* OSCSRC2 Bits MASK */
#define SYSCTL_RCC2_OSCSRC2_MOSC     0x00000000  /* MOSC(Main Oscillator) value */
#define SYSCTL_RCC2_PWRDN2_MASK      0x00002000  /* PWRDN2 Bit MASK */
#define SYSCTL_RCC2_DIV400_MASK      0x40000000  /* DIV400 Bit MASK to Divide PLL as 400 MHz vs. 200 */
#define SYSCTL_RCC2_SYSDIV2_MASK     0x1FC00000  /* SYSDIV2 Bits MASK */
#define SYSCTL_RIS_PLLLRIS_MASK      0x00000040  /* PLLLRIS Bit MASK */
#define SYSCTL_RCC2_SYSDIV2_BIT_POS     22       /* SYSDIV2 Bits Postion start from bit number 22 */
#define SYSDIV2_VALUE 4

#define SYSTICK_PRIORITY_MASK  0x1FFFFFFF
#define SYSTICK_INTERRUPT_PRIORITY  3
#define SYSTICK_PRIORITY_BITS_POS   29

/* Enable IRQ Interrupts ... This Macro enables IRQ interrupts by clearing the I-bit in the PRIMASK. */
#define Enable_Interrupts()    __asm("CPSIE I")

/* Disable IRQ Interrupts ... This Macro disables IRQ interrupts by setting the I-bit in the PRIMASK. */
#define Disable_Interrupts()   __asm("CPSID I")

/* Go to low power mode while waiting for the next interrupt */
#define Wait_For_Interrupt()   __asm("WFI")

void Init_PLL(void)
{
   SYSCTL_RCC2_REG |=SYSCTL_RCC2_USERCC2_MASK;  //1
   SYSCTL_RCC2_REG |=SYSCTL_RCC2_BYPASS2_MASK; //2
   SYSCTL_RCC_REG &=~(SYSCTL_RCC_XTAL_MASK); //3
   SYSCTL_RCC_REG |=SYSCTL_RCC_XTAL_16MHZ;  //4
   SYSCTL_RCC2_REG &=~(SYSCTL_RCC2_OSCSRC2_MASK); //5
   SYSCTL_RCC2_REG |=SYSCTL_RCC2_OSCSRC2_MOSC; //6
   SYSCTL_RCC2_REG &=~(SYSCTL_RCC2_PWRDN2_MASK); //7
   SYSCTL_RCC2_REG |=SYSCTL_RCC2_DIV400_MASK;//8
   SYSCTL_RCC2_REG &=~(SYSCTL_RCC2_SYSDIV2_MASK);
   SYSCTL_RCC2_REG = SYSCTL_RCC2_REG | (SYSCTL_RCC2_SYSDIV2_BIT_POS << SYSDIV2_VALUE);
   while(!(SYSCTL_RIS_REG & SYSCTL_RIS_PLLLRIS_MASK) );
   SYSCTL_RCC2_REG &=~(SYSCTL_RCC2_BYPASS2_MASK);

}
unsigned char tick=0;
void SysTick_Handler(void)
{
  tick++;
}

/* Enable the SystTick Timer to run using the System Clock with Frequency 16Mhz and Count one second */
void SysTick_Init(void)
{
    SYSTICK_CTRL_REG    = 0;              /* Disable the SysTick Timer by Clear the ENABLE Bit */
    SYSTICK_RELOAD_REG  = 15999999;       /* Set the Reload value with 15999999 to count 1 Second */
    SYSTICK_CURRENT_REG = 0;              /* Clear the Current Register value */
    /* Configure the SysTick Control Register 
     * Enable the SysTick Timer (ENABLE = 1)
     * Disable SysTick Interrupt (INTEN = 0)
     * Choose the clock source to be System Clock (CLK_SRC = 1) */
    SYSTICK_CTRL_REG   |= 0x07;
    
    NVIC_SYSTEM_PRI3_REG = (NVIC_SYSTEM_PRI3_REG & SYSTICK_PRIORITY_MASK) | (SYSTICK_INTERRUPT_PRIORITY<<SYSTICK_PRIORITY_BITS_POS);   
}


/* Enable PF1, PF2 and PF3 (RED, Blue and Green LEDs) */
void Leds_Init(void)
{
    GPIO_PORTF_AMSEL_REG &= 0xF1;         /* Disable Analog on PF1, PF2 and PF3 */
    GPIO_PORTF_PCTL_REG  &= 0xFFFF000F;   /* Clear PMCx bits for PF1, PF2 and PF3 to use it as GPIO pin */
    GPIO_PORTF_DIR_REG   |= 0x0E;         /* Configure PF1, PF2 and PF3 as output pin */
    GPIO_PORTF_AFSEL_REG &= 0xF1;         /* Disable alternative function on PF1, PF2 and PF3 */
    GPIO_PORTF_DEN_REG   |= 0x0E;         /* Enable Digital I/O on PF1, PF2 and PF3 */
    GPIO_PORTF_DATA_REG  &= 0xF1;         /* Clear bit 0, 1 and 2 in Data regsiter to turn off the leds */
}

int main(void)
{
    /* Enable clock for PORTF and allow time for clock to start*/  
    volatile unsigned long delay = 0;
    SYSCTL_REGCGC2_REG |= 0x00000020;
    delay = SYSCTL_REGCGC2_REG;
    
    /* Initailize the LEDs as GPIO Pins */
    Leds_Init();
    
    /* Initalize the SysTick Timer to count one second */
    SysTick_Init();
    
    Init_PLL();
    
    while(1)
    {
        if(tick==5)
        {
        GPIO_PORTF_DATA_REG = (GPIO_PORTF_DATA_REG & 0xF1) | 0x02; /* Turn on the Red LED and disbale the others */
        }
        if(tick==10)
        {
        GPIO_PORTF_DATA_REG = (GPIO_PORTF_DATA_REG & 0xF1) | 0x04; /* Turn on the Blue LED and disbale the others */
        }
        if(tick==15)
        {
        GPIO_PORTF_DATA_REG = (GPIO_PORTF_DATA_REG & 0xF1) | 0x08; /* Turn on the Green LED and disbale the others */
        tick=0;
        }
    }
}