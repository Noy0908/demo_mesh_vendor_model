#include <zephyr/kernel.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h> 

#include "node_app.h"
#include "model_handler.h"
#include "nus_setting_app.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(model_handler, CONFIG_BT_MESH_MODEL_LOG_LEVEL);

#define MAX_DELAY_SECONDS 100


node_info_t node_details = {
    .serial_number = 0,
    .capacity = 0,
    .quality = 0,
    .mesh_address = 0,
};

static node_table_t node_table[NODE_INFO_TABLE_SIZE];

/* Set up a repeating delayed work to publish node details to All Nodes. */
static struct k_work_delayable node_update_work;




void node_path_table_init(void) 
{
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) 
    {
        memset(&node_table[i], 0, sizeof(node_table_t));
    }
}


void update_node_path_table(node_info_t new_node) 
{
    int64_t current_time = k_uptime_get();

    // Try to find existing entry
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) 
    {
        if (node_table[i].node_info.serial_number == new_node.serial_number) 
        {
            node_table[i].node_info = new_node;
            node_table[i].timestamp = current_time;
            return;
        }
    }

    // Try to find an empty slot
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) 
    {
        if (node_table[i].node_info.serial_number == 0) 
        {
            node_table[i].node_info = new_node;
            node_table[i].timestamp = current_time;
            return;
        }
    }

    // If table is full, optionally replace the oldest entry
    int oldest_index = 0;
    int64_t oldest_time = node_table[0].timestamp;
    for (int i = 1; i < NODE_INFO_TABLE_SIZE; ++i) 
    {
        if (node_table[i].timestamp < oldest_time) 
        {
            oldest_time = node_table[i].timestamp;
            oldest_index = i;
        }
    }

    node_table[oldest_index].node_info = new_node;
    node_table[oldest_index].timestamp = current_time;
}



void purge_obsolete_node_info(void) 
{
    int64_t now = k_uptime_seconds();

    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) 
    {
        if (node_table[i].node_info.serial_number != 0 &&
            (now - node_table[i].timestamp) > NODE_INFO_TIMEOUT) 
        {
            // Mark entry as empty
            memset(&node_table[i], 0, sizeof(node_table_t));
        }
    }
}


uint16_t get_remote_node_addr(uint64_t serial_number)
{
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) 
    {
        if (node_table[i].node_info.serial_number == serial_number) 
        {
            return node_table[i].node_info.mesh_address;
        }
    }
    return 0; // Not found
}


int publish_node_details(void)
{
    if (!bt_mesh_is_provisioned()) 
    {
        LOG_ERR("Mesh is not provisioned, cannot publish node details.");
        return -ENOTCONN;
    }
    
    // Publish the node details to all nodes
    int err = vendor_model_publish_messages(&node_details, sizeof(node_info_t));
    if (err) 
    {
        LOG_ERR("Failed to publish node details: %d", err);
        return err;
    }

    LOG_INF("Node details published successfully.");
    return 0;
}

static void update_node_details(struct k_work *work)
{
    // Remove obsolete node info entries every 1h
    purge_obsolete_node_info();     

    publish_node_details();
   
    // Reschedule for 60 minutes later
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    k_work_schedule(dwork, K_MINUTES(60)); 
}


void node_app_init(void) 
{
    node_details.serial_number = get_device_sn(); // Load device serial number from settings

    // start work queue with random delay up to 100s
    uint8_t delay_s = (rand() % (MAX_DELAY_SECONDS + 1));
    k_timeout_t delay = K_SECONDS(delay_s);
    // Initialize the node path table
    node_path_table_init();

    // Initialize the delayed work for publishing node details
    k_work_init_delayable(&node_update_work, update_node_details);

    // Schedule the first publication of node details
    k_work_schedule(&node_update_work, delay);
}