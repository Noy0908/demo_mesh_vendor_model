#ifndef __NODE_APP_H_
#define __NODE_APP_H_

#include <zephyr/kernel.h>



#define NODE_INFO_TABLE_SIZE            10
#define NODE_INFO_TIMEOUT               (60 * 60)  // 1 hour in seconds

typedef struct {
    uint64_t serial_number;
    uint16_t mesh_address;
    uint8_t capacity;  // Capacity for cellular network
    uint8_t quality;    // Status of the node (e.g., online, offline)  
} node_info_t;


typedef struct {
    node_info_t node_info;
    uint32_t timestamp;
} node_table_t;

extern node_info_t node_details;

extern void node_app_init(void);

extern int publish_node_details(void);

extern void node_path_table_init(void);

extern void update_node_path_table(node_info_t new_node);

extern uint16_t get_remote_node_addr(uint64_t serial_number);


#endif