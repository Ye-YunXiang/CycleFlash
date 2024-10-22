 /** \file flash_filesystem.h
 * \brief A demo implementation of flash filesystem for Nexus and Product Code
 * \author Angaza
 * \copyright 2021 Angaza, Inc.
 * \license This file is released under the MIT license
 *
 * The above copyright notice and license shall be included in all copies
 * or substantial portions of the Software.
 *
 * This module uses the Zephyr "NVS" flash filesystem module to set up
 * two separate 'flash filesystems', one for Nexus persistent storage,
 * one for Product persistent storage.
 *
 * This isolation helps prevent changes to product or Nexus NV storage
 * (sizes, frequency of writes, etc) from negatively impacting system
 * behavior elsewhere.
 *
 * If not using Zephyr NVS to provide persistent storage, modify this file
 * to use a different method of reading and writing to NV for product and
 * Nexus code.
 */

#ifndef __CFS_DEMO__H__
#define __CFS_DEMO__H__

#include <stdbool.h>
#include <stdint.h>


bool flash_filesystem_init(void);

int flash_filesystem_write_product_nv(uint32_t id, void* data, uint32_t len);

int flash_filesystem_read_product_nv(uint32_t id, void* data, uint32_t len);

bool flash_filesystem_erase_product_nv(void);

uint32_t flash_filesystem_product_current_id_get(void);

uint32_t flash_filesystem_product_current_valid_id_get(void);

bool flash_filesystem_product_delete_object(void);


#endif // __CFS_DEMO__H__
