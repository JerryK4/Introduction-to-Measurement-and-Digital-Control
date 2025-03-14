/*
2. Sử dụng vi điều khiển kết nối với mạch H-bridge L298, động cơ DC và lập trình điều khiển tốc độ động cơ.
*/
#include "stm32f4xx.h"
void TIM2_PWM_Init(void);
void Delay_ms(uint32_t ms);
void GPIO_Init(void);
int main(void) {
   GPIO_Init();  // Cấu hình PB5 làm đầu ra
   TIM2_PWM_Init(); // Cấu hình PWM trên PB10
   while (1) {
       GPIOB->BSRR = (1 << 21); // tắt PB5 (5 + 16)
       for (int duty = 30; duty <= 90; duty += 10) {
           TIM2->CCR3 = (1000 * duty) / 100; // Điều chỉnh duty cycle
           Delay_ms(200);
       }
       Delay_ms(1000);
       GPIOB->BSRR = (1 << 5); // bật PB5
       for (int duty = 0; duty <= 70; duty += 10) {
           TIM2->CCR3 = (1000 * duty) / 100;
           Delay_ms(200);
       }
   }
}
void TIM2_PWM_Init(void) {
   RCC->AHB1ENR |= (1 << 1); // Bật clock GPIOB
   RCC->APB1ENR |= (1 << 0); // Bật clock TIM2
   GPIOB->MODER |= (2 << 20);  // PB10 chế độ Alternate Function
   GPIOB->AFR[1] |= (1 << 8);  // Chọn AF1 (TIM2_CH3) cho PB10
   TIM2->PSC = 83;   // Chia clock xuống 1 MHz
   TIM2->ARR = 999; // PWM tần số 1 kHz
   TIM2->CCR3 = 0;       // Bắt đầu với duty cycle 0%
   TIM2->CCMR2 |= (6 << 12) | (1 << 11); // PWM mode 1, preload enable
   TIM2->CCER |= (1 << 8);  // Kích hoạt PWM kênh 3
   TIM2->CR1 |= (1 << 0); // Bật Timer 2
}
void GPIO_Init(void) {
   RCC->AHB1ENR |= (1 << 1);  // Bật clock GPIOB
   GPIOB->MODER |= (1 << 10); // Cấu hình PB5 là Output
}
void Delay_ms(uint32_t ms) {
   for (uint32_t i = 0; i < ms * 4000; i++) {
       __NOP();
   }
}
