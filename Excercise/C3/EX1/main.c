#include "stm32f4xx.h"
void ADC_Init(void);
uint16_t ADC_Read(void);
void PWM_Init(void);
void PWM_SetDutyCycle(uint16_t duty);
int main(void) {
   ADC_Init();  // Cấu hình ADC đọc cảm biến quang
   PWM_Init();  // Cấu hình PWM để điều chỉnh độ sáng LED
   while (1) {
       uint16_t adcValue = ADC_Read();  // Đọc giá trị từ cảm biến ánh sáng
       // Chuyển đổi ADC (0 → 500) thành PWM (255 → 0)
       uint16_t pwmValue = (adcValue > 500) ? 0 : (255 - (adcValue * 255 / 500));
       PWM_SetDutyCycle(pwmValue);  // Điều chỉnh độ sáng LED
   }
}
// ============================
// Cấu hình ADC đọc từ PA0 (ADC1_IN0)
// ============================
void ADC_Init(void) {
   RCC->APB2ENR |= (1<<8);	// Bật clock cho ADC1
   RCC->AHB1ENR |= (1<<0);	// Bật clock cho GPIOA
   GPIOA->MODER |= (3 << (0 * 2));  // PA0 là chế độ Analog
   ADC1->CR2 = 0;  // Tắt ADC trước khi cấu hình
   ADC1->SQR3 = 0; // Chọn kênh ADC1_IN0 (PA0)
   ADC1->SMPR2 |= (7 << (0 * 3)); // Chọn tốc độ lấy mẫu (480 cycles)
   ADC1->CR2 |= (1<<0);		// Bật ADC
}
uint16_t ADC_Read(void) {
   ADC1->CR2 |= (1<<30);	 // Bắt đầu chuyển đổi
   while (!(ADC1->SR & (1<<1))); // Chờ chuyển đổi hoàn tất
   return ADC1->DR; // Trả về giá trị ADC
}
// ============================
// Cấu hình PWM trên TIM2_CH1 (PA5)
// ============================
void PWM_Init(void) {
   RCC->APB1ENR |= (1<<0);	// Bật clock cho TIM2
   RCC->AHB1ENR |= (1<<0);	// Bật clock cho GPIOA
   GPIOA->MODER |= (2 << (5 * 2));  // PA5 ở chế độ Alternate Function
   GPIOA->AFR[0] |= (1 << (5 * 4)); // AF1 cho TIM2_CH1
   TIM2->PSC = 15;  // Prescaler: 16MHz / (15+1) = 1MHz
   TIM2->ARR = 255; // Giá trị PWM max (8-bit)
   TIM2->CCMR1 |= (6 << 4); // Chế độ PWM1
   TIM2->CCMR1 |= (1 << 3); // Cho phép preload
   TIM2->CCER |= (1<<0);	// Bật TIM2_CH1 output

   TIM2->CR1 |= (1<<0); // Bật Timer
}
// ============================
// Điều chỉnh độ sáng LED
// ============================
void PWM_SetDutyCycle(uint16_t duty) {
   TIM2->CCR1 = duty; // Cập nhật giá trị PWM
}
