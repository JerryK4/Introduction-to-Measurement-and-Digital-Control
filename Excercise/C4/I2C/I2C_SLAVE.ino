#define SLA 0x08
char receiveBuffer[20];

void I2C_Init() {
    TWAR = SLA << 1;  // Đặt địa chỉ Slave
    TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT);
}

void I2C_Wait() {
    while (!(TWCR & (1 << TWINT)));
}

void I2C_Respond() {
    TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWINT);
    I2C_Wait();
}

void setup() {
    Serial.begin(9600);
    I2C_Init();
}

void loop() {
    I2C_Respond();  // Đợi yêu cầu từ Master

    if ((TWSR & 0xF8) == 0x60) {  // Master gửi dữ liệu
        uint8_t i = 0;
        do {
            I2C_Respond();
            receiveBuffer[i] = TWDR;
        } while (receiveBuffer[i++] != '\0' && i < sizeof(receiveBuffer) - 1);
        receiveBuffer[i] = '\0';

        Serial.print("Slave nhận từ Master: ");
        Serial.println(receiveBuffer);
    }

    // Gửi phản hồi "Hi Master"
    if ((TWSR & 0xF8) == 0xA8) {  // Master yêu cầu đọc dữ liệu từ Slave
        Serial.println("Slave: Gửi -> Hi Master");
        char reply[] = "Hi Master";
        for (uint8_t i = 0; reply[i] != '\0'; i++) {
            TWDR = reply[i];
            TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
            I2C_Wait();
        }
        TWDR = '\0';  // Kết thúc chuỗi
        TWCR = (1 << TWEN) | (1 << TWINT);
        I2C_Wait();
    }
}
