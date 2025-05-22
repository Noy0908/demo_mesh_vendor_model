/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef VND_CLI_H__
#define VND_CLI_H__

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/kernel.h>
#include "vnd_common.h"

/**
 * @brief Vendor Client Model
 * @defgroup bt_mesh_vendor_cli Vendor Client Model
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Vendor Client Model Context */
struct bt_mesh_vendor_cli {
	/** Vendor model entry */
	const struct bt_mesh_model *model;
	/** Publish parameters */
	struct bt_mesh_model_pub pub;
	/** Publication message */
	struct net_buf_simple pub_msg;
	/** Publication message buffer */
	uint8_t buf[BT_MESH_VENDOR_MSG_MAXLEN_SET + 3 + 4];
	/** Acknowledged message tracking */
	struct bt_mesh_msg_ack_ctx ack_ctx;
	/** @brief Status message handler
	 *
	 * Called when a Vendor_Status message is received
	 *
	 * @param[in] cli    Vendor Client model
	 * @param[in] ctx    Message context
	 * @param[in] status Vendor status contained in the message
	 */
	void (*const status_handler)(struct bt_mesh_vendor_cli *cli,
			     struct bt_mesh_msg_ctx *ctx,
			     const struct bt_mesh_vendor_status *status);
};

/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _bt_mesh_vendor_cli_op[];
extern const struct bt_mesh_model_cb _bt_mesh_vendor_cli_cb;
/** @endcond */

/** @def BT_MESH_VND_CLI_INIT
 *
 * @brief Initialization parameters for the @ref bt_mesh_vendor_cli.
 *
 * @param[in] _status_handler Optional status message handler.
 */
#define BT_MESH_VND_CLI_INIT(_status_handler)                                  \
	{                                                                      \
		.status_handler = _status_handler,                             \
	}

/** Vendor Client model composition data entry. */
#define BT_MESH_MODEL_VND_CLI(_cli)                                           \
	BT_MESH_MODEL_VND_CB(BT_COMP_ID_VENDOR, BT_MESH_MODEL_ID_VENDOR_CLI, \
			     _bt_mesh_vendor_cli_op, &(_cli)->pub,            \
			     BT_MESH_MODEL_USER_DATA(struct bt_mesh_vendor_cli, _cli), \
			     &_bt_mesh_vendor_cli_cb)

/** Default timeout for vendor client operations */
#define VND_CLI_TIMEOUT K_MSEC(5000)

/**
 * @brief Send a vendor set message
 *
 * @param cli      Vendor Client model
 * @param ctx      Message context, or NULL to use the configured publish parameters
 * @param set      Vendor set message to send
 * @return 0 on success, or negative error code otherwise
 */
int bt_mesh_vendor_cli_set(struct bt_mesh_vendor_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_vendor_set *set,
			   struct bt_mesh_vendor_status *rsp);

/**
 * @brief Send a vendor get message and wait for status response
 *
 * @param cli      Vendor Client model
 * @param ctx      Message context, or NULL to use the configured publish parameters
 * @param rsp      Status response buffer, filled with the response data on success
 * @return 0 on success, or negative error code otherwise
 */
int bt_mesh_vendor_cli_get(struct bt_mesh_vendor_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_status *rsp);

/**
 * @brief Send a vendor set unacknowledged message
 *
 * @param cli      Vendor Client model
 * @param ctx      Message context, or NULL to use the configured publish parameters
 * @param set      Vendor set message to send
 * @return 0 on success, or negative error code otherwise
 */
int bt_mesh_vendor_cli_set_unack(struct bt_mesh_vendor_cli *cli,
			         struct bt_mesh_msg_ctx *ctx,
			         const struct bt_mesh_vendor_set *set);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* VND_CLI_H__ */
