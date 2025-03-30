#include "stm32f4xx.h"

#define DHT11_PIN 6  // PA6
#define USART_TX_PIN 2  // PA2 (USART2 TX)

// Hàm delay chính xác (µs)
void delay_us(uint32_t us) {
    SysTick->LOAD = (SystemCoreClock / 1000000) * us - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 5;
    while (!(SysTick->CTRL & 0x10000));
    SysTick->CTRL = 0;
}

// Cấu hình GPIO PA6 làm Output
void DHT11_SetOutput(void) {
    GPIOA->MODER |= (1 << (DHT11_PIN * 2)); // Output mode
    GPIOA->OTYPER |= (1 << DHT11_PIN);      // Open-drain
    GPIOA->PUPDR |= (1 << (DHT11_PIN * 2)); // Pull-up
}

// Cấu hình GPIO PA6 làm Input
void DHT11_SetInput(void) {
    GPIOA->MODER &= ~(3 << (DHT11_PIN * 2)); // Input mode
    GPIOA->PUPDR |= (1 << (DHT11_PIN * 2));  // Pull-up
}

// Khởi tạo UART2 (PA2 TX)
void UART2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Bật clock UART2
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Bật clock GPIOA

    GPIOA->MODER |= (2 << (USART_TX_PIN * 2)); // PA2 là AF mode
    GPIOA->AFR[0] |= (7 << (USART_TX_PIN * 4)); // AF7 cho USART2

    USART2->BRR = (SystemCoreClock / 9600); // Baud rate 9600 (16MHz Clock)
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE; // Enable USART2, TX mode
}

// Gửi 1 ký tự qua UART2
void UART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

// Gửi chuỗi qua UART2
void UART2_SendString(char *str) {
    while (*str) {
        while (!(USART2->SR & USART_SR_TXE)); // Chờ TX sẵn sàng
        USART2->DR = *str++; // Gửi dữ liệu
    }
}




// Đọc dữ liệu từ DHT11
uint8_t DHT11_Read(uint8_t *temperature, uint8_t *humidity) {
    uint8_t data[5] = {0};

    DHT11_SetOutput();
    GPIOA->BSRR = (1 << (DHT11_PIN + 16));
    for (int i = 0; i <= 1800; i ++);
    GPIOA->BSRR = (1 << DHT11_PIN);
    for (int i = 0; i <= 30; i ++);


    //Đợi phản hồi từ cảm biến
    DHT11_SetInput();
    delay_us(40);
    if (!(GPIOA->IDR & (1 << DHT11_PIN))) {
        while (!(GPIOA->IDR & (1 << DHT11_PIN)));
        while (GPIOA->IDR & (1 << DHT11_PIN));
    } else {
        return 0; // Lỗi kết nối
    }

    //Đọc 40 bit dữ liệu
    for (uint8_t i = 0; i < 40; i++) {
        while (!(GPIOA->IDR & (1 << DHT11_PIN))); // Đợi bit bắt đầu

        for (int i = 0; i <= 40; i ++);
        if (GPIOA->IDR & (1 << DHT11_PIN)) { // Nếu vẫn HIGH sau 40µs => Bit = 1
            data[i / 8] = (data[i / 8] << 1) | 1;
            while (GPIOA->IDR & (1 << DHT11_PIN));
        } else {
            data[i / 8] = (data[i / 8] << 1);
        }
    }

    // 4. Kiểm tra checksum
    if ((data[0] + data[1] + data[2] + data[3]) == data[4]) {
        *humidity = data[0];
        *temperature = data[2];
        return 1; // Thành công
    }

    return 0;
}

// Cấu hình GPIOB (PB0 - PB7) làm Output
void GPIOB_Output_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Bật clock GPIOB
    GPIOB->MODER |= 0x00005555; // PB0 - PB7 là output
}

void ADC_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Bật clock ADC1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Bật clock GPIOA

    GPIOA->MODER |= (3 << (0 * 2)); // PA0 là analog mode

    ADC1->CR1 = 0; // Đặt về mặc định
    ADC1->CR2 = ADC_CR2_ADON; // Bật ADC

    ADC1->SQR3 = 0; // Chọn kênh 0 (PA0)
    ADC1->SMPR2 |= (7 << 0);
}

// Đọc giá trị ADC từ PA0
uint16_t ADC_Read(void) {
    ADC1->SQR3 = 0; // Chọn kênh 0 (PA0)
    ADC1->CR2 |= ADC_CR2_SWSTART; // Bắt đầu chuyển đổi
    while (!(ADC1->SR & ADC_SR_EOC)); // Chờ chuyển đổi hoàn tất
    return ADC1->DR; // Trả về giá trị đọc
}

// Cấu hình PWM trên PA7
void PWM_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Bật clock TIM3
    GPIOA->MODER |= (2 << (7 * 2)); // PA7 là AF mode
    GPIOA->AFR[0] |= (2 << (7 * 4)); // AF2 cho TIM3 CH2

    TIM3->PSC = 16 - 1; // Prescaler (16MHz / 16 = 1MHz)
    TIM3->ARR = 1000; // Chu kỳ PWM (1ms = 1kHz)
    TIM3->CCMR1 |= (6 << 12); // Chọn chế độ PWM1
    TIM3->CCER |= TIM_CCER_CC2E; // Bật PWM trên CH2
    TIM3->CR1 |= TIM_CR1_CEN; // Bật Timer
}

// Cập nhật độ rộng xung PWM (ngược giá trị ADC)
void PWM_SetDuty(uint16_t adc_value) {
    TIM3->CCR2 = 900 - (adc_value * 900 / 4095); // Độ rộng xung tỉ lệ nghịch với ADC
}

int main(void) {
    uint8_t temperature, humidity;

    // Bật clock GPIOA
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Khởi tạo UART2
    UART2_Init();

    GPIOB_Output_Init();
    ADC_Init();
    PWM_Init();
    // In thông báo khởi động
    UART2_SendString("DHT11Test \r");

    while (1) {
    	UART2_SendString("DHT11Test \r"); // bắt đầu 1 lần đọc

        uint8_t temperature, humidity;
        uint16_t adc_value = ADC_Read();
        PWM_SetDuty(adc_value);				// xét PWM lối ra PA7
        if (DHT11_Read(&temperature, &humidity)) {
            char buffer[50];
            sprintf(buffer, "%d %d %d\n", temperature, humidity, adc_value); // gửi dữ liệu đo được lên labview
            UART2_SendString(buffer);
            GPIOB->ODR = humidity << 1;		// cập nhật giá trị của độ ẩm ra 8 bit lối ra.
        } else {
            UART2_SendString("DHT11 Read Error\r\n");
        }

        for (int i = 0; i <= 1000000; i ++); // Đọc lại sau 1 giây
    }
}
