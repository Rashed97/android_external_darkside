/**
 * Copyright (C) 2015-2016 Guillaume Delugré
 * Copyright (C) 2017 Rashed Abdel-Tawab (rashed@linux.com)
 * Copyright (C) 2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <asm-generic/ioctl.h>
#include <keyutils.h>
#include <libgen.h>

/* Include kernel definitions */
// TODO: find a some way to import these directly from the kernel
#include "kernel/pfk.h"
#include "kernel/ext4_crypto.h"
#include "kernel/ext4.h"

/* Import EXT4 encryption related definitions that for some reason AREN'T
 * defined in the kernel */
#include "ext4_crypto_config.h"

#define UNUSED __attribute__((unused))
#define VERBOSE_PRINT(opts, format, args...) ({      \
    if ( opts.verbose )                              \
        fprintf(stderr, format "\n", ##args);        \
    })                                               \
