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

int main(void)
{

	//turn on GPIO clocks
	RCC_AHB2ENR |= (1U << 1) | (1U << 2);   // GPIOB and GPIOC

	//Configure blue LED
	GPIOB_MODER &= ~(3U << (7 * 2)); //clear
	GPIOB_MODER |=  (1U << (7 * 2)); //make output

	//clear to make button gpio input (input is 00)
	GPIOC_MODER &= ~(3U << (13 * 2));

	//turn on SYSCFG
	RCC_APB2ENR |= (1U << 0);

	//Map push button to the corresponding external interrupt
	SYSCFG_EXTICR4 &= ~(0xFU << 4); //clear
	SYSCFG_EXTICR4 |=  (0x2U << 4); //map to EXTICR4

	EXTI_IMR1  |= (1U << 13);   // unmask line 13

	EXTI_RTSR1 |= (1U << 13);   // rising edge (button press)

	//enable interrupt 40
	NVIC_ISER1 |= (1U << 8);


while(1)
{
};
}

//override the weak function
void EXTI15_10_IRQHandler(void)
{
    EXTI_PR1 = (1U << 13);     // clear the pending
    GPIOB_ODR ^= (1U << 7);    // toggle blue LED
}
