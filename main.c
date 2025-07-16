#include <stdint.h>
/* registers map: */
// NVIC
#define NVIC_REG 0xe000e100
// RCC
#define RCC 0x40023800
#define RCC_AHB1ENR_OFFSET 0x30
#define RCC_APB1ENR_OFFSET 0x40
// GPIO
#define GPIO_PORT_D 0x40020C00
#define GPIO_PORT_C 0x40020800
#define GPIO_ODR_OFFSET 0x14
#define GPIO_MODER_OFFSET 0x0
#define GPIO_AFRL0_OFFSET 0x20
#define GPIO_BSRR_OFFSET 0x18

#define GPIO_IDR_OFFSET 0x10
#define GPIO_ODR_OFFSET 0x14
//USAR2
#define USART2 0x40004400
#define USART2_BRR_OFFSET 0x08 //define baud rate
#define USART2_CR1_OFFSET 0x0C // control register 1
#define USART2_DR_OFFSET 0x04 // data register
#define USART2_SR_OFFSET 0x00 // status register
/* end registers map */
#define SET_BIT(REG, BIT) (*((uint32_t*)REG) |= (BIT))
#define CLEAR_BIT(REG, BIT) ((*(uint32_t*)REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((*(uint32_t*)REG) & (BIT))
#define CLEAR_REG(REG) ((*(uint32_t*)REG) = (0x0))
#define WRITE_REG(REG, VAL) (*((uint32_t*)REG) = (VAL))
#define READ_REG(REG) ((*(uint32_t*)REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) WRITE_REG((REG),
(((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define TOGGLE_BIT(REG, BIT) (*((uint32_t*)REG) ^= (BIT))
void delayMS(int delay);
void USART2_Init (void);
char USART2_Read (void);
void USART_Text_Write(char *text);
void save_num(char c);
void error_fun(void);
void save_num1(void);
short overflow(short st, char b, char n);
short operation(char op, short n1, short n2);
void calculate(void);
char buffer[5];
short num1 = 0;
short num2 = 0;
short result = 0;
short state = 0;
int main(void)
{
int no_interrupt = 0;
SET_BIT((RCC + RCC_AHB1ENR_OFFSET), (1 << 0)); // Enable GPIOA
SET_BIT((RCC + RCC_AHB1ENR_OFFSET), (1 << 2)); // Enable GPIOC
SET_BIT((GPIO_PORT_D + GPIO_MODER_OFFSET), (1 << 30)); // set GPIO_D_15
as output.
// initialize the USART2
USART2_Init();
while(1){
// if the button is pressed
if(!( READ_BIT(GPIO_PORT_C + GPIO_IDR_OFFSET, (1<<15)) )) {
delayMS(100);
// toggle the led
TOGGLE_BIT(GPIO_PORT_D + GPIO_ODR_OFFSET , (1 << 5));
// toggle activation of interrupt
TOGGLE_BIT(USART2 + USART2_CR1_OFFSET, (1 << 5));
no_interrupt ^= 1;
// is the interruption mode is enabled, we send a message to the
console telling
// that we are working with interruption, if not, we also send a
message to the
// console saying it
if(no_interrupt == 0)
USART_Text_Write("\r\n************ Mode with interruption
************\r\n");
else
USART_Text_Write("\r\n************ Mode without interruption
************\r\n");
}
// if we are working without interruptions, we read all the time for a
value
// and calculate the operation, or send an error if it is the case
if(no_interrupt){
calculate();
}
}
}
void USART2_IRQHandler(void) {
// we enter this function only when we are working with interruption, and
calculate every time we get
// a new value in the console
calculate();
}
// this function makes the calculation, it reads the input values. If we
receive more than 5 numbers, it.
// sends an error. If the result also is more than 5 numbers, we send an
error. If we have an overflow,
// also sends and error. When we get the symbol "=" we show the result of
the operation.
void calculate(void){
char sign;
if (state == 0) {
if( READ_BIT(USART2 + USART2_SR_OFFSET, (1<<5)) ){
char c = READ_REG(USART2 + USART2_DR_OFFSET);
//USART2->DR = c;
// We read only characters that are digits '0-9'
if(((c>47) && (c<58))){
save_num(c);
}
// we read the operation's symbols + - * /
else if(((c==42) || (c==43) || (c==45) || (c==47))){
WRITE_REG(USART2 + USART2_DR_OFFSET, c);
sign = c;
num1 = atoi(buffer); // convert chain of chars into number short
if(!overflow(num1, buffer[0], buffer[4])){
state = 1;
}
else {
state = 3;
}
}
}
}
else if (state == 1) {
if( READ_BIT(USART2 + USART2_SR_OFFSET, (1<<5)) ){
char c = READ_REG(USART2 + USART2_DR_OFFSET);
//USART2->DR = c;
if(((c>47) && (c<58))){
save_num(c);
}
else if(c==61){
WRITE_REG(USART2 + USART2_DR_OFFSET, c);
num2 = atoi(buffer);
//size_t n = sizeof(buffer) / sizeof(buffer[0]);
if(!overflow(num2, buffer[0], buffer[4])){
short fin = operation(sign, num1, num2);
if((fin>=0) && ((sign == 48) || (sign == 43) || (sign ==
47))){
char s[5];
USART_Text_Write(s);
USART_Text_Write("\r\n");
state = 0;
}
else if(sign == 45) {
char s[5];
USART_Text_Write(s);
USART_Text_Write("\r\n");
state = 0;
}
else{
error_fun();
state = 3;
}
} else {
state = 3;
}
}
}
}
}
short overflow(short st, char b, char n){
short s = b - '0';
short t = n - '0';
if((st<0) || ((t>=0) && ((st/10000) != s))){
error_fun();
return 1;
}
else {
return 0;
}
}
short operation(char op, short n1, short n2){
// check which sign we read, and make the correspondent operation
switch(op) {
case 42:
return n1 * n2;
break;
case 43:
return n1 + n2;
break;
case 45:
return n1 - n2;
break;
case 47:
return n1 / n2;
break;
default:
break;
}
}
void USART2_Init(void){
SET_BIT((RCC + RCC_APB1ENR_OFFSET), (1 << 17)); // enable usart2
clock.
SET_BIT((GPIO_PORT_D + GPIO_AFRL0_OFFSET), (1 << 20)); // enable
alternative function PD5
SET_BIT((GPIO_PORT_D + GPIO_AFRL0_OFFSET), (1 << 24)); // enable
alternative function PD6
SET_BIT((GPIO_PORT_D + GPIO_MODER_OFFSET ), (1 << 11)); // set PD5 to
be tx
SET_BIT((GPIO_PORT_D + GPIO_MODER_OFFSET ), (1 << 13)); // set PD6 to b
rx
WRITE_REG(USART2 + USART2_BRR_OFFSET, 0x0683); //9600 baud at 16mhz.
SET_BIT((USART2 + USART2_CR1_OFFSET ), (1 << 5)); //enable usart
interruption
SET_BIT((USART2 + USART2_CR1_OFFSET ), (1 << 2)); //enable rx
SET_BIT((USART2 + USART2_CR1_OFFSET ), (1 << 3)); // enable tx
SET_BIT((USART2 + USART2_CR1_OFFSET ), (1 << 13)); // enable usart
// enable USART2_IRQ
*(uint32_t*)NVIC_REG |= 1 << 38;
}
char USART2_Read(void){
// check if we have data available
if( READ_BIT(USART2 + USART2_SR_OFFSET, (1<<5))) {
return READ_REG(USART2 + USART2_DR_OFFSET); // read data register.
}
}
void save_num(char text){
// save the read number in the buffer of size 5 characters
char temp;
for(int i = 0; i<= 5; i++){
temp = buffer[i];
if(i==5) {error_fun();}
else if(!((temp>47) && (temp<58))){
buffer[i] = text;
WRITE_REG(USART2 + USART2_DR_OFFSET, text); //write data in the
register.
break;
}
}
}
void error_fun(void) {
//if we get an error, we display a messagen and turn on a led
USART_Text_Write("Error\r\n");
SET_BIT(GPIO_PORT_D + GPIO_BSRR_OFFSET, (1<<15)); // set bit PD15
}
char USART2_Write(char ch)
{
// function for writing a character
while(! READ_BIT(USART2 + USART2_SR_OFFSET, (1<<7)) ){} //check if tx data
is not empty
WRITE_REG(USART2 + USART2_DR_OFFSET, (ch & 0xff));
return ch;
}
void USART_Text_Write(char *text){
//function for writing a message
while(*text) USART2_Write(*text++);
}
void delayMS(int delay){
int i;
for( ;delay>0; delay--){
for(i=0; i<=3195;i++);
}
}
