#include <avr/io.h>
#include <util/delay.h>

void ADC_Config(void) {
    ADMUX = (1 << REFS0); // Chọn điện áp tham chiếu là AVcc, kênh ADC0
    ADCSRA = (1 << ADEN)  // Bật ADC
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Chia tần số 128
}

uint16_t ADC_Read(void) {
    ADCSRA |= (1 << ADSC); // Bắt đầu chuyển đổi
    while (ADCSRA & (1 << ADSC)); // Chờ chuyển đổi hoàn tất
    return ADC;
}

void USART_Init(void) {
    UBRR0H = 0;
    UBRR0L = 103; // Baudrate 9600 với 16MHz
    UCSR0B = (1 << TXEN0); // Bật truyền TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Dữ liệu 8-bit
}

void USART_Transmit(char c) {
    while (!(UCSR0A & (1 << UDRE0))); // Chờ TX sẵn sàng
    UDR0 = c;
}

void USART_SendString(const char *str) {
    while (*str) {
        USART_Transmit(*str++);
    }
}

int main(void) {
    ADC_Config();
    USART_Init();
    
    char buffer[10];
    while (1) {
        uint16_t value = ADC_Read();
        itoa(value, buffer, 10);
        USART_SendString(buffer);
        USART_SendString("\r\n");
    }
}