/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "../include/vnd_srv.h"
#include "model_utils.h"

LOG_MODULE_REGISTER(vnd_srv, CONFIG_BT_MESH_MODEL_LOG_LEVEL);

static int handle_set(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		     struct net_buf_simple *buf)
{
	if (buf->len > BT_MESH_VENDOR_MSG_MAXLEN_STATUS) {
		return -EMSGSIZE;
	}

	struct bt_mesh_vendor_srv *srv = model->rt->user_data;
	struct bt_mesh_vendor_set set = {
		.buf = buf
	};

	LOG_DBG("Received SET message from 0x%04X, data length %d", ctx->addr, buf->len);

	if (srv->handlers && srv->handlers->set) {
		net_buf_simple_reset(&srv->status_msg);
		struct bt_mesh_vendor_status rsp = {
			.buf = &srv->status_msg
		};

		int err = srv->handlers->set(srv, ctx, &set, &rsp);

		if (err == 0) {
			bt_mesh_vendor_srv_status_send(srv, ctx, &rsp);
		}
	}

	return 0;
}

static int handle_set_unack(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		     struct net_buf_simple *buf)
{
	if (buf->len > BT_MESH_VENDOR_MSG_MAXLEN_STATUS) {
		return -EMSGSIZE;
	}

	struct bt_mesh_vendor_srv *srv = model->rt->user_data;
	struct bt_mesh_vendor_set set = {
		.buf = buf
	};

	LOG_DBG("Received SET UNACK message, data length %d", buf->len);

	if (srv->handlers && srv->handlers->set) {
		net_buf_simple_reset(&srv->status_msg);
		struct bt_mesh_vendor_status rsp = {
			.buf = &srv->status_msg
		};

		/* Call the same handler but don't send any response */
		srv->handlers->set(srv, ctx, &set, &rsp);
	}

	return 0;
}

static int handle_get(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		     struct net_buf_simple *buf)
{
	struct bt_mesh_vendor_srv *srv = model->rt->user_data;
	struct bt_mesh_vendor_get get = { 0 };
	bool has_len = (buf->len == BT_MESH_VENDOR_MSG_MAXLEN_GET);

	/* Check if the length parameter is included in the message */
	if (has_len) {
		get.length = net_buf_simple_pull_le16(buf);
		LOG_DBG("GET message with length parameter: %u", get.length);
	} else {
		LOG_DBG("GET message without length parameter");
	}

	bt_mesh_vendor_get_type_t get_type = net_buf_simple_pull_u8(buf); // Pull the type byte, if present

	net_buf_simple_reset(&srv->status_msg);
	struct bt_mesh_vendor_status rsp = {
		.buf = &srv->status_msg
	};

	int err = srv->handlers->get(srv, ctx, has_len ? &get : NULL, &rsp);
	/* Send response only if handler returned success */
	if (err == 0) {
		switch (get_type) {
		case BT_MESH_VENDOR_GET_TYPE_STATUS:
			LOG_DBG("GET request for status");
			return bt_mesh_vendor_srv_status_send(srv, ctx, &rsp);
			break;
		case BT_MESH_VENDOR_GET_TYPE_NODE_DETAILS:
			LOG_DBG("GET request for node details");
			return bt_mesh_vendor_srv_pub(srv, ctx, &rsp);
			break;
		case BT_MESH_VENDOR_GET_TYPE_METER_DATA:
			LOG_DBG("GET request for meter data");
			return bt_mesh_vendor_srv_meter_data_send(srv, ctx, &rsp);
			break;
		default:
			LOG_WRN("Unknown GET request type: %d", get_type);
			return -EINVAL; /* Invalid request type */
		}
	}

	return 0;
}

// static int handle_get_node_details(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
// 		     struct net_buf_simple *buf)
// {
// 	struct bt_mesh_vendor_srv *srv = model->rt->user_data;
// 	struct bt_mesh_vendor_get get = { 0 };
// 	bool has_len = (buf->len == BT_MESH_VENDOR_MSG_MAXLEN_GET);

// 	/* Check if the length parameter is included in the message */
// 	if (has_len) {
// 		get.length = net_buf_simple_pull_le16(buf);
// 		LOG_DBG("GET message with length parameter: %u", get.length);
// 	} else {
// 		LOG_DBG("GET message without length parameter");
// 	}

// 	net_buf_simple_reset(&srv->status_msg);
// 	struct bt_mesh_vendor_status rsp = {
// 		.buf = &srv->status_msg
// 	};

// 	int err = srv->handlers->get(srv, ctx, has_len ? &get : NULL, &rsp);

// 	/* Send response only if handler returned success */
// 	if (err == 0) {
// 		return bt_mesh_vendor_srv_status_send(srv, ctx, &rsp);
// 	}

// 	return 0;
// }

const struct bt_mesh_model_op _bt_mesh_vendor_srv_op[] = {
	{ BT_MESH_VENDOR_OP_SET, 0, handle_set },
	{ BT_MESH_VENDOR_OP_SET_UNACK, 0, handle_set_unack },
	{ BT_MESH_VENDOR_OP_GET, 0, handle_get },
	{ BT_MESH_VENDOR_OP_GET_NODE_DETAILS, 0, handle_get },
	{ BT_MESH_VENDOR_OP_GET_METER_DATA, 0, handle_get },
	BT_MESH_MODEL_OP_END,
};

static int vendor_srv_init(const struct bt_mesh_model *model)
{
	struct bt_mesh_vendor_srv *srv = model->rt->user_data;

	srv->model = model;
	srv->pub.msg = &srv->pub_msg;
	net_buf_simple_init_with_data(&srv->pub_msg, srv->buf, sizeof(srv->buf));
	net_buf_simple_init_with_data(&srv->status_msg, srv->status_buf_data, sizeof(srv->status_buf_data));
	// bt_mesh_model_msg_init(&srv->pub_msg, BT_MESH_VENDOR_OP_STATUS);
	net_buf_simple_reset(&srv->status_msg);

	/* Make sure get set handlers are set*/
	if (!srv->handlers || !srv->handlers->get || !srv->handlers->set) {
		LOG_ERR("Get or set handler not set");
		return -EINVAL;
	}

	return 0;
}

static void vendor_srv_reset(const struct bt_mesh_model *model)
{
	struct bt_mesh_vendor_srv *srv = model->rt->user_data;

	net_buf_simple_reset(&srv->status_msg);
	net_buf_simple_reset(&srv->pub_msg);
	bt_mesh_model_msg_init(&srv->pub_msg, BT_MESH_VENDOR_OP_STATUS);
}

const struct bt_mesh_model_cb _bt_mesh_vendor_srv_cb = {
	.init = vendor_srv_init,
	.reset = vendor_srv_reset,
};

int bt_mesh_vendor_srv_status_send(struct bt_mesh_vendor_srv *srv,
                                   struct bt_mesh_msg_ctx *ctx,
                                   struct bt_mesh_vendor_status *rsp)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_STATUS, srv->status_msg.len);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_STATUS);

	if (rsp->buf->len > 0) {
		net_buf_simple_add_mem(&msg, rsp->buf->data, rsp->buf->len);
	}

	LOG_DBG("Sending STATUS message, data length %d", rsp->buf->len);

	if (ctx) {
		return bt_mesh_model_send(srv->model, ctx, &msg, NULL, NULL);
	} else {
		return bt_mesh_model_publish(srv->model);
	}
}


int bt_mesh_vendor_srv_pub(struct bt_mesh_vendor_srv *srv,
			   struct bt_mesh_msg_ctx *ctx,
			   struct bt_mesh_vendor_status *value)
{
	int err;

	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_STATUS_NODE_DETAILS,
				 value->buf->len);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_STATUS_NODE_DETAILS);

	if (value->buf->len > 0) {
		net_buf_simple_add_mem(&msg, value->buf->data, value->buf->len);
	}
	// LOG_DBG("Publishing NODE DETAILS message, data length %d", value->buf->len);
	PRINT_HEX("server publish:", value->buf->data, value->buf->len);

	err = bt_mesh_msg_send(srv->model, ctx, &msg);
	if (err) {
		return err;
	}
	return 0;
}



int bt_mesh_vendor_srv_meter_data_send(struct bt_mesh_vendor_srv *srv,
                                   struct bt_mesh_msg_ctx *ctx,
                                   struct bt_mesh_vendor_status *rsp)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_STATUS_METER_DATA, srv->status_msg.len);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_STATUS_METER_DATA);

	if (rsp->buf->len > 0) {
		net_buf_simple_add_mem(&msg, rsp->buf->data, rsp->buf->len);
	}

	LOG_DBG("Sending METER DATA message, data length %d", rsp->buf->len);

	if (ctx) {
		return bt_mesh_model_send(srv->model, ctx, &msg, NULL, NULL);
	} else {
		return bt_mesh_model_publish(srv->model);
	}
}

