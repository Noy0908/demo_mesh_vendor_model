/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

 #ifndef __NUS_SETTING_APP_H_
 #define __NUS_SETTING_APP_H_
 
#define DEVICE_SN_SIZE          6

 int save_mac_address(uint8_t *mac, uint8_t len);

 int16_t save_new_baudrate(uint8_t *data, uint8_t len);

 int16_t save_device_sn(uint8_t *data, uint8_t len);

 uint8_t * get_mac_address(void);

 uint32_t get_uart_baudrate(void);

 uint64_t get_device_sn(void);

 void nus_settings_init(void);

 #endif