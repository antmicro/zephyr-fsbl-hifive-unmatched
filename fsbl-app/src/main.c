/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/arch_interface.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ZephyrFSBL, LOG_LEVEL_INF);

#define CPU_STACK_SIZE 1024

K_THREAD_STACK_DEFINE(cpu1_stack, CPU_STACK_SIZE);
K_THREAD_STACK_DEFINE(cpu2_stack, CPU_STACK_SIZE);
K_THREAD_STACK_DEFINE(cpu3_stack, CPU_STACK_SIZE);
K_THREAD_STACK_DEFINE(cpu4_stack, CPU_STACK_SIZE);

struct z_thread_stack_element (*stack[])[1024] = {
	&cpu1_stack,
	&cpu2_stack,
	&cpu3_stack,
	&cpu4_stack,
};

int load_file(const char *path, void *offset)
{
	struct fs_file_t file;
	struct fs_dirent de;
	int ret = 0;
	ssize_t read;

	fs_file_t_init(&file);

	ret = fs_stat(path, &de);
	if (ret < 0) {
		LOG_ERR("Stat '%s' failed: %d", path, ret);
	}

	LOG_INF("'%s' loading %ld bytes at %p", path, de.size, offset);

	ret = fs_open(&file, path, FS_O_READ);
	if (ret < 0) {
		LOG_ERR("Open '%s' failed: %d", path, ret);
		return ret;
	}

	read = fs_read(&file, offset, de.size);
	if (read < 0) {
		ret = read;
	}

	if (read < de.size) {
		LOG_ERR("Couldn't load whole file (only %ld bytes)", read);
		ret = -1;
	}

	fs_close(&file);
	return ret;
}

#define SBI_FILE   "opensbi.bin"
#define IMAGE_FILE "Image"
#define FDT_FILE   "hifive-unmatched-a00.dtb"

#define SBI_ADDR   0x80000000
#define IMAGE_ADDR 0x80200000
#define FDT_ADDR   0x88000000

int load_binaries(void)
{
	int ret;
	static struct fs_mount_t mp = {
		.type = FS_EXT2,
		.flags = 0,
		.storage_dev = "SDMMC",
		.mnt_point = "/ext",
	};

	ret = fs_mount(&mp);
	if (ret < 0) {
		LOG_ERR("Mount failed: %d", ret);
		return ret;
	}

	ret = load_file("/ext/"SBI_FILE, (void *)SBI_ADDR);
	if (ret < 0) {
		LOG_ERR("Failed to load '%s': %d", SBI_FILE, ret);
		return ret;
	}

	ret = load_file("/ext/"IMAGE_FILE, (void *)IMAGE_ADDR);
	if (ret < 0) {
		LOG_ERR("Failed to load '%s': %d", IMAGE_FILE, ret);
		return ret;
	}

	ret = load_file("/ext/"FDT_FILE, (void *)FDT_ADDR);
	if (ret < 0) {
		LOG_ERR("Failed to load '%s': %d", FDT_FILE, ret);
		return ret;
	}

	ret = fs_unmount(&mp);
	if (ret < 0) {
		LOG_ERR("Unmount failed: %d", ret);
		return ret;
	}

	return 0;
}

typedef void (*boot_fun_t)(unsigned long, unsigned long);

FUNC_NORETURN void boot_fn(void *arg)
{
	boot_fun_t boot_f = (boot_fun_t)SBI_ADDR;

	arch_irq_lock();
	boot_f((unsigned long)arg, (unsigned long)FDT_ADDR);

	LOG_ERR("Boot failed for thread %ld", (uintptr_t)arg);
	__builtin_unreachable();
}

int main(void)
{
	LOG_PRINTK("Zephyr fsbl-app\n");

	if (load_binaries() < 0) {
		LOG_ERR("Failed to load binaries");
		return 0;
	}

	/* Jump to OpenSBI */
	for (long i = 1; i < CONFIG_MP_NUM_CPUS; ++i) {
		arch_start_cpu(i, *stack[i-1], CPU_STACK_SIZE, boot_fn, (void *)i);
	}
	boot_fn(0);
}
