/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef VND_SRV_H__
#define VND_SRV_H__

#include <zephyr/bluetooth/mesh.h>
#include "vnd_common.h"

/**
 * @brief Vendor Server Model
 * @defgroup bt_mesh_vendor_srv Vendor Server Model
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration of vendor server model */
struct bt_mesh_vendor_srv;

/** @def BT_MESH_VENDOR_SRV_INIT
 *
 * @brief Initialization parameters for a @ref bt_mesh_vendor_srv instance.
 *
 * @param[in] _handlers Pointer to a handler function structure.
 */
#define BT_MESH_VENDOR_SRV_INIT(_handlers)                                    \
	{                                                                      \
		.handlers = _handlers,                                         \
	}

/** Vendor Server Model Handler functions */
struct bt_mesh_vendor_srv_handlers {
	/** @brief Set callback
	 *
	 * Called when a Vendor_Set message is received
	 *
	 * @param srv    Vendor Server model
	 * @param ctx    Message context
	 * @param set    Vendor set message received
	 * @param rsp    Vendor status message to be sent
	 *
	 * @return 0 on success, or negative error code to send response later or to skip response
	 */
	int (*const set)(struct bt_mesh_vendor_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_set *set,
			  struct bt_mesh_vendor_status *rsp);

	/** @brief Get callback
	 *
	 * Called when a Vendor_Get message is received
	 *
	 * @param srv    Vendor Server model
	 * @param ctx    Message context
	 * @param get    Vendor get message parameters, can be NULL
	 * @param rsp    Vendor status message to be sent
	 *
	 * @return 0 on success, or negative error code to send response later or to skip response
	 */
	int (*const get)(struct bt_mesh_vendor_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_vendor_get *get,
			  struct bt_mesh_vendor_status *rsp);
};

/** Vendor Server Model Context */
struct bt_mesh_vendor_srv {
	/** Application handler functions */
	const struct bt_mesh_vendor_srv_handlers *const handlers;
	/** Vendor model entry */
	const struct bt_mesh_model *model;
	/** Publish parameters */
	struct bt_mesh_model_pub pub;
	/** Publication message */
	struct net_buf_simple pub_msg;
	/** Publication message buffer */
	uint8_t buf[BT_MESH_VENDOR_MSG_MAXLEN_STATUS + 4];
	/** Status buffer */
	struct net_buf_simple status_msg;
	/** Current status data */
	uint8_t status_buf_data[BT_MESH_VENDOR_MSG_MAXLEN_STATUS + 4];
};

/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _bt_mesh_vendor_srv_op[];
extern const struct bt_mesh_model_cb _bt_mesh_vendor_srv_cb;
/** @endcond */

/** Vendor Server model composition data entry. */
#define BT_MESH_MODEL_VND_SRV(srv, handlers)                                      \
	BT_MESH_MODEL_VND_CB(BT_COMP_ID_VENDOR, BT_MESH_MODEL_ID_VENDOR_SRV,  \
			     _bt_mesh_vendor_srv_op, &(srv)->pub,              \
			     (srv), &_bt_mesh_vendor_srv_cb)

/**
 * @brief Store model handlers in the server context
 *
 * This function should be called before the model is registered in the
 * composition data.
 *
 * @param srv      Vendor Server model
 * @param handlers Callbacks for the model instance
 */
static inline void bt_mesh_vendor_srv_set_handlers(struct bt_mesh_vendor_srv *srv,
                                                 const struct bt_mesh_vendor_srv_handlers *handlers)
{
    /* Cannot directly modify const pointer, use const_cast equivalent in C */
    *(const struct bt_mesh_vendor_srv_handlers **)&srv->handlers = handlers;
}

/**
 * @brief Send a status message
 *
 * @param srv Vendor Server model
 * @param ctx Message context to send with, or NULL to publish
 * @param rsp Vendor status message to be sent
 * @return 0 on success, or negative error code otherwise
 */
int bt_mesh_vendor_srv_status_send(struct bt_mesh_vendor_srv *srv,
                                   struct bt_mesh_msg_ctx *ctx,
                                   struct bt_mesh_vendor_status *rsp);



/**
 * @brief server model send message
 *
 * @param srv Vendor Server model
 * @param ctx Message context to send with, or NULL to publish
 * @param rsp Vendor status message to be sent
 * @return 0 on success, or negative error code otherwise
 */
int bt_mesh_vendor_srv_node_details_send(struct bt_mesh_vendor_srv *srv,
			   struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_status *value);


/**
 * @brief Send a meter data message
 *
 * @param srv Vendor Server model
 * @param ctx Message context to send with, or NULL to publish
 * @param rsp Vendor status message to be sent
 * @return 0 on success, or negative error code otherwise
 */
int bt_mesh_vendor_srv_meter_data_send(struct bt_mesh_vendor_srv *srv,
                                   struct bt_mesh_msg_ctx *ctx,
                                   struct bt_mesh_vendor_status *rsp);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* VND_SRV_H__ */
