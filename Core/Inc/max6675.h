/*
 * max6675.h
 *
 *  Created on: Aug 1, 2025
 *      Author: skylink
 */

#ifndef INC_MAX6675_H_
#define INC_MAX6675_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#define MAX6675_CONVERSION_TIME_MS  220     // Typical conversion time in milliseconds
#define MAX6675_RESOLUTION          0.25f   // Each bit = 0.25°C
#define MAX6675_MAX_TEMP            1024.0f // Maximum readable temp
#define MAX6675_MIN_TEMP            0.0f    // Minimum readable temp
#define MAX6675_ERROR_TEMP          -999.0f // Error sentinel value
typedef enum {
    MAX6675_OK = 0,
    MAX6675_ERROR_DISCONNECTED,
    MAX6675_ERROR_SPI,
    MAX6675_ERROR_TIMEOUT,
    MAX6675_ERROR_INVALID_PARAM
} max6675_status_t;

// ===============================
// Sensor Object Structure
// ===============================
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    float last_temperature;
    uint32_t last_read_time;
    uint32_t error_count;
    max6675_status_t last_status;
    bool initialized;
} max6675_t;

// ===============================
// Function Declarations
// ===============================
max6675_status_t MAX6675_Init(max6675_t *sensor, SPI_HandleTypeDef *hspi,
                              GPIO_TypeDef *cs_port, uint16_t cs_pin);

max6675_status_t MAX6675_ReadTemperature(max6675_t *sensor, float *temperature);

max6675_status_t MAX6675_StartConversion(max6675_t *sensor);

bool MAX6675_IsConversionReady(max6675_t *sensor);

const char* MAX6675_GetErrorString(max6675_status_t status);

void MAX6675_GetStats(max6675_t *sensor, uint32_t *error_count, float *last_temp);

#endif /* INC_MAX6675_H_ */
