# lab2-EE186

Part 2 
Explain how external interrupts work and how the ISR handles button presses. What happens to the main loop? When does the ISR get executed? What would happen if you did not clear the EXTI pending flag inside the ISR?

External interrupts are when the hardware watches for an event (so the CPU doesn’t need to keep checking). The button press makes a rising edge on PC13, SYSCFG routes PC13 to EXTI13, and EXTI sets the pending flag and raises a request. Next, the NVIC decides whether the CPU is going to respond (both the EXTI mask and NVIC enable have to be on in order for the request to be approved). Lastly, if approved, the CPU looks up the handler in the vector table and runs it. 

Inside the ISR, it first clears the pending flag in EXTI_PR1. When it runs it toggles the blue LED on PB7. 

When the interrupt fires, the CPU pauses the loop, saves its registers to the stack, runs the ISR, restores the registers, and resumes leaving the main loop right where it left off. ISR gets executed after the button is pressed, and EXTI and NVIC lets it through. 

My code never calls the ISR. The hardware triggers it by looking up its address in the vector table, which works because my function name matches the weak default handler in the startup file.

 If you do not clear the pending flag inside the ISR then the NVIC sees the request still pending as soon as the ISR exits, so the CPU immediately runs the ISR again and again and again. As a result, main barely gets to run and the LED toggles so fast it looks stuck or flickering instead of following button presses.


Part 3
Explain how timer interrupt works. What function is counting time passed? What function triggers the ISR? How is the timer interrupt different from an external interrupt?

A timer interrupt is a hardware generated signal that pauses the CPU at regular and predictable intervals to run an ISR. The clock source is the chip’s oscillator, which pulses at a fixed frequency. (in my case an internal oscillator that runs at 16 MHz). The prescaler divides down the clock frequency and controls how fast the timer ticks. The counter is a built-in hardware counter that counts up at every tick, and it is what counts the time that has passed. I set the auto-reload register (ARR) to the number the counter should count up to. When the counter reaches ARR, the timer generates an update event: it sets a flag in TIM4_SR, resets the counter to 0, and starts counting again. Because the update interrupt is enabled in TIM4_DIER, that flag sends a request to the NVIC, which tells the CPU to run the ISR. The ISR clears the flag in TIM4_SR and then does its job. Unlike an external interrupt which is triggered by something outside the chip like a button press a timer interrupt is triggered by the chip’s own counter, meaning it fires at regular intervals and doesn’t need SYSCFG or EXTI.

Calculate the timer frequency. Show the process
(from the given slides)
Counter frequency: f_counter = f_clock / (PSC + 1)
TIM4 input clock is 16MHz
Target of 1s
PSC = 15999
f_counter = 16,000,000 / (15999 + 1) = 16,000,000 / 16,000 = 1000 Hz
Time per interrupt: Time = (PSC + 1) / f_clock × (ARR + 1)
1 x 1000Hz = 1000 ticks
ARR + 1 = 1000, ARR = 999
Check: (15999 + 1) / 16,000,000 × (999 + 1) = 0.001 × 1000 = 1 s 


