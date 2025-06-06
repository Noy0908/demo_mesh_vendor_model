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

#include "model_handler.h"

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

	printk("Initializing Vendor Model Demo");

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err: %d)", err);
	}

	return 0;
}
