/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include <zephyr/bluetooth/mesh.h>
#include "vnd_cli.h"

/**
 * @brief Initialize the model handler module
 *
 * @return The Bluetooth Mesh composition data for the node
 */
const struct bt_mesh_comp *model_handler_init(void);

/**
 * @brief Send a Vendor Set message
 *
 * @param data     Data to send
 * @param len      Length of data
 * @param rsp      Pointer to response buffer, or NULL to not wait for response
 * @return 0 on success, negative error code otherwise
 */
int vendor_model_send_set(const uint8_t *data, size_t len, struct bt_mesh_vendor_status *rsp);

/**
 * @brief Send a Vendor Get message
 *
 * @return 0 on success, negative error code otherwise
 */
int vendor_model_send_get(void);

#endif /* MODEL_HANDLER_H__ */
