/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic Mesh Vendor Model Demo
 */
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh.h>
#include <dk_buttons_and_leds.h>
#include <bluetooth/mesh/dk_prov.h>
#include <zephyr/logging/log.h>

#include "uart_app.h"
#include "model_handler.h"

LOG_MODULE_REGISTER(peripheral_uart);

#define STACKSIZE 						0x2000
#define PRIORITY 						7

K_SEM_DEFINE(ble_init_ok, 0, 1);


void error(void)
{
	LOG_ERR("Panic!");

	while (true) {
		/* Spin for ever */
		k_sleep(K_MSEC(1000));
	}
}

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err: %d)", err);
		return;
	}

	printk("Bluetooth initialized");

	err = dk_leds_init();
	if (err) {
		printk("Failed to initialize LEDs (err: %d)", err);
		return;
	}

	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	if (err) {
		printk("Initializing mesh failed (err: %d)", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");

	printk("Press Button 1 to send SET message\n");
	printk("Press Button 2 to send SET UNACK request\n");
	printk("Press Button 3 to send GET request\n");
}

int main(void)
{
	int err;

	LOG_INF("Initializing Vendor Model Demo, the version is %s", CONFIG_FW_VERSION);

	err = uart_init();
	if (err) {
		error();
	}

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err: %d)", err);
	}

	LOG_INF("Bluetooth initialized");

	k_sem_give(&ble_init_ok);

	return 0;
}

void ble_write_thread(void)
{
	/* Don't go any further until BLE is initialized */
	k_sem_take(&ble_init_ok, K_FOREVER);

	struct uart_data_t uart_data = {
		.len = 0,
	};

	for (;;) 
	{
		/* Wait for data from UART to be processed */
		struct uart_data_t *buf = k_fifo_get(&fifo_uart_rx_data, K_FOREVER);
		if(buf)
		{
			int plen = MIN(sizeof(uart_data.data) - uart_data.len, buf->len);
			int loc = 0;

			while (plen > 0) {
				memcpy(&uart_data.data[uart_data.len], &buf->data[loc], plen);
				uart_data.len += plen;
				loc += plen;
	#ifdef CONFIG_BT_NUS_CRLF_UART_TERMINATION
				if (uart_data.len >= sizeof(uart_data.data) ||
				(uart_data.data[uart_data.len - 1] == '\n') ||
				(uart_data.data[uart_data.len - 1] == '\r')) {
	#endif
					if(!transparent_flag)
					{
						handle_uart_data(&uart_data);		//handle uart command from host mcu
					}
					// else
					// {
					// 	ble_send_uart_data(uart_data.data, uart_data.len);
					// }
					
					uart_data.len = 0;
	#ifdef CONFIG_BT_NUS_CRLF_UART_TERMINATION
				}
	#endif
				plen = MIN(sizeof(uart_data.data), buf->len - loc);
			}

			k_free(buf);
		}
	}
}


K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL,
		NULL, PRIORITY, 0, 0);