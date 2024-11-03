#include "cycle_flash_system.h"

cfs_system_handle_t object = {
    .name = "object",
    .address = 0x000000,
    .sector_count = 4,
    .data_size = 20,
    .type = CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE,
};
