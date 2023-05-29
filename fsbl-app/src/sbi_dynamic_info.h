/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SBI_DYNAMIC_INFO_H__
#define __SBI_DYNAMIC_INFO_H__

/*
 * Compatible with def in 'include/sbi/fw_dynamic.h ' in OpenSBI v1.2:
 * https://github.com/riscv-software-src/opensbi/blob/v1.2/include/sbi/fw_dynamic.h
 */

#define FW_DYNAMIC_INFO_MAGIC_VALUE		0x4942534f
#define FW_DYNAMIC_INFO_VERSION_2		0x2

/* Possible next mode values */
#define FW_DYNAMIC_INFO_NEXT_MODE_U		0x0
#define FW_DYNAMIC_INFO_NEXT_MODE_S		0x1
#define FW_DYNAMIC_INFO_NEXT_MODE_M		0x3

#define FW_DYNAMIC_INFO_BOOT_HART_DEFAULT	-1

/* Representation dynamic info expected by OpenSBI */
struct fw_dynamic_info {
	unsigned long magic;
	unsigned long version;
	unsigned long next_addr;
	unsigned long next_mode;
	unsigned long options;
	unsigned long boot_hart;
} __packed;

/*
 * Compatible with def in 'include/sbi/sbi_scratch.h ' in OpenSBI v1.2:
 * https://github.com/riscv-software-src/opensbi/blob/v1.2/include/sbi/sbi_scratch.h
 */
enum sbi_dynamic_info_options {
	/** Disable prints during boot */
	SBI_SCRATCH_NO_BOOT_PRINTS = (1 << 0),
	/** Enable runtime debug prints */
	SBI_SCRATCH_DEBUG_PRINTS = (1 << 1),
};

#endif /* __SBI_DYNAMIC_INFO_H__ */
