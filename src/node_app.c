#include <zephyr/kernel.h>
#include <stdint.h>
#include <string.h>

#include "node_app.h"


static node_info_t node_info_table[NODE_INFO_TABLE_SIZE];






void node_path_table_init(void) {
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) {
        node_info_table[i].serial_number = 0;
    }
}


void update_node_path_table(uint64_t serial_number, uint16_t src_address) {
    int64_t current_time = k_uptime_get();

    // Try to find existing entry
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) {
        if (node_info_table[i].serial_number == serial_number) {
            node_info_table[i].mesh_address = src_address;
            node_info_table[i].timestamp = current_time;
            return;
        }
    }

    // Try to find an empty slot
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) {
        if (node_info_table[i].serial_number == 0) {
            node_info_table[i].serial_number = serial_number;
            node_info_table[i].mesh_address = src_address;
            node_info_table[i].timestamp = current_time;
            return;
        }
    }

    // If table is full, optionally replace the oldest entry
    int oldest_index = 0;
    int64_t oldest_time = node_info_table[0].timestamp;
    for (int i = 1; i < NODE_INFO_TABLE_SIZE; ++i) {
        if (node_info_table[i].timestamp < oldest_time) {
            oldest_time = node_info_table[i].timestamp;
            oldest_index = i;
        }
    }

    node_info_table[oldest_index].serial_number = serial_number;
    node_info_table[oldest_index].mesh_address = src_address;
    node_info_table[oldest_index].timestamp = current_time;
}



void purge_obsolete_node_info(void) {
    int64_t now = k_uptime_seconds();

    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) {
        if (node_info_table[i].serial_number != 0 &&
            (now - node_info_table[i].timestamp) > NODE_INFO_TIMEOUT) {

            // Mark entry as empty
            node_info_table[i].serial_number = 0;
            node_info_table[i].mesh_address = 0;
            node_info_table[i].timestamp = 0;
        }
    }
}


uint16_t get_remote_node_addr(uint64_t serial_number) {
    for (int i = 0; i < NODE_INFO_TABLE_SIZE; ++i) {
        if (node_info_table[i].serial_number == serial_number) {
            return node_info_table[i].mesh_address;
        }
    }
    return 0; // Not found
}
