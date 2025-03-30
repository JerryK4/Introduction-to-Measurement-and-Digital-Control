#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

void UART_init() {
    // Cấu hình tốc độ baud
    UBRR0H = (UBRR_VALUE >> 8);   // Byte cao
    UBRR0L = UBRR_VALUE;          // Byte thấp
    
    // Cấu hình chế độ truyền (8-bit, 1 stop bit, không parity)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); 

    // Bật bộ truyền UART
    UCSR0B = (1 << TXEN0);
}

void UART_sendChar(char c) {
    while (!(UCSR0A & (1 << UDRE0)));  // Chờ cho thanh ghi trống
    UDR0 = c;  // Gửi dữ liệu
}

void UART_sendString(const char* str) {
    while (*str) {
        UART_sendChar(*str++);
    }
}

void setup() {
    UART_init();  // Khởi tạo UART
}

void loop() {
    static int count = 0;
    char buffer[20];
    
    sprintf(buffer, "Data: %d\r\n", count++);
    UART_sendString(buffer);
    
    delay(500);  // Chờ 500ms
}
