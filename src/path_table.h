#ifndef __PATH_TABLE_H_
#define __PATH_TABLE_H_

#include <zephyr/kernel.h>

#define NODE_INFO_TABLE_SIZE 10
#define NODE_INFO_TIMEOUT (60 * 60)  // 1 hour in seconds

typedef struct {
    uint64_t serial_number;
    uint16_t mesh_address;
    uint32_t timestamp;
} node_info_t;


#endif