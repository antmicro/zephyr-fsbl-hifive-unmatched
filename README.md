# Zephyr First Stage Boot Loader (FSBL) for HiFive Unmatched
Copyright (c) 2023 [Antmicro](https://www.antmicro.com)

This application is an FSBL for booting Linux on the HiFive Unmatched board.

Boot stages:
1. Zephyr FSBL
    - loads OpenSBI, the Linux kernel, and the device tree at specific addresses in memory
2. OpenSBI (jump target)
    - prepares the SBI interface and jumps to Linux
3. Linux

## Dependencies

Setup Python dependencies for the project:
<!-- name="python-deps" -->
```
pip3 install -r requirements.txt
```

Install Zephyr dependencies:
<!-- name="zephyr-deps" -->
```
apt-get update
apt-get install -y git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
                   python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils \
                   file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 curl
```

Setup Zephyr SDK:
<!-- name="zephyr-sdk-setup" -->
```
mkdir -p zephyr-sdk
pushd zephyr-sdk
curl -kL https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/zephyr-sdk-0.16.1_linux-x86_64.tar.xz | tar xJ --strip 1
./setup.sh -t all -h -c
export ZEPHYR_SDK_INSTALL_DIR=$(pwd)
popd
```

## Repository initialization

To build the project or work on it, clone the repository and initialize it:

<!-- name="zephyr-init" -->
```
west init -l fsbl-app
west update
python3 -m pip install -r zephyr/scripts/requirements.txt
```

## Building the binary

To build the binary, run the `west build` command:

<!-- name="zephyr-build" -->
```
west build -b hifive_unmatched fsbl-app
```

You can find the produced binary at `build/zephyr/zephyr.bin`.

## Demo setup

To run the demo, you will also need to build the following:
- Linux kernel
- Devicetree
- OpenSBI
- Rootfs (It will be embedded into the Linux kernel image)

All of them are prepared in buildroot, a tool that generates embedded Linux systems.
You can set it up as follows:

0. Install buildroot dependencies (see https://buildroot.org/downloads/manual/manual.html#requirement):

<!-- name="buildroot-deps" -->
```
apt-get update
apt-get install -y git sed make binutils build-essential diffutils gcc g++ bash patch gzip bzip2 \
                   perl tar cpio unzip rsync file bc findutils python3 wget
```

1. Clone the buildroot repository with tag `2023.02`:

<!-- name="buildroot-clone" -->
```
git clone --depth 1 git://git.buildroot.net/buildroot -b 2023.02
```

2. Configure and start building images:

<!-- name="buildroot-build" -->
```
pushd buildroot
make BR2_EXTERNAL=../buildroot-cfg/ fsbl_defconfig
make
popd
```

The produced binaries will be stored in the `buildroot/output/images` directory.

### SD card setup

To run the demo on HiFive Unmatched the SD card has to be carefully prepared.
The ZSBL (Zeroth Stage Bootloader) contained in ROM is responsible for loading FSBL into L2 LIM.
It searches for it on a partition with GUID type `5B193300-FC78-40CD-8002-E86C45580B47`.
The whole partition is loaded into memory and then ZSBL jumps into it.

The FSBL expects the partition with ext2 file system at sector 2082.
It searches for files which are then loaded into main memory at specific offsets.

NOTE: This guide assumes that the SD card is visible as `/dev/sdc`.
Before running these commands, verify the path to the SD card on your PC and change `/dev/sdc` accordingly.

1. Prepare partitions on the SD card
    - clear the card and create partitions for FSBL and ext2 file system:

    ```
    sudo sgdisk -g --clear -a 1 \
      --new=1:34:2081    --change-name=1:fsbl --typecode=1:5B193300-FC78-40CD-8002-E86C45580B47 \
      --new=2:2082:+127M --change-name=2:boot --typecode=2:0x8300 \
      /dev/sdc
    ```

    - format the partition to the ext2 file system

    ```
    sudo mkfs.ext2 /dev/sdc2 -b 4K -I 128 -O ^ext_attr,^resize_inode,^dir_index,^sparse_super,^large_file
    ```

2. Write the FSBL binary to the SD card

    ```
    sudo dd if=zephyr.bin of=/dev/sdc seek=34
    ```

3. Copy files that will be loaded by the FSBL
    - `opensbi.bin` -- OpenSBI binary
    - `hifive-unmatched-a00.dtb` -- device tree blob
    - `Image` -- Linux kernel image (uncompressed)

    ```
    mkdir mnt
    sudo mount /dev/sdc2 mnt
    sudo cp path/to/fw_jump.bin mnt/opensbi.bin
    sudo cp path/to/dtb mnt/hifive-unmatched-a00.dtb
    sudo cp path/to/Image mnt/Image
    sudo umount mnt
    ```

### Additional resources
* [HiFive Unmatched board](https://www.sifive.com/boards/hifive-unmatched)
* [OpenSBI](https://github.com/riscv-software-src/opensbi)
