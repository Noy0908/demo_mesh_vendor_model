/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "../include/vnd_cli.h"
#include <model_utils.h>

LOG_MODULE_REGISTER(vnd_cli, CONFIG_BT_MESH_MODEL_LOG_LEVEL);

static int handle_status(const struct bt_mesh_model *model, \
			 struct bt_mesh_msg_ctx *ctx, \
			 struct net_buf_simple *buf)
{
	struct bt_mesh_vendor_cli *cli = model->rt->user_data;
	struct bt_mesh_vendor_status status;
	struct bt_mesh_vendor_status *rsp;

	LOG_DBG("Received STATUS message from 0x%04X,, data length %d", ctx->addr, buf->len);

	status.buf = buf;

	if (bt_mesh_msg_ack_ctx_match(&cli->ack_ctx, BT_MESH_VENDOR_OP_STATUS, ctx->addr, (void **)&rsp)) {
		rsp->buf = buf;

		bt_mesh_msg_ack_ctx_rx(&cli->ack_ctx);
	}

	if (cli->status_handler) {
		cli->status_handler(cli, ctx, &status);
	}

	return 0;
}

const struct bt_mesh_model_op _bt_mesh_vendor_cli_op[] = {
	{
		BT_MESH_VENDOR_OP_STATUS, 0, handle_status
	},
	BT_MESH_MODEL_OP_END,
};

static int vendor_cli_init(const struct bt_mesh_model *model)
{
	struct bt_mesh_vendor_cli *cli = model->rt->user_data;

	cli->model = model;
	cli->pub.msg = &cli->pub_msg;
	net_buf_simple_init_with_data(&cli->pub_msg, cli->buf, sizeof(cli->buf));
	bt_mesh_msg_ack_ctx_init(&cli->ack_ctx);

	return 0;
}

static void vendor_cli_reset(const struct bt_mesh_model *model)
{
	struct bt_mesh_vendor_cli *cli = model->rt->user_data;

	net_buf_simple_reset(cli->pub.msg);
	bt_mesh_msg_ack_ctx_reset(&cli->ack_ctx);
}

const struct bt_mesh_model_cb _bt_mesh_vendor_cli_cb = {
	.init = vendor_cli_init,
	.reset = vendor_cli_reset,
};

int bt_mesh_vendor_cli_set(struct bt_mesh_vendor_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_vendor_set *set,
			   struct bt_mesh_vendor_status *rsp)
{
	if (set && set->buf && set->buf->len > BT_MESH_VENDOR_MSG_MAXLEN_SET) {
		return -EMSGSIZE;
	}

	LOG_DBG("Sending SET message, data length %d", set->buf->len);

	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_SET, set->buf->len);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_SET);

	if (set->buf->len > 0) {
		net_buf_simple_add_mem(&msg, set->buf->data, set->buf->len);
	}

	struct bt_mesh_msg_rsp_ctx rsp_ctx = {
		.ack = &cli->ack_ctx,
		.op = BT_MESH_VENDOR_OP_STATUS,
		.user_data = rsp,
		.timeout = model_ackd_timeout_get(cli->model, ctx),
	};

	return bt_mesh_msg_ackd_send(cli->model, ctx, &msg, rsp ? &rsp_ctx : NULL);
}

int bt_mesh_vendor_cli_get(struct bt_mesh_vendor_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_vendor_get *get,
			   struct bt_mesh_vendor_status *rsp)
{
	/* Define buffer with enough space for the length parameter if present */
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_GET, BT_MESH_VENDOR_MSG_MAXLEN_GET);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_GET);

	/* Add optional length parameter if present */
	if (get) {
		LOG_DBG("Sending GET message with length parameter: %u", get->length);
		net_buf_simple_add_le16(&msg, get->length);
	} else {
		LOG_DBG("Sending GET message without length parameter");
	}

	struct bt_mesh_msg_rsp_ctx rsp_ctx = {
		.ack = &cli->ack_ctx,
		.op = BT_MESH_VENDOR_OP_STATUS,
		.user_data = rsp,
		.timeout = model_ackd_timeout_get(cli->model, ctx),
	};

	return bt_mesh_msg_ackd_send(cli->model, ctx, &msg, rsp ? &rsp_ctx : NULL);
}

int bt_mesh_vendor_cli_set_unack(struct bt_mesh_vendor_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_vendor_set *set)
{
	if (set && set->buf && set->buf->len > BT_MESH_VENDOR_MSG_MAXLEN_SET) {
		return -EMSGSIZE;
	}

	LOG_DBG("Sending SET UNACK message, data length %d", set->buf->len);

	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_SET_UNACK, set->buf->len);
	bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_SET_UNACK);

	if (set->buf->len > 0) {
		net_buf_simple_add_mem(&msg, set->buf->data, set->buf->len);
	}

	/* No acknowledgment is expected, so we use direct send */
	return bt_mesh_msg_send(cli->model, ctx, &msg);
}
