#include "stm32f4xx.h"
#include <string.h>
#include <stdlib.h>
#include<stdio.h>

#define UART_BUFFER_SIZE  20

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_index = 0;
volatile uint8_t waveform_type = 2;
volatile uint16_t frequency = 1000; // Mặc định 1kHz
volatile uint8_t new_data_flag = 0;

// Bảng mẫu dạng sóng 8-bit
const uint8_t sine_wave[256] = { /* Dữ liệu mẫu sin 8-bit */
			127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,
		    176,179,182,185,188,191,194,197,200,203,206,209,212,215,218,221,
		    224,227,229,232,235,238,241,243,246,249,251,254,255,255,255,255,
		    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		    254,252,250,248,246,244,242,240,238,236,234,232,230,228,226,224,
		    222,220,218,216,214,212,210,208,206,204,202,200,198,196,194,192,
		    190,188,186,184,182,180,178,176,174,172,170,168,166,164,162,160,
		    158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,
		    125,123,121,119,117,115,113,111,109,107,105,103,101,99,97,95,
		    93,91,89,87,85,83,81,79,77,75,73,71,69,67,65,63,
		    61,59,57,55,53,51,49,47,45,43,41,39,37,35,33,31,
		    29,27,25,23,21,19,17,15,13,11,9,7,5,3,1,0,
		    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,
		    33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63 };
const uint8_t triangle_wave[256] = { /* Dữ liệu mẫu tam giác */
		  0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,
		  32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,
		  64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,
		  96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
		  128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,
		  160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,
		  192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,
		  224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,
		  255,253,251,249,247,245,243,241,239,237,235,233,231,229,227,225,
		  223,221,219,217,215,213,211,209,207,205,203,201,199,197,195,193,
		  191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,161,
		  159,157,155,153,151,149,147,145,143,141,139,137,135,133,131,129,
		  127,125,123,121,119,117,115,113,111,109,107,105,103,101,99,97,
		  95,93,91,89,87,85,83,81,79,77,75,73,71,69,67,65,
		  63,61,59,57,55,53,51,49,47,45,43,41,39,37,35,33,
		  31,29,27,25,23,21,19,17,15,13,11,9,7,5,3,1
};
const uint8_t square_wave[256] = { /* Dữ liệu mẫu vuông */
		  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
const uint8_t sawtooth_wave[256] = { /* Dữ liệu mẫu răng cưa */
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
		  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
		  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
		  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
		  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
		  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
		  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
		  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
		  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
		  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
		  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
		  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
		  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
		  208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
		  224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
		  240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

// Khai báo chức năng
void SystemClock_Config(void);
void UART2_Init(void);
void GPIO_Init(void);
void TIM2_Init(uint16_t freq);
void UART2_IRQHandler(void);
void parse_uart_data(void);
void generate_waveform(void);

int main(void) {
    SystemClock_Config();
    UART2_Init();
    GPIO_Init();
    TIM2_Init(frequency);

    while (1) {
        if (new_data_flag) {
            new_data_flag = 0;
            parse_uart_data();
            TIM2_Init(frequency);
        }
    }
}

// Cấu hình Clock
void SystemClock_Config(void) {
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
}

// Cấu hình UART2
void UART2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Bật Clock cho UART2
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  // Bật Clock cho GPIOA

    GPIOA->MODER |= (2 << (2 * 2)) | (2 << (3 * 2)); // PA2 - Tx, PA3 - Rx chế độ AF
    GPIOA->AFR[0] |= (7 << (2 * 4)) | (7 << (3 * 4)); // AF7 cho UART2

    USART2->BRR = 16000000 / 9600; // Baudrate 9600
    USART2->CR1 |= USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
    NVIC_EnableIRQ(USART2_IRQn); // Bật ngắt UART2
}

// Cấu hình GPIO
void GPIO_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Bật Clock cho GPIOB
    GPIOB->MODER |= 0x00005555; // PB0-PB7 Output
}

// Cấu hình Timer2 để phát sóng
void TIM2_Init(uint16_t freq) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 160 - 1; // Chia tần số xuống 100kHz
    TIM2->ARR = (100000 / (freq*100)) - 1;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM2_IRQn);
}

// Xử lý ngắt UART2
void USART2_IRQHandler(void) {
if (USART2->SR & USART_SR_RXNE) {  // Nếu có dữ liệu nhận
        char c = USART2->DR;
if (c == '\n' || c == '\r') { // Kết thúc chuỗi khi nhận ký tự xuống dòng
            uart_buffer[uart_index] = '\0'; // Kết thúc chuỗi
            uart_index = 0;
            new_data_flag = 1; // Đánh dấu có dữ liệu mới
        } else {
            if (uart_index < UART_BUFFER_SIZE - 1) {
                uart_buffer[uart_index++] = c;
            } else {
                uart_index = 0; // Reset nếu buffer quá dài
            }
        }
    }
}


// Giải mã chuỗi UART nhận được
void parse_uart_data(void) {
    char *token = strtok((char *)uart_buffer, ",");
    if (token) waveform_type = atoi(token);
    token = strtok(NULL, ",");
    if (token) frequency = atoi(token);
}


// Xử lý ngắt Timer2 để phát sóng
void TIM2_IRQHandler(void) {
    static uint16_t index = 0;
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        uint8_t value = 0;
        switch (waveform_type){
            case 0: value = sine_wave[index]; break;
            case 1: value = triangle_wave[index]; break;
            case 2: value = square_wave[index]; break;
            case 3: value = sawtooth_wave[index]; break;
        }
        GPIOB->ODR = value; // Xuất dữ liệu ra PB0 -> PB7
        index = (index + 1) % 256;
    }
}