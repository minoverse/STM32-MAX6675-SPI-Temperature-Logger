/*
 * amx6675.c
 *
 *  Created on: Aug 1, 2025
 *      Author: skylink
 */


// File: max6675.c

#include "max6675.h"

max6675_status_t MAX6675_Init(max6675_t *sensor, SPI_HandleTypeDef *hspi,
                               GPIO_TypeDef *cs_port, uint16_t cs_pin) {
    if (!sensor || !hspi || !cs_port) {
        return MAX6675_ERROR_INVALID_PARAM;
    }

    sensor->hspi = hspi;
    sensor->cs_port = cs_port;
    sensor->cs_pin = cs_pin;
    sensor->last_temperature = 0.0f;
    sensor->last_read_time = 0;
    sensor->error_count = 0;
    sensor->last_status = MAX6675_OK;
    sensor->initialized = false;

    // Set CS high (inactive)
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

    // Wait for power-up stabilization
    HAL_Delay(100);

    // Try reading to confirm connectivity
    float temp;
    max6675_status_t status = MAX6675_ReadTemperature(sensor, &temp);
    if (status == MAX6675_OK || status == MAX6675_ERROR_DISCONNECTED) {
        sensor->initialized = true;
        return MAX6675_OK;
    }

    return status;
}

max6675_status_t MAX6675_ReadTemperature(max6675_t *sensor, float *temperature) {
    if (!sensor || !temperature || !sensor->initialized) {
        return MAX6675_ERROR_INVALID_PARAM;
    }

    uint8_t rx_data[2] = {0};
    uint16_t raw_temp;

    // Start SPI communication
    HAL_GPIO_WritePin(sensor->cs_port, sensor->cs_pin, GPIO_PIN_RESET);
    for (volatile int i = 0; i < 100; i++);  // Short delay for CS setup

    HAL_StatusTypeDef result = HAL_SPI_Receive(sensor->hspi, rx_data, 2, 1000);
    HAL_GPIO_WritePin(sensor->cs_port, sensor->cs_pin, GPIO_PIN_SET);

    if (result != HAL_OK) {
        sensor->error_count++;
        sensor->last_status = MAX6675_ERROR_SPI;
        return MAX6675_ERROR_SPI;
    }

    raw_temp = ((uint16_t)rx_data[0] << 8) | rx_data[1];

    // Check for thermocouple disconnect (D2 == 1)
    if (raw_temp & 0x0004) {
        sensor->error_count++;
        sensor->last_status = MAX6675_ERROR_DISCONNECTED;
        *temperature = MAX6675_ERROR_TEMP;
        return MAX6675_ERROR_DISCONNECTED;
    }

    // Extract and convert temperature
    raw_temp >>= 3;
    float temp_c = raw_temp * MAX6675_RESOLUTION;

    if (temp_c < MAX6675_MIN_TEMP || temp_c > MAX6675_MAX_TEMP) {
        sensor->error_count++;
        sensor->last_status = MAX6675_ERROR_SPI;
        *temperature = MAX6675_ERROR_TEMP;
        return MAX6675_ERROR_SPI;
    }

    sensor->last_temperature = temp_c;
    sensor->last_read_time = HAL_GetTick();
    sensor->last_status = MAX6675_OK;
    *temperature = temp_c;

    return MAX6675_OK;
}

max6675_status_t MAX6675_StartConversion(max6675_t *sensor) {
    if (!sensor || !sensor->initialized) {
        return MAX6675_ERROR_INVALID_PARAM;
    }
    sensor->last_read_time = HAL_GetTick();  // Conversion begins after CS high
    return MAX6675_OK;
}

bool MAX6675_IsConversionReady(max6675_t *sensor) {
    if (!sensor || !sensor->initialized) {
        return false;
    }
    return (HAL_GetTick() - sensor->last_read_time >= MAX6675_CONVERSION_TIME_MS);
}

const char* MAX6675_GetErrorString(max6675_status_t status) {
    switch (status) {
        case MAX6675_OK: return "OK";
        case MAX6675_ERROR_DISCONNECTED: return "Thermocouple Disconnected";
        case MAX6675_ERROR_SPI: return "SPI Communication Error";
        case MAX6675_ERROR_TIMEOUT: return "Timeout Error";
        case MAX6675_ERROR_INVALID_PARAM: return "Invalid Parameter";
        default: return "Unknown Error";
    }
}

void MAX6675_GetStats(max6675_t *sensor, uint32_t *error_count, float *last_temp) {
    if (sensor && error_count && last_temp) {
        *error_count = sensor->error_count;
        *last_temp = sensor->last_temperature;
    }
}
