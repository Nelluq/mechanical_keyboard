/**
 * MIT License
 * 
 * Copyright (c) 2020 Cullen Jemison
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "flash_store.h"

#include <string.h>

#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/memorymap.h>

#define FLASH_STORE_BASE_ADDR (FLASH_BASE + (FLASH_PAGE_SIZE * (FLASH_NUM_PAGES - 1)))
#define HALF_WORD_SIZE sizeof(uint16_t)

void flash_store_write(uint16_t addr, void *data, uint16_t size) {
    if ((addr >= FLASH_PAGE_SIZE) || (addr + size > FLASH_PAGE_SIZE)) {
        return;
    }

    uint32_t flash_addr = FLASH_STORE_BASE_ADDR + addr;

    // read the whole page before erasing
    uint8_t buf[FLASH_PAGE_SIZE];  // temporarily allocated so that this huge buffer doesn't stick around all the time
    flash_store_read(0, buf, FLASH_PAGE_SIZE);
    memcpy(&buf[addr], data, size);  // copy new data into the buffer

    flash_unlock();
    flash_erase_page(FLASH_STORE_BASE_ADDR);
    for (uint16_t write_idx = 0; write_idx < FLASH_PAGE_SIZE; write_idx += HALF_WORD_SIZE) {
        flash_program_half_word(flash_addr + write_idx, *(uint16_t*)&buf[write_idx]);
    }
    flash_lock();
}

void flash_store_read(uint16_t addr, void *data, uint16_t size) {
    if ((addr >= FLASH_PAGE_SIZE) || (addr + size > FLASH_PAGE_SIZE)) {
        return;
    }
    uint32_t flash_addr = FLASH_STORE_BASE_ADDR + addr;
    memcpy(data, (uint32_t*)flash_addr, size);
}
