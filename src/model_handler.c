/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>

#include "../include/vnd_srv.h"
#include "../include/vnd_cli.h"
#include "model_handler.h"

LOG_MODULE_REGISTER(model_handler, CONFIG_BT_MESH_MODEL_LOG_LEVEL);

/* Sample message buffer */
static const char set_msg[] =
"Hello World- 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
static const char status_msg[] =
"Response OK- 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";

/* Forward declaration for the client status handler */
static void handle_vendor_status(struct bt_mesh_vendor_cli *cli,
				  struct bt_mesh_msg_ctx *ctx,
				  const struct bt_mesh_vendor_status *status);

/* Vendor model operation callbacks */
static int handle_vendor_set(struct bt_mesh_vendor_srv *srv,
				struct bt_mesh_msg_ctx *ctx,
				const struct bt_mesh_vendor_set *set,
				struct bt_mesh_vendor_status *rsp);

static int handle_vendor_get(struct bt_mesh_vendor_srv *srv,
				struct bt_mesh_msg_ctx *ctx,
				struct bt_mesh_vendor_status *rsp);

static const struct bt_mesh_vendor_srv_handlers vendor_srv_handlers = {
	.set = handle_vendor_set,
	.get = handle_vendor_get,
};

/* Set up a repeating delayed work to blink the DK's LEDs when attention is requested. */
static struct k_work_delayable attention_blink_work;
static bool attention;

static void attention_blink(struct k_work *work)
{
	static bool led_on;

	if (attention) {
		led_on = !led_on;
		dk_set_led(0, led_on);

		struct k_work_delayable *dwork = k_work_delayable_from_work(work);
		k_work_schedule(dwork, K_MSEC(300));
	} else {
		dk_set_led(0, 0);
	}
}

/* Health Server callback */
static void attention_on(const struct bt_mesh_model *mod)
{
	attention = true;
	k_work_schedule(&attention_blink_work, K_NO_WAIT);
}

static void attention_off(const struct bt_mesh_model *mod)
{
	attention = false;
}

static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

/**************************************************************************************************/
/* Server model instance */
static struct bt_mesh_vendor_srv vendor_srv = BT_MESH_VENDOR_SRV_INIT(&vendor_srv_handlers);

/* Server set callback */
static int handle_vendor_set(struct bt_mesh_vendor_srv *srv,
				struct bt_mesh_msg_ctx *ctx,
				const struct bt_mesh_vendor_set *set,
				struct bt_mesh_vendor_status *rsp)
{
	char data[BT_MESH_VENDOR_MSG_MAXLEN_SET + 1] = {0};
	size_t len = set->buf->len;

	/* Safety check for buffer size */
	if (len > BT_MESH_VENDOR_MSG_MAXLEN_SET) {
		len = BT_MESH_VENDOR_MSG_MAXLEN_SET;
	}

	/* Copy data for display */
	if (len > 0) {
		memcpy(data, set->buf->data, len);
		data[len] = '\0'; /* Null-terminate the string */
	}

	LOG_INF("Received SET message: \"%s\"", data);

	/* Populate the response status message */
	net_buf_simple_reset(rsp->buf);
	net_buf_simple_add_mem(rsp->buf, status_msg, strlen(status_msg));

	return 0; /* Return success to send response immediately */
}

/* Server get callback */
static int handle_vendor_get(struct bt_mesh_vendor_srv *srv,
				struct bt_mesh_msg_ctx *ctx,
				struct bt_mesh_vendor_status *rsp)
{
	LOG_INF("Received GET request");

	/* Populate the response status message */
	net_buf_simple_reset(rsp->buf);
	net_buf_simple_add_mem(rsp->buf, status_msg, strlen(status_msg));

	LOG_INF("Sending STATUS response");

	return 0; /* Return success to send response immediately */
}

/**************************************************************************************************/
/* Client model instance */
static struct bt_mesh_vendor_cli vendor_cli = BT_MESH_VND_CLI_INIT(handle_vendor_status);

/* Client status callback */
static void handle_vendor_status(struct bt_mesh_vendor_cli *cli,
				  struct bt_mesh_msg_ctx *ctx,
				  const struct bt_mesh_vendor_status *status)
{
	char data[BT_MESH_VENDOR_MSG_MAXLEN_STATUS + 1] = {0};
	size_t len = status->buf->len;

	if (len > BT_MESH_VENDOR_MSG_MAXLEN_STATUS) {
		len = BT_MESH_VENDOR_MSG_MAXLEN_STATUS;
	}

	if (len > 0) {
		memcpy(data, status->buf->data, len);
		data[len] = '\0'; /* Null-terminate the string */
	}

	LOG_INF("Received STATUS response: \"%s\"", data);
}


int vendor_model_send_set(const uint8_t *data, size_t len, struct bt_mesh_vendor_status *rsp)
{
	LOG_INF("Sending SET message: \"%s\"", (char *)data);

	NET_BUF_SIMPLE_DEFINE(temp_buf, BT_MESH_VENDOR_MSG_MAXLEN_SET);
	net_buf_simple_add_mem(&temp_buf, data, len);

	struct bt_mesh_vendor_set set = {
		.buf = &temp_buf
	};

	return bt_mesh_vendor_cli_set(&vendor_cli, NULL, &set, rsp);
}

int vendor_model_send_get(void)
{
	return bt_mesh_vendor_cli_get(&vendor_cli, NULL, NULL);
}

/* Handle button events */
static void button_handler(uint32_t pressed, uint32_t changed)
{
	int err;

	if (pressed & changed & BIT(DK_BTN1)) {
		/* Send SET message with "Hello World" string */
		err = vendor_model_send_set((const uint8_t *)set_msg, strlen(set_msg), NULL);
		if (err) {
			LOG_ERR("Failed to send SET message (err: %d)", err);
		}
	}

	if (pressed & changed & BIT(DK_BTN2)) {
		/* Send GET message to request status */
		err = vendor_model_send_get();
		if (err) {
			LOG_ERR("Failed to send GET message (err: %d)", err);
		}
	}
}

/**************************************************************************************************/
/* Composition data element for the server model */
static const struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(
		1, BT_MESH_MODEL_LIST(
			BT_MESH_MODEL_CFG_SRV,
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
		BT_MESH_MODEL_LIST(
			BT_MESH_MODEL_VND_SRV(&vendor_srv, &vendor_srv_handlers),
			BT_MESH_MODEL_VND_CLI(&vendor_cli))
	),
};

/* Composition data for the mesh node */
static const struct bt_mesh_comp comp = {
	.cid = BT_COMP_ID_VENDOR,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	int err;

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Failed to initialize LEDs (err: %d)", err);
	}

	err = dk_buttons_init(button_handler);
	if (err) {
		LOG_ERR("Failed to initialize buttons (err: %d)", err);
	}

	k_work_init_delayable(&attention_blink_work, attention_blink);

	return &comp;
}
