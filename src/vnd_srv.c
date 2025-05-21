/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "../include/vnd_srv.h"

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

	LOG_DBG("Received SET message, data length %d", buf->len);

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

static int handle_get(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		     struct net_buf_simple *buf)
{
	LOG_DBG("Received GET message");
	struct bt_mesh_vendor_srv *srv = model->rt->user_data;

	net_buf_simple_reset(&srv->status_msg);
	struct bt_mesh_vendor_status rsp = {
		.buf = &srv->status_msg
	};

	int err = srv->handlers->get(srv, ctx, &rsp);

	/* Send response only if handler returned success */
	if (err == 0) {
		bt_mesh_vendor_srv_status_send(srv, ctx, &rsp);
	}

	return 0;
}

const struct bt_mesh_model_op _bt_mesh_vendor_srv_op[] = {
	{ BT_MESH_VENDOR_OP_SET, 0, handle_set },
	{ BT_MESH_VENDOR_OP_GET, 0, handle_get },
	BT_MESH_MODEL_OP_END,
};

static int vendor_srv_init(const struct bt_mesh_model *model)
{
	struct bt_mesh_vendor_srv *srv = model->rt->user_data;

	srv->model = model;
	net_buf_simple_init_with_data(&srv->pub_msg, srv->buf, sizeof(srv->buf));
	net_buf_simple_init_with_data(&srv->status_msg, srv->status_buf_data, sizeof(srv->status_buf_data));
	bt_mesh_model_msg_init(&srv->pub_msg, BT_MESH_VENDOR_OP_STATUS);
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
