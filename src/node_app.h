#ifndef __NODE_APP_H_
#define __NODE_APP_H_

#include <zephyr/kernel.h>

#define DEVICE_SN_SIZE                  6

#define NODE_INFO_TABLE_SIZE            10
#define NODE_INFO_TIMEOUT               (60 * 60)  // 1 hour in seconds

typedef struct {
    uint64_t serial_number;
    uint16_t mesh_address;
    uint32_t timestamp;
} node_info_t;



extern uint16_t get_remote_node_addr(uint64_t serial_number);


#endif