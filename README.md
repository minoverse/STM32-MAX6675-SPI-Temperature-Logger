# STM32 MAX6675 SPI Temperature Logger (F446RE)

A simple embedded project that uses the MAX6675 K-type thermocouple sensor to measure temperature and sends the data over UART to a PC. Built using STM32CubeIDE for the STM32 Nucleo-F446RE board.

---

## Project Overview

- **MCU**: STM32F446RE (NUCLEO board)
- **Sensor**: MAX6675 (SPI-based thermocouple interface)
- **Display**: Serial monitor via UART
- **IDE**: STM32CubeIDE

---

##  Hardware Required

| Component            | Description                        |
|----------------------|------------------------------------|
| STM32 Nucleo-F446RE  | MCU board                          |
| MAX6675 Module       | Thermocouple interface             |
| K-type Thermocouple  | Temperature probe                  |
| Jumper Wires         | For connections                    |
| USB Cable (Micro-B)  | For power, UART, and programming   |

---

## Wiring

| MAX6675 Pin | STM32 Pin        |
|-------------|------------------|
| VCC         | 3.3V             |
| GND         | GND              |
| SCK         | PA5 (SPI1_SCK)   |
| SO (MISO)   | PA6 (SPI1_MISO)  |
| CS          | PA4 (GPIO Output)|

UART (for Serial Output):

| UART Signal | STM32 Pin |
|-------------|-----------|
| TX          | PA2       |
| RX          | PA3       |

---

##  STM32CubeIDE Configuration

### Peripherals:
- **SPI1**: Full Duplex Master, Software NSS, Prescaler /128
- **USART2**: Asynchronous Mode, 115200 baud
- **GPIO PA4**: Output (used for manual CS control)

---

##  Code Snippet (main.c)

```c
void HAL_UART_Send_Temp(uint16_t temp) {
    char buffer[50]; // no malloc
    snprintf(buffer, sizeof(buffer), "Temperature: %d C\r\n", temp);
    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

uint16_t MAX6675_Read_Temp() {
    uint8_t data[2];
    uint16_t value;

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // CS LOW
    HAL_SPI_Receive(&hspi1, data, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // CS HIGH

    value = ((data[0] << 8) | data[1]) >> 3;
    return value * 0.25;
}
