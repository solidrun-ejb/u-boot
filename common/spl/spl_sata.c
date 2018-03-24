/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Derived work from spl_usb.c
 */

#include <common.h>
#include <dm.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <sata.h>
#include <scsi.h>
#include <errno.h>
#include <fat.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

static int sata_load_legacy(struct spl_image_info *spl_image, struct blk_desc *stor_dev,
			   ulong sector, struct image_header *header)
{
	u32 image_size_sectors;
	unsigned long count;
	int ret;

	ret = spl_parse_image_header(spl_image, header);
	if (ret)
		return ret;

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image->size + stor_dev->blksz - 1) /
			      stor_dev->blksz;

	/* Read the header too to avoid extra memcpy */
	count = blk_dread(stor_dev, sector, image_size_sectors,
			  (void *)(ulong)spl_image->load_addr);
	debug("read %x sectors to %lx\n", image_size_sectors,
	      spl_image->load_addr);
	if (count != image_size_sectors)
		return -EIO;

	return 0;
}

static ulong h_spl_load_read(struct spl_load_info *load, ulong sector,
			     ulong count, void *buf)
{
	return blk_dread(load->dev, sector, count, buf);
}

static __maybe_unused
int sata_load_image_raw_sector(struct spl_image_info *spl_image,
			      struct blk_desc *stor_dev, unsigned long sector)
{
	unsigned long count;
	struct image_header *header;
	int ret = 0;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
					 sizeof(struct image_header));

	/* read image header to find the image size & load address */
	count = blk_dread(stor_dev, sector, 1, header);
	debug("hdr read sector %lx, count=%lu\n", sector, count);
	if (count == 0) {
		ret = -EIO;
		goto end;
	}

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		struct spl_load_info load;

		debug("Found FIT\n");
		load.dev = stor_dev;
		load.priv = NULL;
		load.filename = NULL;
		load.bl_len = stor_dev->blksz;
		load.read = h_spl_load_read;
		ret = spl_load_simple_fit(spl_image, &load, sector, header);
	} else {
		ret = sata_load_legacy(spl_image, stor_dev, sector, header);
	}

end:
	if (ret) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("sata_load_image_raw_sector: sata block read error\n");
#endif
		return -1;
	}

	return 0;
}

static int spl_sata_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int err = 0;
	struct blk_desc *stor_dev;
	int if_type;	

#ifdef CONFIG_SPL_SATA_SUPPORT
#ifdef CONFIG_SPL_SATA_BOOT_DEVICE
	err = init_sata(CONFIG_SPL_SATA_BOOT_DEVICE);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: sata init failed: err - %d\n", err);
#endif
		return err;
	} else {
		if_type = IF_TYPE_SATA;
#else
		if_type = IF_TYPE_SCSI;
		scsi_init();
#endif
		/* try to recognize storage devices immediately */
		scsi_scan(false);
		stor_dev = blk_get_devnum_by_type(if_type, 0);
		if (!stor_dev)
			return -ENODEV;
#ifdef CONFIG_SPL_SATA_BOOT_DEVICE
	}
#endif
#endif

	if (err)
		goto out;

#ifdef CONFIG_SYS_SATA_RAW_MODE_U_BOOT_USE_SECTOR
	err = sata_load_image_raw_sector(spl_image, stor_dev,
			CONFIG_SYS_SATA_RAW_MODE_U_BOOT_SECTOR);
	if (!err)
		return err;
	else
		goto out;
#endif

#ifdef CONFIG_SYS_SATA_FAT_BOOT_PARTITION
#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
		err = spl_load_image_fat(spl_image, stor_dev,
					CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
	}
	if (err)
		puts("Error loading sata device\n");
#endif

out:
	return err;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);
