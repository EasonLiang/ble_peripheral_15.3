#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "boards.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_lbs.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define ACEE(a) { unsigned int err_code ; err_code=a; APP_ERROR_CHECK(err_code); }

#define CONN_SUP_TIMEOUT_BLINKY             MSEC_TO_UNITS(4000,UNIT_10_MS)      //gap_params_init for blinky
#define MAX_CONN_INTERVAL_BLINKY            MSEC_TO_UNITS(200,UNIT_1_25_MS)
#define MIN_CONN_INTERVAL_BLINKY            MSEC_TO_UNITS(100,UNIT_1_25_MS)
#define SLAVE_LATENCY_BLINKY                0
#define CONN_SUP_TIMEOUT_UART               MSEC_TO_UNITS(4000,UNIT_10_MS)      //gap_params_init for uart
#define MAX_CONN_INTERVAL_UART              MSEC_TO_UNITS(75,UNIT_1_25_MS)
#define MIN_CONN_INTERVAL_UART              MSEC_TO_UNITS(20,UNIT_1_25_MS)
#define SLAVE_LATENCY_UART                  0
#define CONN_SUP_TIMEOUT_BPS                MSEC_TO_UNITS(4000,UNIT_10_MS)      //gap_params_init for bps
#define MAX_CONN_INTERVAL_BPS               MSEC_TO_UNITS(1000,UNIT_1_25_MS)
#define MIN_CONN_INTERVAL_BPS               MSEC_TO_UNITS(500,UNIT_1_25_MS)
#define SLAVE_LATENCY_BPS                   0
#define CONN_SUP_TIMEOUT_GLS                MSEC_TO_UNITS(4000,UNIT_10_MS)      //gap_params_init for gls
#define MAX_CONN_INTERVAL_GLS               MSEC_TO_UNITS(100,UNIT_1_25_MS)
#define MIN_CONN_INTERVAL_GLS               MSEC_TO_UNITS(10,UNIT_1_25_MS)
#define SLAVE_LATENCY_GLS                   0

// #define BLE_ADV_FAST_INTEVAL_BLINKY         64
// #define BLE_ADV_FAST_TIMEOUT_BLINKY         18000
#define BLE_ADV_FAST_INTEVAL_UART           64                                  //advertising_init for uart
#define BLE_ADV_FAST_TIMEOUT_UART           18000
#define BLE_ADV_FAST_INTEVAL_BPS            40                                  //advertising_init for bps
#define BLE_ADV_FAST_TIMEOUT_BPS            18000
#define BLE_ADV_FAST_INTEVAL_GLS            40                                  //advertising_init for gls
#define BLE_ADV_FAST_TIMEOUT_GLS            18000

#define FIRST_CONN_PARAMS_UPDATE_DELAY_BLINKY   APP_TIMER_TICKS(20000)          //conn_params_init for blinky
#define NEXT_CONN_PARAMS_UPDATE_DELAY_BLINKY    APP_TIMER_TICKS(5000) 
#define FIRST_CONN_PARAMS_UPDATE_DELAY_UART     APP_TIMER_TICKS(30000)          //conn_params_init for uart
#define NEXT_CONN_PARAMS_UPDATE_DELAY_UART      APP_TIMER_TICKS(5000)  
#define FIRST_CONN_PARAMS_UPDATE_DELAY_BPS      APP_TIMER_TICKS(30000)          //conn_params_init for bps
#define NEXT_CONN_PARAMS_UPDATE_DELAY_BPS       APP_TIMER_TICKS(5000) 
#define FIRST_CONN_PARAMS_UPDATE_DELAY_GLS      APP_TIMER_TICKS(30000)          //conn_params_init for gls
#define NEXT_CONN_PARAMS_UPDATE_DELAY_GLS       APP_TIMER_TICKS(5000)

#define DIASTOLIC_0         117
#define MEAN_ARTERIAL_0     103
#define PULSE_RATE_0        60
#define SYSTOLIC_0          76

#define DIASTOLIC_1         121
#define MEAN_ARTERIAL_1     81
#define PULSE_RATE_1        106
#define SYSTOLIC_1          72

#define DIASTOLIC_2         138
#define MEAN_ARTERIAL_2     88
#define PULSE_RATE_2        120
#define SYSTOLIC_2          105

#define DIASTOLIC_3         145
#define MEAN_ARTERIAL_3     100
#define PULSE_RATE_3        131
#define SYSTOLIC_3          125

// check "Compatibility mode" button in path : nRF Connect -> System Menu(left upper corner) -> Settings -> Scanner 

#ifndef ACEE
#else
    // #define ACEE
    // #undef err_code
#endif