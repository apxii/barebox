/*
 * Copyright (C) 2010 Marek Belisko <marek.belisko@open-nandra.com>
 *
 * Based on a9m2440.c board init by Juergen Beisert, Pengutronix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/**
 * @file
 * @brief mini2440 Specific Board Initialization routines
 *
 */

#include <common.h>
#include <driver.h>
#include <init.h>
#include <generated/mach-types.h>
#include <partition.h>
#include <dm9000.h>
#include <nand.h>
#include <asm/armlinux.h>
#include <asm/io.h>
#include <mach/s3c24x0-iomap.h>
#include <mach/s3c24x0-nand.h>
#include <mach/s3c24xx-generic.h>

static struct memory_platform_data ram_pdata = {
	.name		= "ram0",
	.flags		= DEVFS_RDWR,
};

static struct device_d sdram_dev = {
	.id		= -1,
	.name		= "mem",
	.map_base	= CS6_BASE,
	.size		= 64 * 1024 * 1024,
	.platform_data	= &ram_pdata,
};

static struct s3c24x0_nand_platform_data nand_info = {
	.nand_timing = CALC_NFCONF_TIMING(A9M2440_TACLS, A9M2440_TWRPH0, A9M2440_TWRPH1)
};

static struct device_d nand_dev = {
	.name     = "s3c24x0_nand",
	.map_base = S3C24X0_NAND_BASE,
	.platform_data	= &nand_info,
};

/*
 * dm9000 network controller onboard
 * Connected to CS line 4 and interrupt line EINT7,
 * data width is 16 bit
 * Area 1: Offset 0x300...0x303
 * Area 2: Offset 0x304...0x307
 */
static struct dm9000_platform_data dm9000_data = {
	.iobase   = CS4_BASE + 0x300,
	.iodata   = CS4_BASE + 0x304,
	.buswidth = DM9000_WIDTH_16,
	.srom     = 1,
};

static struct device_d dm9000_dev = {
	.name     = "dm9000",
	.map_base = CS4_BASE + 0x300,
	.size     = 8,
	.platform_data = &dm9000_data,
};

static int mini2440_devices_init(void)
{
	uint32_t reg;

	reg = readl(BWSCON);

	/* CS#4 to access the network controller */
	reg &= ~0x000f0000;
	reg |=  0x000d0000;	/* 16 bit */
	writel(0x1f4c, BANKCON4);

	writel(reg, BWSCON);

	/* release the reset signal to external devices */
	reg = readl(MISCCR);
	reg |= 0x10000;
	writel(reg, MISCCR);

	register_device(&nand_dev);
	register_device(&sdram_dev);
	register_device(&dm9000_dev);
#ifdef CONFIG_NAND
	/* ----------- add some vital partitions -------- */
	devfs_del_partition("self_raw");
	devfs_add_partition("nand0", 0x00000, 0x40000, PARTITION_FIXED, "self_raw");
	dev_add_bb_dev("self_raw", NULL);

	devfs_del_partition("env_raw");
	devfs_add_partition("nand0", 0x40000, 0x20000, PARTITION_FIXED, "env_raw");
	dev_add_bb_dev("env_raw", NULL);
#endif
	armlinux_add_dram(&sdram_dev);
	armlinux_set_bootparams((void *)sdram_dev.map_base + 0x100);
	armlinux_set_architecture(MACH_TYPE_MINI2440);

	return 0;
}

device_initcall(mini2440_devices_init);

#ifdef CONFIG_S3C24XX_NAND_BOOT
void __bare_init nand_boot(void)
{
	s3c24x0_nand_load_image((void *)TEXT_BASE, 256 * 1024, 0, 512);
}
#endif

static struct device_d mini2440_serial_device = {
	.name     = "s3c24x0_serial",
	.map_base = UART1_BASE,
	.size     = UART1_SIZE,
};

static int mini2440_console_init(void)
{
	register_device(&mini2440_serial_device);
	return 0;
}

console_initcall(mini2440_console_init);

/** @page mini2440 FriendlyARM's mini2440

This system is based on a Samsung S3C2440 CPU. The card is shipped with:

- S3C2440\@400 MHz or 533 MHz (ARM920T/ARMv4T)
- 12 MHz crystal reference
- 32.768 kHz crystal reference
- SDRAM 64 MiB (one bank only)
   - HY57V561620 (two devices for 64 MiB to form a 32 bit bus)
     - 4M x 16bit x 4 Banks Mobile SDRAM
     - 8192 refresh cycles / 64 ms
     - CL2\@100 MHz
     - 133 MHz max
     - collumn address size is 9 bits
     - row address size is 13 bits
   - MT48LC16M16 (two devices for 64 MiB to form a 32 bit bus)
     - 4M x 16bit x 4 Banks Mobile SDRAM
     - commercial & industrial type
     - 8192 refresh cycles / 64 ms
     - CL2\@100 MHz
     - 133 MHz max
     - collumn address size is 9 bits
     - row address size is 13 bits
- NAND Flash 128MiB...1GiB
   - K9Fxx08
- NOR Flash (up to 22 address lines available)
   - AM29LV160DB, 2 MiB
   - SST39VF1601, 2 MiB
   - 16 bit data bus
- SD card interface, 3.3V (fixed voltage)
- Host and device USB interface, USB1.1 compliant
- UDA1341TS Audio
- DM9000 Ethernet interface
  - uses CS#4
  - uses EINT7
  - 16 bit data bus
- I2C interface, 100 KHz and 400 KHz
  - EEPROM
    - ST M24C08
    - address 0x50
- Speaker on GPB0 ("low" = inactive)
- LCD interface
- Touch Screen interface
- Camera interface
- I2S interface
- AC97 Audio-CODEC interface
- three serial RS232 interfaces (one with level converter)
- SPI interface
- JTAG interface

How to get the binary image:

Using the default configuration:

@code
make ARCH=arm mini2440_defconfig
@endcode

Build the binary image:

@code
make ARCH=arm CROSS_COMPILE=armv4compiler
@endcode

@note replace the armv4compiler with your ARM v4 cross compiler.

How to bring in \a barebox ?

First run it as a second stage bootloader. There are two known working ways to
do so:

One way is to use the "device firmware update" feature of the 'supervivi'.
 - connect a terminal application to the mini2440's serial connector
 - switch S2 to 'boot from NOR' to boot into 'supervivi'
 - connect your host to the usb device connector on the mini2440
 - switch on your mini2440
 - in 'supervivi' type q (command line) then:
@code
load ram 0x31000000 \<barebox-size\> u
@endcode
 - use a tool for DFU update (for example from openkomo) to transfer the 'barebox.bin' binary
 - then in 'supervivi' just run
@code
go 0x31000000
@endcode

A second way is to use any kind of JTAG adapter. For this case I'm using the
'JTAKkey tiny' from Amontec and OpenOCD. First you need an adapter for this
kind of Dongle as it uses a 20 pin connector with 2.54 mm grid, and the
mini2440 uses a 10 pin connector with 2 mm grid.

@code
             Amontec JTAGkey tiny               mini2440
           -------------------------------------------------------
              VREF  1   2  n.c.             VREF  1   2  VREF
            TRST_N  3   4  GND            TRST_N  3   4  SRST_N
               TDI  5   6  GND               TDI  5   6  TDO
               TMS  7   8  GND               TMS  7   8  GND
               TCK  9  10  GND               TCK  9  10  GND
              n.c. 11  12  GND
               TDO 13  14  GND
            SRST_N 15  16  GND
              n.c. 17  18  GND
              n.c. 19  20  GND
@endcode

Create a simple board description file. I did it this way:

@code
source [find interface/jtagkey-tiny.cfg]
source [find target/samsung_s3c2440.cfg]

adapter_khz 12000
@endcode

And then the following steps:
 - connect a terminal application to the mini2440's serial connector
 - connect the mini2440 to a working network
 - switch S2 to boot from NOR to boot into 'supervivi'
 - switch on your mini2440
 - run the OpenOCD daemon configured with the file shown above
 - connect to the OpenOCD daemon via 'telnet'.
 - run the following commands to download @a barebox into your target
@code
> halt
> load_image \<path to the 'barebox.bin'\> 0x31000000 bin
> resume 0x31000000
@endcode

Now @a barebox is starting from an already initialized CPU and SDRAM (done by
'supervivi').

Change to your terminal console and configure the network first. Adapt the
following settings to your network:
@code
eth0.ipaddr=192.168.1.240
eth0.netmask=255.255.255.0
eth0.gateway=192.168.23.2
eth0.serverip=192.168.1.7
eth0.ethaddr=00:04:f3:00:06:35
@endcode

A 'ping' to your TFTP server should bring a "...is alive" message now.

We are ready now to program @a barebox into the NAND flash:

@code
erase /dev/nand0.barebox.bb
tftp barebox.bin /dev/nand0.barebox.bb
@endcode

*/