/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef VND_COMMON_H__
#define VND_COMMON_H__

#include <zephyr/bluetooth/mesh.h>

/**
 * @brief Vendor Model common definitions
 * @defgroup bt_mesh_vendor_model Vendor Model common definitions
 * @{
 */

/* Company ID - using Nordic company ID */
#define BT_COMP_ID_VENDOR 0x0059

/* Vendor model IDs */
#define BT_MESH_MODEL_ID_VENDOR_SRV 0x1000
#define BT_MESH_MODEL_ID_VENDOR_CLI 0x1001

/* Vendor model opcodes */
#define BT_MESH_VENDOR_OP_SET 	  BT_MESH_MODEL_OP_3(0x10, BT_COMP_ID_VENDOR)
#define BT_MESH_VENDOR_OP_GET 	  BT_MESH_MODEL_OP_3(0x11, BT_COMP_ID_VENDOR)
#define BT_MESH_VENDOR_OP_STATUS  BT_MESH_MODEL_OP_3(0x12, BT_COMP_ID_VENDOR)

/* Maximum message length (excluding 3 byte opcode) is 377 bytes */
#define BT_MESH_VENDOR_MSG_MAXLEN_SET    (377)

/* Status message max length (excluding 3 byte opcode) is 377 bytes */
#define BT_MESH_VENDOR_MSG_MAXLEN_STATUS (377)

/**
 * @brief Vendor Status Message
 *
 * This structure represents the status response from a vendor model server.
 */
struct bt_mesh_vendor_status {
	struct net_buf_simple *buf;
};

/**
 * @brief Vendor Set Message
 *
 * This structure represents the set message sent to a vendor model server.
 */
struct bt_mesh_vendor_set {
	struct net_buf_simple *buf;
};

/** @} */

#endif /* VND_COMMON_H__ */
