/*
 * This file is part of the CycleFlash Library.
 *
 * Copyright (c) 2024, YeYunXiang, <poetrycloud@foxmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the definitions head file for this library.
 * Created on: 2024-7-26
 */
// Encoding:GB2312

#include "cycleflash.h"
#include "cycleflash_port.h" 

/*数据对象的头指针*/
cfs_object_linked_list *cfs_system_object_head = NULL;
cfs_object_linked_list *cfs_system_object_tail = NULL;


// 添加一个数据块对象
bool cfs_filesystem_add_object(void *object_pointer, \
    enum cycle_flash_filesystem_object_type object_type) 
{
    cfs_object_linked_list *new_node = 
        (cfs_object_linked_list *)CFS_MALLOC(sizeof(cfs_object_linked_list));

    if (new_node == NULL) {
        /* Allocation failure */
        return false;
    }

    if (cfs_system_object_head == NULL)
    {
        cfs_system_object_head = new_node;
    }
    else
    {
        cfs_system_object_tail->next = new_node;
    }

    new_node->object_handle = object_pointer;
    new_node->object_type = object_type;
    new_node->data_id = 0;
    cfs_system_object_tail = new_node;

    return true;
}

// 查找链表位置
void *cfs_filesystem_find_linked_list(const void *object_pointer) 
{
    cfs_object_linked_list *temp_pointer = cfs_system_object_head;
    
    while (temp_pointer != NULL) 
    {
        if (temp_pointer->object_handle == object_pointer) 
        {
            return temp_pointer;
        }
        temp_pointer = temp_pointer->next;
    }
    /*No object found*/
    return NULL;
}


bool cfs_filesystem_object_init(void *temp_object, enum cycle_flash_filesystem_object_type)
{
    
}

