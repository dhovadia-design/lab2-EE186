#include <stdint.h>
#define RCC_AHB2ENR  (*(volatile uint32_t *)(0x40021000 + 0x4C))

//port b holds blue LED
#define GPIOB_MODER  (*(volatile uint32_t *)(0x48000400 + 0x00))

//port c holds button
#define GPIOC_MODER  (*(volatile uint32_t *)(0x48000800 + 0x00))

//breaker for SYSCFG
#define RCC_APB2ENR     (*(volatile uint32_t *)(0x40021000 + 0x60))

//external interrupt
#define SYSCFG_EXTICR4  (*(volatile uint32_t *)(0x40010000 + 0x14))

//EXTI setup
#define EXTI_IMR1   (*(volatile uint32_t *)(0x40010400 + 0x00))
#define EXTI_RTSR1  (*(volatile uint32_t *)(0x40010400 + 0x08))

//NVIC address
#define NVIC_ISER1  (*(volatile uint32_t *)(0xE000E100 + 0x04))

//pending address
#define EXTI_PR1   (*(volatile uint32_t *)(0x40010400 + 0x14))

//how to control blue LED
#define GPIOB_ODR  (*(volatile uint32_t *)(0x48000400 + 0x14))

//address for the clock
#define RCC_CR  (*(volatile uint32_t *)(0x40021000 + 0x00))

//need to select clock
#define RCC_CFGR  (*(volatile uint32_t *)(0x40021000 + 0x08))

//breaker for TIM4
#define RCC_APB1ENR1  (*(volatile uint32_t *)(0x40021000 + 0x58))

//TIM4 base address
#define TIM4_PSC  (*(volatile uint32_t *)(0x40000800 + 0x28)) //divider
#define TIM4_ARR  (*(volatile uint32_t *)(0x40000800 + 0x2C)) //count limit

//allows the TIM4 to ask for an ISP
#define TIM4_DIER  (*(volatile uint32_t *)(0x40000800 + 0x0C))

//allow ISP
#define NVIC_ISER0  (*(volatile uint32_t *)(0xE000E100 + 0x00))

//counter
#define TIM4_CR1  (*(volatile uint32_t *)(0x40000800 + 0x00))

//timer status
#define TIM4_SR  (*(volatile uint32_t *)(0x40000800 + 0x10))

//green led
#define GPIOC_ODR  (*(volatile uint32_t *)(0x48000800 + 0x14))

//used to toggle leds
volatile uint32_t color = 0;

//falling edge
#define EXTI_FTSR1  (*(volatile uint32_t *)(0x40010400 + 0x0C))

//button pin
#define GPIOC_IDR  (*(volatile uint32_t *)(0x48000800 + 0x10))

//hold button status
volatile uint32_t held = 0;

int main(void)
{

	//turn on GPIO clocks
	RCC_AHB2ENR |= (1U << 1) | (1U << 2);   // GPIOB and GPIOC

	//Configure LEDs

	//blue
	GPIOB_MODER &= ~(3U << (7 * 2)); //clear
	GPIOB_MODER |=  (1U << (7 * 2)); //make output

	//red
	GPIOB_MODER &= ~(3U << (14 * 2)); //clear
	GPIOB_MODER |=  (1U << (14 * 2)); //make output

	//green
	GPIOC_MODER &= ~(3U << (7 * 2)); //clear
	GPIOC_MODER |=  (1U << (7 * 2)); //make output

	//clear to make button gpio input (input is 00)
	GPIOC_MODER &= ~(3U << (13 * 2));

	//turn on SYSCFG
	RCC_APB2ENR |= (1U << 0);

	//Map push button to the corresponding external interrupt
	SYSCFG_EXTICR4 &= ~(0xFU << 4); //clear
	SYSCFG_EXTICR4 |=  (0x2U << 4); //map to EXTICR4

	EXTI_IMR1  |= (1U << 13);   // unmask line 13

	EXTI_RTSR1 |= (1U << 13);   // rising edge (button press)
	EXTI_FTSR1 |= (1U << 13);   // falling edge

	//enable interrupt 40
	NVIC_ISER1 |= (1U << 8);

	//turn on 16MHz clock
	RCC_CR |= (1U <<8);

	//wait until the clock is ready
	while ((RCC_CR & (1U << 10)) == 0);

	//select the 16Mhz clock
	RCC_CFGR &= ~(3U << 0);  // clear
	RCC_CFGR |=  (1U << 0);  // choose clock

	// wait until the chip confirms
	while ((RCC_CFGR & (3U << 2)) != (1U << 2));

	//turn on clock
	RCC_APB1ENR1 |= (1U << 2);

	//assign base address values (calculated in readme)
	TIM4_PSC = 15999;
	TIM4_ARR = 999; //1s = 16MHz/16000 = 1000 ticks

	//allow ISP request
	TIM4_DIER |= (1U << 0);

	//CPU allows TIM4
	NVIC_ISER0 |= (1U << 30);

	TIM4_CR1 |= (1U << 0);   // start the counter
while(1)
{
};
}

//override the weak function
void EXTI15_10_IRQHandler(void)
{
    EXTI_PR1 = (1U << 13);     // clear the pending

    //button pressed
    if (GPIOC_IDR & (1U << 13)) {
           held = 1;
       }
    //released
    else{
    	held = 0;
    }

    if (held == 1){
    	//green on
    	if (color == 1){
    	    	GPIOC_ODR |= (1U << 7);
    	    	GPIOB_ODR &= ~(1U << 14); // red off
    	    	GPIOB_ODR &= ~(1U << 7); // blue off

    	    }
    	//keep blue on
    	else if (color == 2){
    		GPIOB_ODR |= (1U << 7);
    		GPIOB_ODR &= ~(1U << 14); // red off
    		GPIOC_ODR &= ~(1U << 7); // green off
    	    }
    	//red on
    	else{
    	    GPIOB_ODR |= (1U << 14);
    	    GPIOB_ODR &= ~(1U << 7); // blue off
    	    GPIOC_ODR &= ~(1U << 7); // green off
    	    }
    }
}

//ISP timer trigger
void TIM4_IRQHandler(void)
{
    TIM4_SR &= ~(1U << 0);     // clear

    //make LEDs same if button is held
    if (held == 1) {
            return; //leave LED alone
        }

    //turn off all LEDs
    GPIOB_ODR &= ~(1U << 7); // blue off
    GPIOC_ODR &= ~(1U << 7); // green off
    GPIOB_ODR &= ~(1U << 14); // red off

    color = (color +1) % 3;
    //turn on green
    if (color == 1){
    	GPIOC_ODR |= (1U << 7);
    }
    //turn on blue
    else if (color == 2){
    	GPIOB_ODR |= (1U << 7);
    }
    //red on
    else{
    	GPIOB_ODR |= (1U << 14);
    }

}
