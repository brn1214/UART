# UART

## Goal

_Develop a calculator program. Values are input from a computer via UART in the format: xx...xyxx...x=, where x are decimal digits and y is an operator (+, -, *, /). Number input is terminated either by an operator sign (for the first number) or an equals sign (for the second number), or after five digits have been entered. Note that operands cannot be negative, but the answer can. The size of the result and both operands should be short int (a 16-bit signed number), and overflow protection must be implemented. In case of invalid operations (if the answer or input numbers exceed the variable size in memory), a red LED should turn on, and the word error should be output to the serial channel instead of the answer. Enabling/disabling interrupts should be done by pressing a button on the board, and a message of arbitrary content should be sent to the serial port, indicating which mode is active (with or without interrupts)._

🐈‍⬛🐈

The main goal from this project is to make use of the communication protocol USART. We decided
to use the USART2.
As we can see in the following diagram, it is necessary to enable the clock APB1.

<img width="521" height="591" alt="Captura de pantalla 2025-07-15 a la(s) 11 47 53 p m" src="https://github.com/user-attachments/assets/b6aaa382-191c-45ea-97c4-b7310d3fa221" />

In order to work with this register, we should know that some of the GPIO from the board, have
alternate functions, this means, they don’t only work as input/output, they could manage different
functions as USART, I2C, TIM and so on.
For enabling the USART2 in the STM32F427, we can see in the alternate function mapping that we
can use the GPIOD and use the pins 5 and 6 as transmitter and receiver.

<img width="641" height="563" alt="Captura de pantalla 2025-07-15 a la(s) 11 48 19 p m" src="https://github.com/user-attachments/assets/cea162c4-ba19-4ce1-8a31-39a132fd1784" />

Another important data is that we can see that the USART2 has the AF7 (Alternate function 7), so
we can configure it in the code.
The program should work with interruption and without interruption. Working without interruption
just means that we need to run the code inside the infinite loop, so the program will be reading the
receiver register all the time, in order to know if we have received a data.
Working with interruptions is more efficient. The infinite loop could be doing another task, but as
soon as we get a data in the RX register, it sends an interruption which stops the program and run
the task related to that interruption, so we don’t need to ask all the time if we already get a data.
For this, we use IRQHandler, which will be triggered every time we get an interruption.
The logic of the code is as follows:


---

First, we enter the infinite loop and start working without interruption. We start polling
and wait for a data to be received.
If we get a data, we read that the char is only a number “0-9”, or a symbol “+ - * /”.
First, we read up to 5 numbers. If we get more than 5, we show an error message on the
console and turn on a led.
If we get a symbol, we save the number in a buffer, and start reading the next number.
If we get a “=”, we stop reading and make the mathematical operation, in accordance
with the symbol we got before.
We display the final result on the console.
If the button is pressed, we change to working with interruption. We show a message
on the console.
This time, the code doesn’t do anything, until it the RX gets a data, so it triggers the
interruption and start doing the process of receiving the numbers or symbol. It saves the
data in a buffer, so the data isn’t lost when the program for the interruption is finished.
