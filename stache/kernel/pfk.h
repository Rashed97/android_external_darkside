/* Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef PFK_H_
#define PFK_H_

/*
 * Definitions imported from kernel/include/linux/pfk.h
 */

#define PFK_AES_256_XTS_KEY_SIZE 64
#define PFK_MAX_KEY_SIZE 64

/* This is passed in from userspace into the kernel keyring */
struct ext4_encryption_key {
        __u32 mode;
        char raw[PFK_MAX_KEY_SIZE];
        __u32 size;
} __attribute__((__packed__));

#endif /* PFK_H_ */
