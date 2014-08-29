/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/i2c/smb349.h>
#include <linux/i2c/sx150x.h>
#include <linux/slimbus/slimbus.h>
#include <linux/mfd/wcd9xxx/core.h>
#include <linux/mfd/wcd9xxx/pdata.h>
#include <linux/mfd/pm8xxx/misc.h>
#include <linux/msm_ssbi.h>
#include <linux/spi/spi.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/qcom_crypto_device.h>
#include <linux/msm_ion.h>
#include <linux/memory.h>
#include <linux/memblock.h>
#include <linux/msm_thermal.h>
#include <linux/i2c/atmel_mxt_ts.h>
//shihuiqin++
#include <linux/atmel_ts.h>
#include <linux/synaptics_i2c_rmi.h> //zhangzhao added synaptics for 865a10 2012-8-23
//shihuiqin--
#include <linux/cyttsp-qc.h>
#include <linux/i2c/isa1200.h>
#include <linux/gpio_keys.h>
#include <linux/epm_adc.h>
#include <linux/i2c/sx150x.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <asm/mach/mmc.h>
#include <linux/platform_data/qcom_wcnss_device.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/ion.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/usb/android.h>
#include <mach/socinfo.h>
#include <mach/msm_spi.h>
#include "timer.h"
#include "devices.h"
#include <mach/gpiomux.h>
#include <mach/rpm.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <mach/msm_memtypes.h>
#include <linux/bootmem.h>
#include <asm/setup.h>
#include <mach/dma.h>
#include <mach/msm_dsps.h>
#include <mach/msm_bus_board.h>
#include <mach/cpuidle.h>
#include <mach/mdm2.h>
#include <linux/msm_tsens.h>
#include <mach/msm_xo.h>
#include <mach/msm_rtb.h>
#include <mach/msm_serial_hs.h>
#include <sound/cs8427.h>
#include <media/gpio-ir-recv.h>
#include <linux/fmem.h>
#include <mach/msm_pcie.h>
#include <mach/restart.h>
#include <mach/msm_iomap.h>
#include <media/radio_si470x.h>		//jiaobaocun
#include <mach/msm_serial_hs.h>

#include "msm_watchdog.h"
#include "board-8064.h"
#include "clock.h"
#include "spm.h"
#include <mach/mpm.h>
#include "rpm_resources.h"
#include "pm.h"
#include "pm-boot.h"
#include "devices-msm8x60.h"
#include "smd_private.h"
#include "sysmon.h"

#ifdef CONFIG_PN544_NFC
#include <linux/nfc/pn544.h>
#endif

//shihuiqin++
//ZTEBSP fanjiankang for sensor,20120628++
#include <linux/i2c/lsm330dlc.h>
#include <linux/akm8963.h>
//ZTEBSP fanjiankang for sensor,20120628++
#if defined(CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
//#define SPI_SW_CONTROL    /*SPI_HW_CONTROL*/		/*jiaobaocun deleted for gsbi5 as i2c mode*/
#endif

#define MSM_PMEM_ADSP_SIZE         0x7800000
#define MSM_PMEM_AUDIO_SIZE        0x4CF000
#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#else
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#endif

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
#define HOLE_SIZE		0x20000
#define MSM_ION_MFC_META_SIZE  0x40000 /* 256 Kbytes */
#define MSM_CONTIG_MEM_SIZE  0x65000
#ifdef CONFIG_MSM_IOMMU
#define MSM_ION_MM_SIZE		0x4800000
#define MSM_ION_SF_SIZE		0
#define MSM_ION_QSECOM_SIZE	0x780000 /* (7.5MB) */
#define MSM_ION_HEAP_NUM	8
#else
#define MSM_ION_MM_SIZE		MSM_PMEM_ADSP_SIZE
#define MSM_ION_SF_SIZE		MSM_PMEM_SIZE
#define MSM_ION_QSECOM_SIZE	0x600000 /* (6MB) */
#define MSM_ION_HEAP_NUM	8
#endif
#define MSM_ION_MM_FW_SIZE	(0x200000 - HOLE_SIZE) /* (2MB - 128KB) */
#define MSM_ION_MFC_SIZE	(SZ_8K + MSM_ION_MFC_META_SIZE)
#define MSM_ION_AUDIO_SIZE	MSM_PMEM_AUDIO_SIZE
#else
#define MSM_CONTIG_MEM_SIZE  0x110C000
#define MSM_ION_HEAP_NUM	1
#endif

#define APQ8064_FIXED_AREA_START (0xa0000000 - (MSM_ION_MM_FW_SIZE + \
							HOLE_SIZE))
#define MAX_FIXED_AREA_SIZE	0x10000000
#define MSM_MM_FW_SIZE		(0x200000 - HOLE_SIZE)
#define APQ8064_FW_START	APQ8064_FIXED_AREA_START
#define MSM_ION_ADSP_SIZE	SZ_8M

#define QFPROM_RAW_FEAT_CONFIG_ROW0_MSB     (MSM_QFPROM_BASE + 0x23c)
#define QFPROM_RAW_OEM_CONFIG_ROW0_LSB      (MSM_QFPROM_BASE + 0x220)

/* PCIE AXI address space */
#define PCIE_AXI_BAR_PHYS   0x08000000
#define PCIE_AXI_BAR_SIZE   SZ_128M

/* PCIe pmic gpios */
#define PCIE_WAKE_N_PMIC_GPIO 12
#define PCIE_PWR_EN_PMIC_GPIO 13
#define PCIE_RST_N_PMIC_MPP 1

/*[ECID:000000] ZTEBSP zhangzhao start  20120321, 8x25 bring up  */
//#ifdef CONFIG_TOUCHSCREEN_VIRTUAL_KEYS
struct kobject *android_touch_kobj;
static void touch_sysfs_init(void)
{
	android_touch_kobj = kobject_create_and_add("board_properties", NULL);
	if (android_touch_kobj == NULL) {
		printk(KERN_ERR "%s: subsystem_register failed\n", __func__);
	}
}
//#endif

/*ZTEBSP wangbing, for modelno driver, 20121015 +++*/
#define MODELNO_LEN 10
static char modelno[MODELNO_LEN] = "unknown";
static int __init modelno_kernel_setup(char *p)
{
	strlcpy(modelno, p, MODELNO_LEN);
	printk("%s modelno %s\n", __func__, modelno);
	return 0;
}
char *get_modelno(void)
{
	return (char*)modelno;
}
EXPORT_SYMBOL(get_modelno);
early_param("androidboot.modelno", modelno_kernel_setup);
/*ZTEBSP wangbing, for modelno driver, 20121015 ---*/

/*ZTEBSP wangbing, for modelno driver, 20121226 +++*/
#define MODE_LEN 10
static char bootmode[MODE_LEN] = "unknown";
static int __init bootmode_kernel_setup(char *p)
{
	strlcpy(bootmode, p, MODE_LEN);
	printk("%s bootmode %s\n", __func__, bootmode);
	return 0;
}
char *get_bootmode(void)
{
	return (char*)bootmode;
}
EXPORT_SYMBOL(get_bootmode);
early_param("androidboot.mode", bootmode_kernel_setup);
/*ZTEBSP wangbing, for modelno driver, 20121226 ---*/

#ifdef CONFIG_KERNEL_MSM_CONTIG_MEM_REGION
static unsigned msm_contig_mem_size = MSM_CONTIG_MEM_SIZE;
static int __init msm_contig_mem_size_setup(char *p)
{
	msm_contig_mem_size = memparse(p, NULL);
	return 0;
}
early_param("msm_contig_mem_size", msm_contig_mem_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
static unsigned pmem_size = MSM_PMEM_SIZE;
static int __init pmem_size_setup(char *p)
{
	pmem_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_size", pmem_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;

static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;

static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device apq8064_android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = {.platform_data = &android_pmem_pdata},
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};
static struct platform_device apq8064_android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device apq8064_android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};
#endif /* CONFIG_MSM_MULTIMEDIA_USE_ION */
#endif /* CONFIG_ANDROID_PMEM */

#ifdef CONFIG_BATTERY_BCL
static struct platform_device battery_bcl_device = {
	.name = "battery_current_limit",
	.id = -1,
	};
#endif

struct fmem_platform_data apq8064_fmem_pdata = {
};

static struct memtype_reserve apq8064_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void __init reserve_rtb_memory(void)
{
#if defined(CONFIG_MSM_RTB)
	apq8064_reserve_table[MEMTYPE_EBI1].size += apq8064_rtb_pdata.size;
	pr_info("mem_map: rtb reserved with size 0x%x in pool\n",
			apq8064_rtb_pdata.size);
#endif
}


static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	android_pmem_adsp_pdata.size = pmem_adsp_size;
	android_pmem_pdata.size = pmem_size;
	android_pmem_audio_pdata.size = MSM_PMEM_AUDIO_SIZE;
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/
}

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	apq8064_reserve_table[p->memory_type].size += p->size;
}
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_pdata);
	reserve_memory_for(&android_pmem_audio_pdata);
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
	apq8064_reserve_table[MEMTYPE_EBI1].size += msm_contig_mem_size;
	pr_info("mem_map: contig_mem reserved with size 0x%x in pool\n",
			msm_contig_mem_size);
#endif /*CONFIG_ANDROID_PMEM*/
}

static int apq8064_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

#define FMEM_ENABLED 0

#ifdef CONFIG_ION_MSM
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_cp_heap_pdata cp_mm_apq8064_ion_pdata = {
	.permission_type = IPT_TYPE_MM_CARVEOUT,
	.align = PAGE_SIZE,
	.reusable = FMEM_ENABLED,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_MIDDLE,
	.is_cma = 1,
	.no_nonsecure_alloc = 1,
};

static struct ion_cp_heap_pdata cp_mfc_apq8064_ion_pdata = {
	.permission_type = IPT_TYPE_MFC_SHAREDMEM,
	.align = PAGE_SIZE,
	.reusable = 0,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_HIGH,
	.no_nonsecure_alloc = 1,
};

static struct ion_co_heap_pdata co_apq8064_ion_pdata = {
	.adjacent_mem_id = INVALID_HEAP_ID,
	.align = PAGE_SIZE,
	.mem_is_fmem = 0,
};

static struct ion_co_heap_pdata fw_co_apq8064_ion_pdata = {
	.adjacent_mem_id = ION_CP_MM_HEAP_ID,
	.align = SZ_128K,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_LOW,
};
#endif

static u64 msm_dmamask = DMA_BIT_MASK(32);

static struct platform_device ion_mm_heap_device = {
	.name = "ion-mm-heap-device",
	.id = -1,
	.dev = {
		.dma_mask = &msm_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};

static struct platform_device ion_adsp_heap_device = {
	.name = "ion-adsp-heap-device",
	.id = -1,
	.dev = {
		.dma_mask = &msm_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};
/**
 * These heaps are listed in the order they will be allocated. Due to
 * video hardware restrictions and content protection the FW heap has to
 * be allocated adjacent (below) the MM heap and the MFC heap has to be
 * allocated after the MM heap to ensure MFC heap is not more than 256MB
 * away from the base address of the FW heap.
 * However, the order of FW heap and MM heap doesn't matter since these
 * two heaps are taken care of by separate code to ensure they are adjacent
 * to each other.
 * Don't swap the order unless you know what you are doing!
 */
struct ion_platform_heap apq8064_heaps[] = {
		{
			.id	= ION_SYSTEM_HEAP_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		{
			.id	= ION_CP_MM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MM_HEAP_NAME,
			.size	= MSM_ION_MM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mm_apq8064_ion_pdata,
			.priv	= &ion_mm_heap_device.dev
		},
		{
			.id	= ION_MM_FIRMWARE_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_MM_FIRMWARE_HEAP_NAME,
			.size	= MSM_ION_MM_FW_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &fw_co_apq8064_ion_pdata,
		},
		{
			.id	= ION_CP_MFC_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MFC_HEAP_NAME,
			.size	= MSM_ION_MFC_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mfc_apq8064_ion_pdata,
		},
#ifndef CONFIG_MSM_IOMMU
		{
			.id	= ION_SF_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_SF_HEAP_NAME,
			.size	= MSM_ION_SF_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
#endif
		{
			.id	= ION_IOMMU_HEAP_ID,
			.type	= ION_HEAP_TYPE_IOMMU,
			.name	= ION_IOMMU_HEAP_NAME,
		},
		{
			.id	= ION_QSECOM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_QSECOM_HEAP_NAME,
			.size	= MSM_ION_QSECOM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
		{
			.id	= ION_AUDIO_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_AUDIO_HEAP_NAME,
			.size	= MSM_ION_AUDIO_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
		{
			.id     = ION_ADSP_HEAP_ID,
			.type   = ION_HEAP_TYPE_DMA,
			.name   = ION_ADSP_HEAP_NAME,
			.size   = MSM_ION_ADSP_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
			.priv = &ion_adsp_heap_device.dev,
		},
#endif
};

static struct ion_platform_data apq8064_ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.heaps = apq8064_heaps,
};

static struct platform_device apq8064_ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &apq8064_ion_pdata },
};
#endif

static struct platform_device apq8064_fmem_device = {
	.name = "fmem",
	.id = 1,
	.dev = { .platform_data = &apq8064_fmem_pdata },
};

static void __init reserve_mem_for_ion(enum ion_memory_types mem_type,
				      unsigned long size)
{
	apq8064_reserve_table[mem_type].size += size;
}

static void __init apq8064_reserve_fixed_area(unsigned long fixed_area_size)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	int ret;

	if (fixed_area_size > MAX_FIXED_AREA_SIZE)
		panic("fixed area size is larger than %dM\n",
			MAX_FIXED_AREA_SIZE >> 20);

	reserve_info->fixed_area_size = fixed_area_size;
	reserve_info->fixed_area_start = APQ8064_FW_START;

	ret = memblock_remove(reserve_info->fixed_area_start,
		reserve_info->fixed_area_size);
	pr_info("mem_map: fixed_area reserved at 0x%lx with size 0x%lx\n",
			reserve_info->fixed_area_start,
			reserve_info->fixed_area_size);
	BUG_ON(ret);
#endif
}

/**
 * Reserve memory for ION and calculate amount of reusable memory for fmem.
 * We only reserve memory for heaps that are not reusable. However, we only
 * support one reusable heap at the moment so we ignore the reusable flag for
 * other than the first heap with reusable flag set. Also handle special case
 * for video heaps (MM,FW, and MFC). Video requires heaps MM and MFC to be
 * at a higher address than FW in addition to not more than 256MB away from the
 * base address of the firmware. This means that if MM is reusable the other
 * two heaps must be allocated in the same region as FW. This is handled by the
 * mem_is_fmem flag in the platform data. In addition the MM heap must be
 * adjacent to the FW heap for content protection purposes.
 */
static void __init reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	unsigned int i;
	unsigned int ret;
	unsigned int fixed_size = 0;
	unsigned int fixed_low_size, fixed_middle_size, fixed_high_size;
	unsigned long fixed_low_start, fixed_middle_start, fixed_high_start;
	unsigned long cma_alignment;
	unsigned int low_use_cma = 0;
	unsigned int middle_use_cma = 0;
	unsigned int high_use_cma = 0;


	fixed_low_size = 0;
	fixed_middle_size = 0;
	fixed_high_size = 0;

	cma_alignment = PAGE_SIZE << max(MAX_ORDER, pageblock_order);

	for (i = 0; i < apq8064_ion_pdata.nr; ++i) {
		struct ion_platform_heap *heap =
			&(apq8064_ion_pdata.heaps[i]);
		int use_cma = 0;


		if (heap->extra_data) {
			int fixed_position = NOT_FIXED;

			switch ((int)heap->type) {
			case ION_HEAP_TYPE_CP:
				if (((struct ion_cp_heap_pdata *)
					heap->extra_data)->is_cma) {
					heap->size = ALIGN(heap->size,
						cma_alignment);
					use_cma = 1;
				}
				fixed_position = ((struct ion_cp_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			case ION_HEAP_TYPE_DMA:
				use_cma = 1;
				/* Purposely fall through here */
			case ION_HEAP_TYPE_CARVEOUT:
				fixed_position = ((struct ion_co_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			default:
				break;
			}

			if (fixed_position != NOT_FIXED)
				fixed_size += heap->size;
			else if (!use_cma)
				reserve_mem_for_ion(MEMTYPE_EBI1, heap->size);

			if (fixed_position == FIXED_LOW) {
				fixed_low_size += heap->size;
				low_use_cma = use_cma;
			} else if (fixed_position == FIXED_MIDDLE) {
				fixed_middle_size += heap->size;
				middle_use_cma = use_cma;
			} else if (fixed_position == FIXED_HIGH) {
				fixed_high_size += heap->size;
				high_use_cma = use_cma;
			} else if (use_cma) {
				/*
				 * Heaps that use CMA but are not part of the
				 * fixed set. Create wherever.
				 */
				dma_declare_contiguous(
					heap->priv,
					heap->size,
					0,
					0xb0000000);

			}
		}
	}

	if (!fixed_size)
		return;

	/*
	 * Given the setup for the fixed area, we can't round up all sizes.
	 * Some sizes must be set up exactly and aligned correctly. Incorrect
	 * alignments are considered a configuration issue
	 */

	fixed_low_start = APQ8064_FIXED_AREA_START;
	if (low_use_cma) {
		BUG_ON(!IS_ALIGNED(fixed_low_size + HOLE_SIZE, cma_alignment));
		BUG_ON(!IS_ALIGNED(fixed_low_start, cma_alignment));
	} else {
		BUG_ON(!IS_ALIGNED(fixed_low_size + HOLE_SIZE, SECTION_SIZE));
		ret = memblock_remove(fixed_low_start,
				      fixed_low_size + HOLE_SIZE);
		pr_info("mem_map: fixed_low_area reserved at 0x%lx with size \
				0x%x\n", fixed_low_start,
				fixed_low_size + HOLE_SIZE);
		BUG_ON(ret);
	}

	fixed_middle_start = fixed_low_start + fixed_low_size + HOLE_SIZE;
	if (middle_use_cma) {
		BUG_ON(!IS_ALIGNED(fixed_middle_start, cma_alignment));
		BUG_ON(!IS_ALIGNED(fixed_middle_size, cma_alignment));
	} else {
		BUG_ON(!IS_ALIGNED(fixed_middle_size, SECTION_SIZE));
		ret = memblock_remove(fixed_middle_start, fixed_middle_size);
		pr_info("mem_map: fixed_middle_area reserved at 0x%lx with \
				size 0x%x\n", fixed_middle_start,
				fixed_middle_size);
		BUG_ON(ret);
	}

	fixed_high_start = fixed_middle_start + fixed_middle_size;
	if (high_use_cma) {
		fixed_high_size = ALIGN(fixed_high_size, cma_alignment);
		BUG_ON(!IS_ALIGNED(fixed_high_start, cma_alignment));
	} else {
		/* This is the end of the fixed area so it's okay to round up */
		fixed_high_size = ALIGN(fixed_high_size, SECTION_SIZE);
		ret = memblock_remove(fixed_high_start, fixed_high_size);
		pr_info("mem_map: fixed_high_area reserved at 0x%lx with size \
				0x%x\n", fixed_high_start,
				fixed_high_size);
		BUG_ON(ret);
	}

	for (i = 0; i < apq8064_ion_pdata.nr; ++i) {
		struct ion_platform_heap *heap = &(apq8064_ion_pdata.heaps[i]);

		if (heap->extra_data) {
			int fixed_position = NOT_FIXED;
			struct ion_cp_heap_pdata *pdata = NULL;

			switch ((int) heap->type) {
			case ION_HEAP_TYPE_CP:
				pdata =
				(struct ion_cp_heap_pdata *)heap->extra_data;
				fixed_position = pdata->fixed_position;
				break;
			case ION_HEAP_TYPE_CARVEOUT:
			case ION_HEAP_TYPE_DMA:
				fixed_position = ((struct ion_co_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			default:
				break;
			}

			switch (fixed_position) {
			case FIXED_LOW:
				heap->base = fixed_low_start;
				break;
			case FIXED_MIDDLE:
				heap->base = fixed_middle_start;
				if (middle_use_cma) {
					ret = dma_declare_contiguous(
						heap->priv,
						heap->size,
						fixed_middle_start,
						0xa0000000);
					WARN_ON(ret);
				}
				pdata->secure_base = fixed_middle_start
								- HOLE_SIZE;
				pdata->secure_size = HOLE_SIZE + heap->size;
				break;
			case FIXED_HIGH:
				heap->base = fixed_high_start;
				break;
			default:
				break;
			}
		}
	}
#endif
}

static void __init reserve_mdp_memory(void)
{
	apq8064_mdp_writeback(apq8064_reserve_table);
}

static void __init reserve_cache_dump_memory(void)
{
#ifdef CONFIG_MSM_CACHE_DUMP
	unsigned int total;

	total = apq8064_cache_dump_pdata.l1_size +
		apq8064_cache_dump_pdata.l2_size;
	apq8064_reserve_table[MEMTYPE_EBI1].size += total;
	pr_info("mem_map: cache_dump reserved with size 0x%x in pool\n",
			total);
#endif
}

static void __init reserve_mpdcvs_memory(void)
{
	apq8064_reserve_table[MEMTYPE_EBI1].size += SZ_32K;
}

static void __init apq8064_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
	reserve_ion_memory();
	reserve_mdp_memory();
	reserve_rtb_memory();
	reserve_cache_dump_memory();
	reserve_mpdcvs_memory();
}

static struct reserve_info apq8064_reserve_info __initdata = {
	.memtype_reserve_table = apq8064_reserve_table,
	.calculate_reserve_sizes = apq8064_calculate_reserve_sizes,
	.reserve_fixed_area = apq8064_reserve_fixed_area,
	.paddr_to_memtype = apq8064_paddr_to_memtype,
};

static char prim_panel_name[PANEL_NAME_MAX_LEN];
static char ext_panel_name[PANEL_NAME_MAX_LEN];

static int ext_resolution;

static int __init prim_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(prim_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("prim_display", prim_display_setup);

static int __init ext_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(ext_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("ext_display", ext_display_setup);

static int __init hdmi_resulution_setup(char *param)
{
	int ret;
	ret = kstrtoint(param, 10, &ext_resolution);
	return ret;
}
early_param("ext_resolution", hdmi_resulution_setup);

static void __init apq8064_reserve(void)
{
	apq8064_set_display_params(prim_panel_name, ext_panel_name,
		ext_resolution);
	msm_reserve();
}

static void __init apq8064_early_reserve(void)
{
	reserve_info = &apq8064_reserve_info;
}
#ifdef CONFIG_USB_EHCI_MSM_HSIC
/* Bandwidth requests (zero) if no vote placed */
static struct msm_bus_vectors hsic_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_SPS,
		.ab = 0,
		.ib = 0,
	},
};

/* Bus bandwidth requests in Bytes/sec */
static struct msm_bus_vectors hsic_max_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_SPS,
		.ab = 0,
		.ib = 256000000, /*vote for 32Mhz dfab clk rate*/
	},
};

static struct msm_bus_paths hsic_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(hsic_init_vectors),
		hsic_init_vectors,
	},
	{
		ARRAY_SIZE(hsic_max_vectors),
		hsic_max_vectors,
	},
};

static struct msm_bus_scale_pdata hsic_bus_scale_pdata = {
	hsic_bus_scale_usecases,
	ARRAY_SIZE(hsic_bus_scale_usecases),
	.name = "hsic",
};

static struct msm_hsic_host_platform_data msm_hsic_pdata = {
	.strobe			= 88,
	.data			= 89,
	.bus_scale_table	= &hsic_bus_scale_pdata,
};
#else
static struct msm_hsic_host_platform_data msm_hsic_pdata;
#endif

#define PID_MAGIC_ID		0x71432909
#define SERIAL_NUM_MAGIC_ID	0x61945374
#define SERIAL_NUMBER_LENGTH	127
#define DLOAD_USB_BASE_ADD	0x2A03F0C8

struct magic_num_struct {
	uint32_t pid;
	uint32_t serial_num;
};

struct dload_struct {
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
	uint16_t	reserved4;
	uint16_t	pid;
	char		serial_number[SERIAL_NUMBER_LENGTH];
	uint16_t	reserved5;
	struct magic_num_struct magic_struct;
};

static int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum)
{
	struct dload_struct __iomem *dload = 0;

	dload = ioremap(DLOAD_USB_BASE_ADD, sizeof(*dload));
	if (!dload) {
		pr_err("%s: cannot remap I/O memory region: %08x\n",
					__func__, DLOAD_USB_BASE_ADD);
		return -ENXIO;
	}

	pr_debug("%s: dload:%p pid:%x serial_num:%s\n",
				__func__, dload, pid, snum);
	/* update pid */
	dload->magic_struct.pid = PID_MAGIC_ID;
	dload->pid = pid;

	/* update serial number */
	dload->magic_struct.serial_num = 0;
	if (!snum) {
		memset(dload->serial_number, 0, SERIAL_NUMBER_LENGTH);
		goto out;
	}

	dload->magic_struct.serial_num = SERIAL_NUM_MAGIC_ID;
	strlcpy(dload->serial_number, snum, SERIAL_NUMBER_LENGTH);
out:
	iounmap(dload);
	return 0;
}

static struct android_usb_platform_data android_usb_pdata = {
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

/* Bandwidth requests (zero) if no vote placed */
static struct msm_bus_vectors usb_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

/* Bus bandwidth requests in Bytes/sec */
static struct msm_bus_vectors usb_max_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 60000000,		/* At least 480Mbps on bus. */
		.ib = 960000000,	/* MAX bursts rate */
	},
};

static struct msm_bus_paths usb_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(usb_init_vectors),
		usb_init_vectors,
	},
	{
		ARRAY_SIZE(usb_max_vectors),
		usb_max_vectors,
	},
};

static struct msm_bus_scale_pdata usb_bus_scale_pdata = {
	usb_bus_scale_usecases,
	ARRAY_SIZE(usb_bus_scale_usecases),
	.name = "usb",
};

static int phy_init_seq[] = {
	0x68, 0x81, /* update DC voltage level */
	0x24, 0x82, /* set pre-emphasis and rise/fall time */
	-1
};

#define PMIC_GPIO_DP		27    /* PMIC GPIO for D+ change */
#define PMIC_GPIO_DP_IRQ	PM8921_GPIO_IRQ(PM8921_IRQ_BASE, PMIC_GPIO_DP)
#define MSM_MPM_PIN_USB1_OTGSESSVLD	40

//ztebsp zhangjing add for otg,20120704,++
#if defined(CONFIG_PROJECT_P864A10)||defined(CONFIG_PROJECT_P864A20)
#define GPIO_VREG_OTG_EN PM8921_GPIO_PM_TO_SYS(36)
#elif defined(CONFIG_PROJECT_P864H01) ||defined(CONFIG_PROJECT_P864G02)
#define GPIO_VREG_OTG_EN PM8921_GPIO_PM_TO_SYS(42)

#endif

static int msm_hsusb_vbus_power(bool on)
{
	int rc = 0;
	printk(KERN_INFO "zhangjing :file=%s,func=%s,line=%d,power=%d\n" ,__FILE__,__func__,__LINE__,on);
	rc = gpio_request(GPIO_VREG_OTG_EN, "i2c_host_vbus_en");
	if (rc < 0) {
		pr_err("failed to request GPIO\n");
		return 0;
	}
	gpio_direction_output(GPIO_VREG_OTG_EN, !!on);
	gpio_free(GPIO_VREG_OTG_EN);
	#if 0//a10ÎÞŽËgpio
	rc = gpio_request(GPIO_HOST_EN, "vreg_otg_en");
	if (rc < 0) {
		pr_err("failed to request GPIO\n");
		return 0;
	}
	
	gpio_direction_output(GPIO_HOST_EN, !!on);
	
	gpio_free(GPIO_HOST_EN);
	#endif 
	
	return 1;

}
//ztebsp zhangjing add for otg,20120704,--
#define MSM_MPM_PIN_USB1_OTGSESSVLD	40
static struct msm_otg_platform_data msm_otg_pdata = {
	.mode			= USB_OTG,
	.otg_control		= OTG_PMIC_CONTROL,
	.phy_type		= SNPS_28NM_INTEGRATED_PHY,
	.pmic_id_irq		= PM8921_USB_ID_IN_IRQ(PM8921_IRQ_BASE),
	.power_budget		= 750,
	.bus_scale_table	= &usb_bus_scale_pdata,
	.phy_init_seq		= phy_init_seq,
	.mpm_otgsessvld_int	= MSM_MPM_PIN_USB1_OTGSESSVLD,
	//ztebsp zhangjing add for otg,20120704,++
       .vbus_power		 = msm_hsusb_vbus_power,
	//ztebsp zhangjing add for otg,20120704,--
};

static struct msm_usb_host_platform_data msm_ehci_host_pdata3 = {
	.power_budget = 500,
};

#ifdef CONFIG_USB_EHCI_MSM_HOST4
static struct msm_usb_host_platform_data msm_ehci_host_pdata4;
#endif

static void __init apq8064_ehci_host_init(void)
{
	if (machine_is_apq8064_liquid() || machine_is_mpq8064_cdp() ||
		machine_is_mpq8064_hrd() || machine_is_mpq8064_dtv()) {
		if (machine_is_apq8064_liquid())
			msm_ehci_host_pdata3.dock_connect_irq =
					PM8921_MPP_IRQ(PM8921_IRQ_BASE, 9);

		apq8064_device_ehci_host3.dev.platform_data =
				&msm_ehci_host_pdata3;
		platform_device_register(&apq8064_device_ehci_host3);

#ifdef CONFIG_USB_EHCI_MSM_HOST4
		apq8064_device_ehci_host4.dev.platform_data =
				&msm_ehci_host_pdata4;
		platform_device_register(&apq8064_device_ehci_host4);
#endif
	}
}

static struct smb349_platform_data smb349_data __initdata = {
	.en_n_gpio		= PM8921_GPIO_PM_TO_SYS(37),
	.chg_susp_gpio		= PM8921_GPIO_PM_TO_SYS(30),
	.chg_current_ma		= 2200,
};

static struct i2c_board_info smb349_charger_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(SMB349_NAME, 0x1B),
		.platform_data	= &smb349_data,
	},
};

struct sx150x_platform_data apq8064_sx150x_data[] = {
	[SX150X_EPM] = {
		.gpio_base	= GPIO_EPM_EXPANDER_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= -1,
	},
};

static struct epm_chan_properties ads_adc_channel_data[] = {
	{10, 100}, {500, 50}, {1, 1}, {1, 1},
	{20, 50}, {10, 100}, {1, 1}, {1, 1},
	{10, 100}, {10, 100}, {100, 100}, {200, 100},
	{100, 50}, {2000, 50}, {1000, 50}, {200, 50},
	{200, 100}, {1, 1}, {20, 50}, {500, 50},
	{50, 50}, {200, 100}, {500, 100}, {20, 50},
	{200, 50}, {2000, 100}, {1000, 50}, {100, 50},
	{200, 100}, {500, 50}, {1000, 100}, {200, 50},
	{1000, 50}, {50, 50}, {100, 50}, {100, 50},
	{1, 1}, {1, 1}, {20, 100}, {20, 50},
	{500, 100}, {1000, 100}, {100, 50}, {1000, 50},
	{100, 50}, {1000, 100}, {100, 50}, {100, 50},
};

static struct epm_adc_platform_data epm_adc_pdata = {
	.channel		= ads_adc_channel_data,
	.bus_id	= 0x0,
	.epm_i2c_board_info = {
		.type	= "sx1509q",
		.addr = 0x3e,
		.platform_data = &apq8064_sx150x_data[SX150X_EPM],
	},
	.gpio_expander_base_addr = GPIO_EPM_EXPANDER_BASE,
};

static struct platform_device epm_adc_device = {
	.name   = "epm_adc",
	.id = -1,
	.dev = {
		.platform_data = &epm_adc_pdata,
	},
};

static void __init apq8064_epm_adc_init(void)
{
	epm_adc_pdata.num_channels = 32;
	epm_adc_pdata.num_adc = 2;
	epm_adc_pdata.chan_per_adc = 16;
	epm_adc_pdata.chan_per_mux = 8;
};

/* Micbias setting is based on 8660 CDP/MTP/FLUID requirement
 * 4 micbiases are used to power various analog and digital
 * microphones operating at 1800 mV. Technically, all micbiases
 * can source from single cfilter since all microphones operate
 * at the same voltage level. The arrangement below is to make
 * sure all cfilters are exercised. LDO_H regulator ouput level
 * does not need to be as high as 2.85V. It is choosen for
 * microphone sensitivity purpose.
 */
static struct wcd9xxx_pdata apq8064_tabla_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x10, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(42),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 2700,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT1_SEL,//ZTEBSP lichuangchuang modify TABLA_CFILT3_SEL -> TABLA_CFILT1_SEL for headset detect,20121227
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "VDDD_CDC_D",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_A_1P2V",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct slim_device apq8064_slim_tabla = {
	.name = "tabla-slim",
	.e_addr = {0, 1, 0x10, 0, 0x17, 2},
	.dev = {
		.platform_data = &apq8064_tabla_platform_data,
	},
};

static struct wcd9xxx_pdata apq8064_tabla20_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x60, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(42),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 2700,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT1_SEL,//ZTEBSP lichuangchuang modify TABLA_CFILT3_SEL -> TABLA_CFILT1_SEL for headset detect,20121227
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "VDDD_CDC_D",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_A_1P2V",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct slim_device apq8064_slim_tabla20 = {
	.name = "tabla2x-slim",
	.e_addr = {0, 1, 0x60, 0, 0x17, 2},
	.dev = {
		.platform_data = &apq8064_tabla20_platform_data,
	},
};

/* enable the level shifter for cs8427 to make sure the I2C
 * clock is running at 100KHz and voltage levels are at 3.3
 * and 5 volts
 */
static int enable_100KHz_ls(int enable)
{
	int ret = 0;
	if (enable) {
		ret = gpio_request(SX150X_GPIO(1, 10),
					"cs8427_100KHZ_ENABLE");
		if (ret) {
			pr_err("%s: Failed to request gpio %d\n", __func__,
				SX150X_GPIO(1, 10));
			return ret;
		}
		gpio_direction_output(SX150X_GPIO(1, 10), 1);
	} else {
		gpio_direction_output(SX150X_GPIO(1, 10), 0);
		gpio_free(SX150X_GPIO(1, 10));
	}
	return ret;
}

static struct cs8427_platform_data cs8427_i2c_platform_data = {
	.irq = SX150X_GPIO(1, 4),
	.reset_gpio = SX150X_GPIO(1, 6),
	.enable = enable_100KHz_ls,
};

static struct i2c_board_info cs8427_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("cs8427", CS8427_ADDR4),
		.platform_data = &cs8427_i2c_platform_data,
	},
};

#define HAP_SHIFT_LVL_OE_GPIO		PM8921_MPP_PM_TO_SYS(8)
//doumingming 20121023
#define ISA1200_HAP_EN_GPIO		PM8921_GPIO_PM_TO_SYS(19)
#define ISA1200_HAP_LEN_GPIO		PM8921_GPIO_PM_TO_SYS(20)
#define ISA1200_HAP_CLK			PM8921_GPIO_PM_TO_SYS(44)
#define ISA1200_HAP_CLK_PM8921		PM8921_GPIO_PM_TO_SYS(44)
#define ISA1200_HAP_CLK_PM8917		PM8921_GPIO_PM_TO_SYS(38)

static int isa1200_clk_enable(bool on)
{
	int rc = 0;

    gpio_set_value_cansleep(ISA1200_HAP_CLK, on);

	if (on) {
/*ZTEBSP XO_DIV_1*/
		rc = pm8xxx_aux_clk_control(CLK_MP3_2, XO_DIV_4, true);
		if (rc) {
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
			goto err_gpio_dis;
		}
	} else {
		rc = pm8xxx_aux_clk_control(CLK_MP3_2, XO_DIV_NONE, true);
		if (rc)
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
	}

	return rc;

err_gpio_dis:
	gpio_set_value_cansleep(ISA1200_HAP_CLK, !on);
	return rc;
}

static int isa1200_dev_setup(bool enable)
{
	int rc = 0;

	if (!enable)
		goto free_gpio;

	rc = gpio_request(ISA1200_HAP_CLK, "haptics_clk");
	if (rc) {
		pr_err("%s: unable to request gpio %d config(%d)\n",
			__func__, ISA1200_HAP_CLK, rc);
		return rc;
	}

	rc = gpio_direction_output(ISA1200_HAP_CLK, 0);
	if (rc) {
		pr_err("%s: unable to set direction\n", __func__);
		goto free_gpio;
	}

	return 0;

free_gpio:
	gpio_free(ISA1200_HAP_CLK);
	return rc;
}

/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 start*/
static struct isa1200_regulator isa1200_reg_data[] = {
	{
		.name = "vddp_isa1200",
		.min_uV = ISA_I2C_VTG_MIN_UV,
		.max_uV = ISA_I2C_VTG_MAX_UV,
		.load_uA = ISA_I2C_CURR_UA,
	},
};

static struct isa1200_platform_data isa1200_1_pdata = {
	.name = "vibrator",
	.dev_setup = isa1200_dev_setup,
	.clk_enable = isa1200_clk_enable,
	.need_pwm_clk = true,
	.hap_en_gpio = ISA1200_HAP_EN_GPIO,
	.hap_len_gpio = ISA1200_HAP_LEN_GPIO,
	.max_timeout = 15000,
	.mode_ctrl = PWM_GEN_MODE,
	.pwm_fd = {
		.pwm_div = 256,
	},
	.is_erm = false,
	.smart_en = true,
	.ext_clk_en = true,
	.chip_en = 1,
	.regulator_info = isa1200_reg_data,
	.num_regulators = ARRAY_SIZE(isa1200_reg_data),
};

static struct i2c_board_info isa1200_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("isa1200_1", 0x48),
//		#ifdef CONFIG_HAPTIC_ISA1200
		.platform_data = &isa1200_1_pdata,
//		#endif
	},
};

/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 end*/
/* configuration data for mxt1386e using V2.1 firmware */
static const u8 mxt1386e_config_data_v2_1[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	14, 2, 0, 24, 5, 12, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T7 Object */
	100, 10, 50,
	/* T8 Object */
	25, 0, 20, 20, 0, 0, 0, 0, 0, 0,
	/* T9 Object */
	139, 0, 0, 26, 42, 0, 32, 80, 2, 5,
	0, 5, 5, 0, 10, 30, 10, 10, 255, 2,
	85, 5, 0, 5, 9, 5, 12, 35, 70, 40,
	20, 5, 0, 0, 0,
	/* T18 Object */
	0, 0,
	/* T24 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* T25 Object */
	1, 0, 60, 115, 156, 99,
	/* T27 Object */
	0, 0, 0, 0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	/* T43 Object */
	0, 0, 0, 0, 0, 0, 0, 64, 0, 8,
	16,
	/* T46 Object */
	68, 0, 16, 16, 0, 0, 0, 0, 0,
	/* T47 Object */
	0, 0, 0, 0, 0, 0, 3, 64, 66, 0,
	/* T48 Object */
	1, 64, 64, 0, 0, 0, 0, 0, 0, 0,
	32, 40, 0, 10, 10, 0, 0, 100, 10, 90,
	0, 0, 0, 0, 0, 0, 0, 10, 1, 10,
	52, 10, 12, 0, 33, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T56 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
};

#define MXT_TS_GPIO_IRQ			6
#define MXT_TS_PWR_EN_GPIO		PM8921_GPIO_PM_TO_SYS(23)
#define MXT_TS_RESET_GPIO		33

static struct mxt_config_info mxt_config_array[] = {
	{
		.config		= mxt1386e_config_data_v2_1,
		.config_length	= ARRAY_SIZE(mxt1386e_config_data_v2_1),
		.family_id	= 0xA0,
		.variant_id	= 0x7,
		.version	= 0x21,
		.build		= 0xAA,
		.bootldr_id	= MXT_BOOTLOADER_ID_1386E,
		.fw_name	= "atmel_8064_liquid_v2_2_AA.hex",
	},
	{
		/* The config data for V2.2.AA is the same as for V2.1.AA */
		.config		= mxt1386e_config_data_v2_1,
		.config_length	= ARRAY_SIZE(mxt1386e_config_data_v2_1),
		.family_id	= 0xA0,
		.variant_id	= 0x7,
		.version	= 0x22,
		.build		= 0xAA,
		.bootldr_id	= MXT_BOOTLOADER_ID_1386E,
	},
};

static struct mxt_platform_data mxt_platform_data = {
	.config_array		= mxt_config_array,
	.config_array_size	= ARRAY_SIZE(mxt_config_array),
	.panel_minx		= 0,
	.panel_maxx		= 1365,
	.panel_miny		= 0,
	.panel_maxy		= 767,
	.disp_minx		= 0,
	.disp_maxx		= 1365,
	.disp_miny		= 0,
	.disp_maxy		= 767,
	.irqflags		= IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
	.i2c_pull_up		= true,
	.reset_gpio		= MXT_TS_RESET_GPIO,
	.irq_gpio		= MXT_TS_GPIO_IRQ,
};

static struct i2c_board_info mxt_device_info[] __initdata = {
	{
/*ZTEBSP atmel_mxt_ts*/
		I2C_BOARD_INFO("atmel_mxt_ts_x", 0x5b),
		.platform_data = &mxt_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_TS_GPIO_IRQ),
	},
};
#define CYTTSP_TS_GPIO_IRQ		6
#define CYTTSP_TS_GPIO_SLEEP		33
#define CYTTSP_TS_GPIO_SLEEP_ALT	12

static ssize_t tma340_vkeys_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, 200,
	__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":73:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":230:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":389:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":544:1120:97:97"
	"\n");
}

static struct kobj_attribute tma340_vkeys_attr = {
	.attr = {
		.mode = S_IRUGO,
	},
	.show = &tma340_vkeys_show,
};

static struct attribute *tma340_properties_attrs[] = {
	&tma340_vkeys_attr.attr,
	NULL
};

static struct attribute_group tma340_properties_attr_group = {
	.attrs = tma340_properties_attrs,
};

static int cyttsp_platform_init(struct i2c_client *client)
{
	int rc = 0;
	static struct kobject *tma340_properties_kobj;

	tma340_vkeys_attr.attr.name = "virtualkeys.cyttsp-i2c";
	tma340_properties_kobj = kobject_create_and_add("board_properties",
								NULL);
	if (tma340_properties_kobj)
		rc = sysfs_create_group(tma340_properties_kobj,
					&tma340_properties_attr_group);
	if (!tma340_properties_kobj || rc)
		pr_err("%s: failed to create board_properties\n",
				__func__);

	return 0;
}

static struct cyttsp_regulator cyttsp_regulator_data[] = {
	{
		.name = "vdd",
		.min_uV = CY_TMA300_VTG_MIN_UV,
		.max_uV = CY_TMA300_VTG_MAX_UV,
		.hpm_load_uA = CY_TMA300_CURR_24HZ_UA,
		.lpm_load_uA = CY_TMA300_CURR_24HZ_UA,
	},
	{
		.name = "vcc_i2c",
		.min_uV = CY_I2C_VTG_MIN_UV,
		.max_uV = CY_I2C_VTG_MAX_UV,
		.hpm_load_uA = CY_I2C_CURR_UA,
		.lpm_load_uA = CY_I2C_CURR_UA,
	},
};

static struct cyttsp_platform_data cyttsp_pdata = {
	.panel_maxx = 634,
	.panel_maxy = 1166,
	.disp_minx = 18,
	.disp_maxx = 617,
	.disp_miny = 18,
	.disp_maxy = 1041,
	.flags = 0x01,
	.gen = CY_GEN3,
	.use_st = CY_USE_ST,
	.use_mt = CY_USE_MT,
	.use_hndshk = CY_SEND_HNDSHK,
	.use_trk_id = CY_USE_TRACKING_ID,
	.use_sleep = CY_USE_DEEP_SLEEP_SEL,
	.use_gestures = CY_USE_GESTURES,
	.fw_fname = "cyttsp_8064_mtp.hex",
	/* change act_intrvl to customize the Active power state
	 * scanning/processing refresh interval for Operating mode
	 */
	.act_intrvl = CY_ACT_INTRVL_DFLT,
	/* change tch_tmout to customize the touch timeout for the
	 * Active power state for Operating mode
	 */
	.tch_tmout = CY_TCH_TMOUT_DFLT,
	/* change lp_intrvl to customize the Low Power power state
	 * scanning/processing refresh interval for Operating mode
	 */
	.lp_intrvl = CY_LP_INTRVL_DFLT,
	.sleep_gpio = CYTTSP_TS_GPIO_SLEEP,
	.resout_gpio = -1,
	.irq_gpio = CYTTSP_TS_GPIO_IRQ,
	.regulator_info = cyttsp_regulator_data,
	.num_regulators = ARRAY_SIZE(cyttsp_regulator_data),
	.init = cyttsp_platform_init,
	.correct_fw_ver = 17,
};

static struct i2c_board_info cyttsp_info[] __initdata = {
	{
		I2C_BOARD_INFO(CY_I2C_NAME, 0x24),
		.platform_data = &cyttsp_pdata,
		.irq = MSM_GPIO_TO_INT(CYTTSP_TS_GPIO_IRQ),
	},
};

/*[ECID:000000] ZTEBSP shihuiqin, for touchscreen_mxt224, 20120216 start*/
#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT   /*CONFIG_TOUCHSCREEN_ATMEL_MAXTOUCH*/
#define ATMEL_TS_I2C_NAME "maXTouch"

static struct regulator_bulk_data regs_atmel[] = {
//wangbing, for mxt224 debug, s4 should be always on debfined in msm_rpm_regulator_init_data
//	{ .supply = "8921_s4",  .min_uV = 1800000, .max_uV = 1800000 },
	{ .supply = "8921_lvs4", .min_uV = 1800000, .max_uV = 1800000 },
	
};

static struct regulator_bulk_data regs_atmel1[] = {
//wangbing, for mxt224 debug, s4 should be always on debfined in msm_rpm_regulator_init_data
//	{ .supply = "8921_s4",  .min_uV = 1800000, .max_uV = 1800000 },
	//{ .supply = "8921_l9", .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "8921_l9", .min_uV = 3000000, .max_uV = 3000000 },
	
};


#define ATMEL_TS_GPIO_IRQ 32
#define ATMEL_TS_RESET 33

//wangbing, temp for mxt224 debug
#if 0
static int atmel_ts_power_on(int on)
{
	int rc = on ?
		regulator_bulk_enable(ARRAY_SIZE(regs_atmel), regs_atmel) :
		regulator_bulk_disable(ARRAY_SIZE(regs_atmel), regs_atmel);

	if (rc)
		pr_err("%s: could not %sable regulators: %d\n",
				__func__, on ? "en" : "dis", rc);
	else
		msleep(50);

	if (on)
		gpio_set_value_cansleep(ATMEL_TS_RESET, 1); /* disp disable (resx=0) */
	else
		gpio_set_value_cansleep(ATMEL_TS_RESET, 0); /* disp disable (resx=0) */

	return rc;
}
#endif
static int atmel_ts_platform_init(struct i2c_client *client)
{
	int rc;
	struct device *dev = &client->dev;

	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_atmel), regs_atmel);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}


	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_atmel1), regs_atmel1);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}
//wangbing, temp for mxt224 debug, lvs not support the voltage setting.
#if 1
	rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_atmel1), regs_atmel1);
	if (rc) {
		dev_err(dev, "%s: could not set voltages: %d\n",
				__func__, rc);
		goto reg_free;
	}
#endif	


//wangbing, for mxt224 debug
	rc = gpio_request(ATMEL_TS_RESET, "atmel_ts_reset");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, ATMEL_TS_RESET);
	}
	rc = gpio_direction_output(ATMEL_TS_RESET, 0);
	if (rc) {
		pr_err("%s: unable to set_direction for gpio %d\n",
				__func__, ATMEL_TS_RESET);
	}

	/* configure touchscreen interrupt gpio */
	rc = gpio_request(ATMEL_TS_GPIO_IRQ, "atmel_maxtouch_gpio");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto ts_gpio_tlmm_unconfig;
	}

//wangbing, for mxt224 debug, need internal pullup resistor.
	rc = gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto reg_free;
	}

//wangbing, for mxt224 debug
	regulator_bulk_enable(ARRAY_SIZE(regs_atmel), regs_atmel);

	regulator_bulk_enable(ARRAY_SIZE(regs_atmel1), regs_atmel1);

	msleep(50);
	gpio_set_value_cansleep(ATMEL_TS_RESET, 1); /* disp disable (resx=0) */

	return 0;

ts_gpio_tlmm_unconfig:
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
reg_free:
	regulator_bulk_free(ARRAY_SIZE(regs_atmel), regs_atmel);
out:
	return rc;
}

static int atmel_ts_platform_exit(struct i2c_client *client)
{
	gpio_free(ATMEL_TS_GPIO_IRQ);
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	gpio_free(ATMEL_TS_RESET);//zhangzhao added synaptics for 865a10 2012-8-23
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_RESET, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	
	regulator_bulk_disable(ARRAY_SIZE(regs_atmel), regs_atmel);
	regulator_bulk_free(ARRAY_SIZE(regs_atmel), regs_atmel);
	return 0;
}

static int atmel_ts_read_chg(void)
{
	return gpio_get_value(ATMEL_TS_GPIO_IRQ);
}

static u8 atmel_ts_valid_interrupt(void)
{
	return !atmel_ts_read_chg();
}

#define ATMEL_X_OFFSET 13
#define ATMEL_Y_OFFSET 0

static struct mxt_platform_data_zte atmel_ts_pdata = {
	.numtouch = 10,
	.init_platform_hw = atmel_ts_platform_init,
	.exit_platform_hw = atmel_ts_platform_exit,
//wangbing, temp for mxt224 debug
//	.power_on = atmel_ts_power_on,
	.max_x = 480,
	.max_y = 800,
	.valid_interrupt = atmel_ts_valid_interrupt,
	.read_chg = atmel_ts_read_chg,
        .config_T7 = {255, 255, 50},
        .config_T8 = {10,0,20, 20, 0, 0, 30, 50,20,25},
		/** ZTE_MODIFY yuehongliang modified for 7" touch screen accelerated deletion, yuehongliang0004 */
        //.config_T9 = {131, 0, 0, 18, 11, 1, 0, 30, 2, 6, 10, 2, 2, 48, 5, 10, 30, 3, 0, 5, 32, 3, 0, 0, 0, 0, 64, 0, 0, 0, 18},/** ZTE_MODIFY yuehongliang modified for the boundary of touch screen, yuehongliang0003 */
        .config_T9 = {131, 0, 0, 19, 11, 0, 0, 40, 3, 1, 0, 0, 0, 0, 10, 5, 5, 10, 92, 3,
                    /*860*/ 224, 1,/*480*/ 0, 0, 0, 0, 0, 0, 0, 0, 10,6},/** ZTE_MODIFY yuehongliang modified for improve the threshold when startup device, yuehongliang0005 */
		/** ZTE_MODIFY end yuehongliang0004 */
        //.config_T15 = {3, 0, 11, 3, 1, 2, 1, 30, 2, 0, 0},
	.config_T15 ={0/*enable touch key*/, 27, 41, 3, 1, 0, 32, 40/*30 down the threshhold*/, 3, 0, 0},
	.config_T18 = {0, 0},
	.config_T19 = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        .config_T20 = {13, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        .config_T22 = {13, 0, 0, 25, 0, 230, 255, 4, 50, 0, 1, 0, 15, 20, 25, 30, 2},/** ZTE_MODIFY yuehongliang modified for improve the threshold when startup device, yuehongliang0005 */
        .config_T23 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/** ZTE_MODIFY yuehongliang modified for improve the threshold when startup device, yuehongliang0005 */
        .config_T24 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        .config_T25 = {3, 0, 224, 46, 136, 19, 0, 0, 0, 0, 0, 0, 0, 0},
        //.config_T27 = {0, 0, 0, 0, 0, 0, 0},
        .config_T28 = {0, 0, 3, 16, 16, 60},
        .config_T38 = {0, 0, 0, 0, 0, 0, 0,0},
};

static struct i2c_board_info atmel_ts_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(ATMEL_TS_I2C_NAME, 0x4a),
		.platform_data = &atmel_ts_pdata,
		.irq = MSM_GPIO_TO_INT(ATMEL_TS_GPIO_IRQ),
	},
};
#endif
/*[ECID:000000] ZTEBSP shihuiqin, for touchscreen_mxt224, 20120216 end*/
//zhangzhao added synaptics for 865a10 2012-8-23 start
#define SYNAPTICS_TS_I2C_NAME "synaptics-rmi-ts"

static struct regulator_bulk_data regs_synaptics[] = {
//wangbing, for mxt224 debug, s4 should be always on debfined in msm_rpm_regulator_init_data
//	{ .supply = "8921_s4",  .min_uV = 1800000, .max_uV = 1800000 },
	{ .supply = "8921_lvs4", .min_uV = 1800000, .max_uV = 1800000 },
	
};

static struct regulator_bulk_data regs_synaptics1[] = {
//wangbing, for mxt224 debug, s4 should be always on debfined in msm_rpm_regulator_init_data
//	{ .supply = "8921_s4",  .min_uV = 1800000, .max_uV = 1800000 },
	//{ .supply = "8921_l9", .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "8921_l9", .min_uV = 3000000, .max_uV = 3000000 },
	
};


#define SYNAPTICS_TS_GPIO_IRQ 32
#define SYNAPTICS_TS_RESET 33

static int synaptics_ts_platform_init(struct i2c_client *client)
{
	int rc;
	struct device *dev = &client->dev;

	pr_err("%s ---- power request\n",__func__);
	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_synaptics), regs_synaptics);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}


	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_synaptics1), regs_synaptics1);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}
//wangbing, temp for mxt224 debug, lvs not support the voltage setting.
#if 1
	rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_synaptics1), regs_synaptics1);
	if (rc) {
		dev_err(dev, "%s: could not set voltages: %d\n",
				__func__, rc);
		goto reg_free;
	}


#endif	


//wangbing, for mxt224 debug
	rc = gpio_request(SYNAPTICS_TS_RESET, "atmel_ts_reset");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, SYNAPTICS_TS_RESET);
	}
	rc = gpio_direction_output(SYNAPTICS_TS_RESET, 0);
	if (rc) {
		pr_err("%s: unable to set_direction for gpio %d\n",
				__func__, SYNAPTICS_TS_RESET);
	}

	/* configure touchscreen interrupt gpio */
	rc = gpio_request(SYNAPTICS_TS_GPIO_IRQ, "atmel_maxtouch_gpio");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, SYNAPTICS_TS_GPIO_IRQ);
		goto ts_gpio_tlmm_unconfig;
	}

//wangbing, for mxt224 debug, need internal pullup resistor.
	rc = gpio_tlmm_config(GPIO_CFG(SYNAPTICS_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, SYNAPTICS_TS_GPIO_IRQ);
		goto reg_free;
	}

//wangbing, for mxt224 debug
	regulator_bulk_enable(ARRAY_SIZE(regs_synaptics), regs_synaptics);

	regulator_bulk_enable(ARRAY_SIZE(regs_synaptics1), regs_synaptics1);

	msleep(50);
	gpio_set_value_cansleep(SYNAPTICS_TS_RESET, 1); /* disp disable (resx=0) */

		dev_err(dev, "%s: get regulators over\n",__func__);

	return 0;

ts_gpio_tlmm_unconfig:
	gpio_tlmm_config(GPIO_CFG(SYNAPTICS_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
reg_free:
	regulator_bulk_free(ARRAY_SIZE(regs_atmel), regs_atmel);
out:
		dev_err(dev, "%s: get regulators faild\n",__func__);
	return rc;
}

static int synaptics_ts_platform_exit(void)
{
	gpio_free(SYNAPTICS_TS_GPIO_IRQ);
	gpio_tlmm_config(GPIO_CFG(SYNAPTICS_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	gpio_free(SYNAPTICS_TS_RESET);
	gpio_tlmm_config(GPIO_CFG(SYNAPTICS_TS_RESET, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	regulator_bulk_disable(ARRAY_SIZE(regs_synaptics1), regs_synaptics1);
	regulator_bulk_free(ARRAY_SIZE(regs_synaptics), regs_synaptics);
	return 0;
}



static struct synaptics_ts_platform_data synaptics_ts_pdata = {
	.irq_gpio = SYNAPTICS_TS_GPIO_IRQ,
	.reset_gpio = SYNAPTICS_TS_RESET,
	//.irq_flags = IRQF_TRIGGER_FALLING,
	.init_platform_hw = synaptics_ts_platform_init,
	.exit_platform_hw = synaptics_ts_platform_exit,
};

static struct i2c_board_info synaptics_ts_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(SYNAPTICS_TS_I2C_NAME, 0x22),
		.platform_data = &synaptics_ts_pdata,
		.irq = MSM_GPIO_TO_INT(SYNAPTICS_TS_GPIO_IRQ),
	},
};
//zhangzhao added synaptics for 865a10 2012-8-23 end

/*haowiewie add atmel touch for p864a20 2012-9-26 start */
#define ATMEL_MXT_TS_I2C_NAME "atmel_mxt_ts"

static struct regulator_bulk_data regs_atmel_mxt[] = {
//wangbing, for mxt224 debug, s4 should be always on debfined in msm_rpm_regulator_init_data
//	{ .supply = "8921_s4",  .min_uV = 1800000, .max_uV = 1800000 },
	{ .supply = "8921_lvs4", .min_uV = 1800000, .max_uV = 1800000 },
	
};

static struct regulator_bulk_data regs_atmet_mxtx[] = {
//wangbing, for mxt224 debug, s4 should be always on debfined in msm_rpm_regulator_init_data
//	{ .supply = "8921_s4",  .min_uV = 1800000, .max_uV = 1800000 },
	//{ .supply = "8921_l9", .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "8921_l9", .min_uV = 3000000, .max_uV = 3000000 },
	
};

#define ATMEL_MXT_TS_GPIO_IRQ 32
#define ATMEL_MXT_TS_RESET 33

static int atmel_mxt_ts_platform_init(struct i2c_client *client)
{
	int rc;
	struct device *dev = &client->dev;

	pr_err("%s ---- power request\n",__func__);
	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_atmel_mxt), regs_atmel_mxt);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}


	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_atmet_mxtx), regs_atmet_mxtx);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}
//wangbing, temp for mxt224 debug, lvs not support the voltage setting.
#if 1
	rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_atmet_mxtx), regs_atmet_mxtx);
	if (rc) {
		dev_err(dev, "%s: could not set voltages: %d\n",
				__func__, rc);
		goto reg_free;
	}


#endif	


//wangbing, for mxt224 debug
	rc = gpio_request(ATMEL_MXT_TS_RESET, "atmel_ts_reset");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, ATMEL_MXT_TS_RESET);
	}
	rc = gpio_direction_output(ATMEL_MXT_TS_RESET, 0);
	if (rc) {
		pr_err("%s: unable to set_direction for gpio %d\n",
				__func__, ATMEL_MXT_TS_RESET);
	}

	/* configure touchscreen interrupt gpio */
	rc = gpio_request(ATMEL_MXT_TS_GPIO_IRQ, "atmel_maxtouch_gpio");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, ATMEL_MXT_TS_GPIO_IRQ);
		goto ts_gpio_tlmm_unconfig;
	}

//wangbing, for mxt224 debug, need internal pullup resistor.
	rc = gpio_tlmm_config(GPIO_CFG(ATMEL_MXT_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, ATMEL_MXT_TS_GPIO_IRQ);
		goto reg_free;
	}

//wangbing, for mxt224 debug
	regulator_bulk_enable(ARRAY_SIZE(regs_atmel_mxt), regs_atmel_mxt);

	regulator_bulk_enable(ARRAY_SIZE(regs_atmet_mxtx), regs_atmet_mxtx);

	msleep(50);
	gpio_set_value_cansleep(ATMEL_MXT_TS_RESET, 1); /* disp disable (resx=0) */
	msleep(30);
//	gpio_set_value_cansleep(ATMEL_MXT_TS_RESET, 1);

		dev_err(dev, "%s: get regulators over\n",__func__);

	return 0;

ts_gpio_tlmm_unconfig:
	gpio_tlmm_config(GPIO_CFG(ATMEL_MXT_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
reg_free:
	regulator_bulk_free(ARRAY_SIZE(regs_atmel_mxt), regs_atmel_mxt);
out:
		dev_err(dev, "%s: get regulators faild\n",__func__);
	return rc;
}

static int atmel_mxt_ts_platform_exit(void)
{
	gpio_free(ATMEL_MXT_TS_GPIO_IRQ);
	gpio_tlmm_config(GPIO_CFG(ATMEL_MXT_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	gpio_free(ATMEL_MXT_TS_RESET);
	gpio_tlmm_config(GPIO_CFG(ATMEL_MXT_TS_RESET, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	regulator_bulk_disable(ARRAY_SIZE(regs_atmet_mxtx), regs_atmet_mxtx);
	regulator_bulk_free(ARRAY_SIZE(regs_atmel_mxt), regs_atmel_mxt);
	return 0;
}



static struct mxt_platform_data atmel_mxt_ts_pdata = {
	.irq_gpio = ATMEL_MXT_TS_GPIO_IRQ,
	.reset_gpio = ATMEL_MXT_TS_RESET,
	.irqflags = IRQF_TRIGGER_FALLING,
	.init_platform_hw = atmel_mxt_ts_platform_init,
	.exit_platform_hw = atmel_mxt_ts_platform_exit,
};

static struct i2c_board_info atmel_mxt_ts_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(ATMEL_MXT_TS_I2C_NAME, 0x4a),
		.platform_data = &atmel_mxt_ts_pdata,
		.irq = MSM_GPIO_TO_INT(ATMEL_MXT_TS_GPIO_IRQ),
	},
};
/*haowiewie add atmel touch for p864a20 2012-9-26 end */
//[ECID:000000]zhangqi add mhl begin 20120704
#ifdef CONFIG_FB_MSM_HDMI_MHL_8334
#define MHL_GPIO_INT            55
#define MHL_GPIO_RESET          62

static void mhl_sii_reset_gpio(int on)
{
	printk("zhangqi add %s  on =%d \n",__func__,on);
	gpio_set_value(MHL_GPIO_RESET, on);
	return;
}

/*
 * Request for GPIO allocations
 * Set appropriate GPIO directions
 */
static int mhl_sii_gpio_setup(int on)
{
	int ret,rc;
	static struct regulator *reg_8921_l12;

	printk("zhangqi add %s  on =%d \n",__func__,on);

	if (!reg_8921_l12) {
		reg_8921_l12 = regulator_get(NULL, "8921_l12");
		if (IS_ERR(reg_8921_l12)) {
			pr_err("could not get 8921_l12, rc = %ld\n",
				PTR_ERR(reg_8921_l12));
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_l12, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage failed for 8921_l12, rc=%d\n", rc);
			return -EINVAL;
		}
	}




	if (on) {

		rc = regulator_enable(reg_8921_l12);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_8921_l12", rc);
			return rc;
		}
		
		ret = gpio_request(MHL_GPIO_RESET, "W_RST#");
		if (ret < 0) {
			pr_err("GPIO RESET request failed: %d\n", ret);
			return -EBUSY;
		}
		ret = gpio_direction_output(MHL_GPIO_RESET, 1);
		if (ret < 0) {
			pr_err("SET GPIO RESET direction failed: %d\n", ret);
			gpio_free(MHL_GPIO_RESET);
			return -EBUSY;
		}
		ret = gpio_request(MHL_GPIO_INT, "W_INT");
		if (ret < 0) {
			pr_err("GPIO INT request failed: %d\n", ret);
			gpio_free(MHL_GPIO_RESET);
			return -EBUSY;
		}
		ret = gpio_direction_input(MHL_GPIO_INT);
		if (ret < 0) {
			pr_err("SET GPIO INTR direction failed: %d\n", ret);
			gpio_free(MHL_GPIO_RESET);
			gpio_free(MHL_GPIO_INT);
			return -EBUSY;
		}
	} else {
		gpio_free(MHL_GPIO_RESET);
		gpio_free(MHL_GPIO_INT);
	}

	return 0;
}

static struct msm_mhl_platform_data mhl_platform_data = {
	.gpio_setup = mhl_sii_gpio_setup,
	.reset_pin = mhl_sii_reset_gpio,
};




#endif

static struct i2c_board_info sii_device_info[] __initdata = {
#ifdef CONFIG_FB_MSM_HDMI_MHL_8334
	{
		/*
		 * keeps SI 8334 as the default
		 * MHL TX
		 */
		I2C_BOARD_INFO("sii8334_PAGE_TPI", 0x39),
		.platform_data = &mhl_platform_data,
		.flags = I2C_CLIENT_WAKE,
		.irq = MSM_GPIO_TO_INT(55),
	},
	{
	//	I2C_BOARD_INFO("sii8334_PAGE_TX_L0", 0x39),
	//	.platform_data = &mhl_platform_data,
	},
	{
		I2C_BOARD_INFO("sii8334_PAGE_TX_L1", 0x3d),
		.platform_data = &mhl_platform_data,
	},
	{
		I2C_BOARD_INFO("sii8334_PAGE_TX_2", 0x49),
		.platform_data = &mhl_platform_data,
	},
	{
		I2C_BOARD_INFO("sii8334_PAGE_TX_3", 0x4d),
		.platform_data = &mhl_platform_data,
	},
	{
		I2C_BOARD_INFO("sii8334_PAGE_CBUS", 0x64),
		.platform_data = &mhl_platform_data,
	},

#endif
};

//[ECID:000000]zhangqi add mhl end 20120704
/*[ECID:000000] ZTEBSP fanjiankang, for sensor, 20120628 begin*/
#if defined(CONFIG_TAOS_ALS) || defined(CONFIG_TAOS_27723)
#define TMD2771_IRQ_GPIO  45
#define AKM8963_IRQ_GPIO  56
#endif

#ifdef CONFIG_SENSORS_AK8963
static struct akm8963_platform_data akm_platform_data_8963 = {
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01) || defined (CONFIG_PROJECT_P864A20) \
	|| defined (CONFIG_PROJECT_P864G02)
	.gpio_DRDY = 134,
	.gpio_RST = 0,
	.layout = 5,
	.outbit = 1,
#else
	.gpio_DRDY = 134,
	.gpio_RST = 0,
	.layout = 3,
	.outbit = 1,
#endif
};
#endif

#ifdef  CONFIG_TI_ST_ACC_330D
struct lsm330dlc_acc_platform_data lsm330d_acc_plt_dat = {
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)
                .poll_interval = 20,          //Driver polling interval as 50ms
                .min_interval = 10,    //Driver polling interval minimum 10ms
                .g_range = LSM330DLC_ACC_G_2G,    //Full Scale of LSM303DLH Accelerometer  
                .axis_map_x = 1,      //x = y 
                .axis_map_y = 0,      //y = x 
                .axis_map_z = 2,      //z = z 
                .negate_x = 1,      //x = -x 
                .negate_y = 1,      //y = -y 
                .negate_z = 0,      //z = +z 
                .gpio_int1 = -1,
                .gpio_int2 = -1,
#elif defined (CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
                .poll_interval = 20,          //Driver polling interval as 50ms
                .min_interval = 10,    //Driver polling interval minimum 10ms
                .g_range = LSM330DLC_ACC_G_2G,    //Full Scale of LSM303DLH Accelerometer  
                .axis_map_x = 1,      //x = y 
                .axis_map_y = 0,      //y = x 
                .axis_map_z = 2,      //z = z 
                .negate_x = 0,      //x = +x        //yangze a20
                .negate_y = 0,      //y = +y        //yangze a20
                .negate_z = 0,      //z = +z 
                .gpio_int1 = -1,
                .gpio_int2 = -1,
#else
                .poll_interval = 20,          //Driver polling interval as 50ms
                .min_interval = 10,    //Driver polling interval minimum 10ms
                .g_range = LSM330DLC_ACC_G_2G,    //Full Scale of LSM303DLH Accelerometer  
                .axis_map_x = 0,      //x = x 
                .axis_map_y = 1,      //y = y 
                .axis_map_z = 2,      //z = z 
                .negate_x = 0,      //x = +x 
                .negate_y = 0,      //y = +y 
                .negate_z = 1,      //z = +z 
                .gpio_int1 = -1,
                .gpio_int2 = -1,
#endif
}; 
#endif

#ifdef  CONFIG_TI_ST_GYRO_330D
struct lsm330dlc_gyr_platform_data lsm330d_gyro_plt_dat = {
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)
                .fs_range = LSM330DLC_GYR_FS_2000DPS,
                .axis_map_x = 1,                //x = x
                .axis_map_y = 0,      //y = y
                .axis_map_z = 2,      //z = z
                .negate_x = 1,      //x = +x 
                .negate_y = 1,      //y = +y 
                .negate_z = 0,      //z = +z 
             	   .poll_interval=10,
       	   .min_interval=10,
  //     	   .gpio_int1=29,
 //      	   .gpio_int2=77,
#elif defined (CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
                .fs_range = LSM330DLC_GYR_FS_2000DPS,
                .axis_map_x = 1,                //x = x
                .axis_map_y = 0,      //y = y
                .axis_map_z = 2,      //z = z
                .negate_x = 0,      //x = +x 
                .negate_y = 0,      //y = +y 
                .negate_z = 0,      //z = +z 
             	   .poll_interval=10,
       	   .min_interval=10,
  //     	   .gpio_int1=29,
 //      	   .gpio_int2=77,
#else
                .fs_range = LSM330DLC_GYR_FS_2000DPS,
                .axis_map_x = 0,                //x = x
                .axis_map_y = 1,      //y = y
                .axis_map_z = 2,      //z = z
                .negate_x = 0,      //x = +x 
                .negate_y = 0,      //y = +y 
                .negate_z = 1,      //z = +z 
             	   .poll_interval=10,
       	   .min_interval=10,
  //     	   .gpio_int1=29,
 //      	   .gpio_int2=77,
#endif
};
#endif

static struct i2c_board_info  sensor_i2c0_boardinfo[] __initdata ={
#if 0
#ifdef CONFIG_TAOS_ALS
        {   
                I2C_BOARD_INFO("taos", 0x39),
                .irq = MSM_GPIO_TO_INT(TMD2771_IRQ_GPIO),
        },  
#endif
#endif

#ifdef CONFIG_TI_ST_ACC_330D
  {
     I2C_BOARD_INFO("lsm330dlc_acc", 0x19),      
    .platform_data = (void *)&lsm330d_acc_plt_dat,
  },
#endif
#ifdef CONFIG_TI_ST_GYRO_330D

  {
     I2C_BOARD_INFO("lsm330dlc_gyr", 0x6a),
    .platform_data = (void *)&lsm330d_gyro_plt_dat,
  },
#endif

#ifdef CONFIG_SENSORS_AK8963
  {
	I2C_BOARD_INFO("akm8963", 0x0C),
//	.flags = I2C_CLIENT_WAKE,
	.platform_data = &akm_platform_data_8963,
	.irq = MSM_GPIO_TO_INT(56),
  }
#endif
};
#if defined(CONFIG_TAOS_ALS) || defined(CONFIG_TAOS_27723)
static int taos_gpio_config(void)
{
       int rc = 0;
     	/* configure taos interrupt gpio */
	rc = gpio_request(TMD2771_IRQ_GPIO, "taos_gpio");
	if (rc) {
                printk("taos error gpio request\n");
	}

	rc = gpio_tlmm_config(GPIO_CFG(TMD2771_IRQ_GPIO, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_14MA), GPIO_CFG_ENABLE);
	if (rc) {
                printk("taos error gpio config\n");
	}

	rc = gpio_request(AKM8963_IRQ_GPIO, "akm_gpio");
	if (rc) {
                printk("akm error gpio request\n");
	}

	rc = gpio_tlmm_config(GPIO_CFG(AKM8963_IRQ_GPIO, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_14MA), GPIO_CFG_ENABLE);
	if (rc) {
                printk("akm error gpio config\n");
	}


	return rc;

}
#if defined(CONFIG_TAOS_27723) || defined(CONFIG_TAOS_ALS)
static struct i2c_board_info  taos_i2c_boardinfo[] __initdata ={
        {   
              	I2C_BOARD_INFO("taos", 0x39),
                .irq = MSM_GPIO_TO_INT(TMD2771_IRQ_GPIO),
             // .platform_data	= &tsl2772_data,
        },
};
#endif 

#endif
#if defined(CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
//added by fanjiankang begin
static int morata_gpio_config(void)
{
       int rc = 0;
	printk("morata gpio config\n");
     	/* configure taos interrupt gpio */
	rc = gpio_request(34, "morata_gpio");
	if (rc) {
                printk("morata error gpio request\n");
	}

#if 0
	rc = gpio_tlmm_config(GPIO_CFG(34, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_14MA), GPIO_CFG_ENABLE);
	if (rc) {
                printk("morata error gpio config\n");
	}
#endif
	rc = gpio_direction_output(34, 1);
	if (rc) {
		pr_err("%s: unable to set_direction for MORATA gpio\n",
				__func__);
	}

	return rc;

}
//added by fanjiankang end
#endif
static int sensor_power_init(int on)
{
	static struct regulator *reg_8921_l9, *reg_8921_lvs4;
	static int prev_on=0;
	int rc;

       printk("fanjiankang goes to sensor_power_init\n");
	if (on == prev_on)
		return 0;
       

	if (!reg_8921_l9) {
		reg_8921_l9 = regulator_get(NULL, "8921_l9");
		if (IS_ERR(reg_8921_l9)) {
			pr_err("could not get reg_8921_l9, rc = %ld\n",
				PTR_ERR(reg_8921_l9));
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_l9, 3000000, 3000000);
		if (rc) {
			pr_err("set_voltage failed for reg_8921_l9, rc=%d\n", rc);
			return -EINVAL;
		}
	}

	if (!reg_8921_lvs4) {
		reg_8921_lvs4 = regulator_get(NULL, "8921_lvs4");
		if (IS_ERR(reg_8921_lvs4)) {
			pr_err("could not get reg_8921_lvs4, rc = %ld\n",
				PTR_ERR(reg_8921_lvs4));
			return -ENODEV;
		}
	}

       printk("fanjiankang can go here sensor:\n");
	if (on) {

		rc = regulator_enable(reg_8921_l9);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_8921_l9", rc);
			return rc;
		}
		rc = regulator_enable(reg_8921_lvs4);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_8921_lvs4", rc);
			return rc;
		}
		
	} else {

		rc = regulator_disable(reg_8921_l9);
		if (rc) {
			pr_err("disable reg_8921_l9 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_8921_lvs4);
		if (rc) {
			pr_err("disable reg_8921_lvs4 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	}

	prev_on = on;

	return rc;
}
/*[ECID:000000] ZTEBSP fanjiankang, for sensor, 20120628 end*/

#define MSM_WCNSS_PHYS	0x03000000
#define MSM_WCNSS_SIZE	0x280000

static struct resource resources_wcnss_wlan[] = {
	{
		.start	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.end	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.name	= "wcnss_wlanrx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.end	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.name	= "wcnss_wlantx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_WCNSS_PHYS,
		.end	= MSM_WCNSS_PHYS + MSM_WCNSS_SIZE - 1,
		.name	= "wcnss_mmio",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 64,
		.end	= 68,
		.name	= "wcnss_gpios_5wire",
		.flags	= IORESOURCE_IO,
	},
};

static struct qcom_wcnss_opts qcom_wcnss_pdata = {
	.has_48mhz_xo	= 1,
};

static struct platform_device msm_device_wcnss_wlan = {
	.name		= "wcnss_wlan",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_wcnss_wlan),
	.resource	= resources_wcnss_wlan,
	.dev		= {.platform_data = &qcom_wcnss_pdata},
};

static struct platform_device msm_device_iris_fm __devinitdata = {
	.name = "iris_fm",
	.id   = -1,
};

#ifdef CONFIG_QSEECOM
/* qseecom bus scaling */
static struct msm_bus_vectors qseecom_clks_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = 0,
		.ab = 0,
	},
};

static struct msm_bus_vectors qseecom_enable_dfab_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 70000000UL,
		.ib = 70000000UL,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 2480000000UL,
		.ib = 2480000000UL,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = 0,
		.ab = 0,
	},
};

static struct msm_bus_vectors qseecom_enable_sfpb_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = (64 * 8) * 1000000UL,
		.ab = (64 * 8) *  100000UL,
	},
};

static struct msm_bus_vectors qseecom_enable_dfab_sfpb_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 70000000UL,
		.ib = 70000000UL,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 2480000000UL,
		.ib = 2480000000UL,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = (64 * 8) * 1000000UL,
		.ab = (64 * 8) *  100000UL,
	},
};

static struct msm_bus_paths qseecom_hw_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(qseecom_clks_init_vectors),
		qseecom_clks_init_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_dfab_vectors),
		qseecom_enable_dfab_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_sfpb_vectors),
		qseecom_enable_sfpb_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_dfab_sfpb_vectors),
		qseecom_enable_dfab_sfpb_vectors,
	},
};

static struct msm_bus_scale_pdata qseecom_bus_pdata = {
	qseecom_hw_bus_scale_usecases,
	ARRAY_SIZE(qseecom_hw_bus_scale_usecases),
	.name = "qsee",
};

static struct platform_device qseecom_device = {
	.name		= "qseecom",
	.id		= 0,
	.dev		= {
		.platform_data = &qseecom_bus_pdata,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

#define QCE_SIZE		0x10000
#define QCE_0_BASE		0x11000000

#define QCE_HW_KEY_SUPPORT	0
#define QCE_SHA_HMAC_SUPPORT	1
#define QCE_SHARE_CE_RESOURCE	3
#define QCE_CE_SHARED		0

static struct resource qcrypto_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV8064_CE_IN_CHAN,
		.end = DMOV8064_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV8064_CE_IN_CRCI,
		.end = DMOV8064_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV8064_CE_OUT_CRCI,
		.end = DMOV8064_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource qcedev_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV8064_CE_IN_CHAN,
		.end = DMOV8064_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV8064_CE_IN_CRCI,
		.end = DMOV8064_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV8064_CE_OUT_CRCI,
		.end = DMOV8064_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)

static struct msm_ce_hw_support qcrypto_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcrypto_device = {
	.name		= "qcrypto",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcrypto_resources),
	.resource	= qcrypto_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcrypto_ce_hw_suppport,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

static struct msm_ce_hw_support qcedev_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcedev_device = {
	.name		= "qce",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcedev_resources),
	.resource	= qcedev_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcedev_ce_hw_suppport,
	},
};
#endif

static struct mdm_vddmin_resource mdm_vddmin_rscs = {
	.rpm_id = MSM_RPM_ID_VDDMIN_GPIO,
	.ap2mdm_vddmin_gpio = 30,
	.modes  = 0x03,
	.drive_strength = 8,
	.mdm2ap_vddmin_gpio = 80,
};

static struct gpiomux_setting mdm2ap_status_gpio_run_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct mdm_platform_data mdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
	.early_power_on = 1,
	.sfr_query = 1,
	.send_shdn = 1,
	.vddmin_resource = &mdm_vddmin_rscs,
	.peripheral_platform_device = &apq8064_device_hsic_host,
	.ramdump_timeout_ms = 120000,
	.mdm2ap_status_gpio_run_cfg = &mdm2ap_status_gpio_run_cfg,
	.sysmon_subsys_id_valid = 1,
	.sysmon_subsys_id = SYSMON_SS_EXT_MODEM,
};

static struct mdm_platform_data amdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
	.early_power_on = 1,
	.sfr_query = 1,
	.send_shdn = 1,
	.vddmin_resource = &mdm_vddmin_rscs,
	.peripheral_platform_device = &apq8064_device_hsic_host,
	.ramdump_timeout_ms = 120000,
	.mdm2ap_status_gpio_run_cfg = &mdm2ap_status_gpio_run_cfg,
	.sysmon_subsys_id_valid = 1,
	.sysmon_subsys_id = SYSMON_SS_EXT_MODEM,
	.no_a2m_errfatal_on_ssr = 1,
};

static struct mdm_vddmin_resource bmdm_vddmin_rscs = {
	.rpm_id = MSM_RPM_ID_VDDMIN_GPIO,
	.ap2mdm_vddmin_gpio = 30,
	.modes  = 0x03,
	.drive_strength = 8,
	.mdm2ap_vddmin_gpio = 64,
};

static struct mdm_platform_data bmdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
	.sfr_query = 1,
	.send_shdn = 1,
	.vddmin_resource = &bmdm_vddmin_rscs,
	.peripheral_platform_device = &apq8064_device_ehci_host3,
	.ramdump_timeout_ms = 120000,
	.mdm2ap_status_gpio_run_cfg = &mdm2ap_status_gpio_run_cfg,
	.sysmon_subsys_id_valid = 1,
	.sysmon_subsys_id = SYSMON_SS_EXT_MODEM2,
	.no_a2m_errfatal_on_ssr = 1,
};

static struct mdm_platform_data sglte2_mdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
	.early_power_on = 1,
	.sfr_query = 1,
	.vddmin_resource = &mdm_vddmin_rscs,
	.peripheral_platform_device = &apq8064_device_hsic_host,
	.ramdump_timeout_ms = 120000,
	.mdm2ap_status_gpio_run_cfg = &mdm2ap_status_gpio_run_cfg,
	.sysmon_subsys_id_valid = 1,
	.sysmon_subsys_id = SYSMON_SS_EXT_MODEM,
	.no_a2m_errfatal_on_ssr = 1,
	.subsys_name = "external_modem_mdm",
};

static struct mdm_platform_data sglte2_qsc_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
     /* delay between two PS_HOLDs */
	.ps_hold_delay_ms = 500,
	.ramdump_timeout_ms = 600000,
	.no_powerdown_after_ramdumps = 1,
	.image_upgrade_supported = 1,
	.no_a2m_errfatal_on_ssr = 1,
	.kpd_not_inverted = 1,
	.subsys_name = "external_modem",
};

static struct tsens_platform_data apq_tsens_pdata  = {
		.tsens_factor		= 1000,
		.hw_type		= APQ_8064,
		.tsens_num_sensor	= 11,
		.slope = {1176, 1176, 1154, 1176, 1111,
			1132, 1132, 1199, 1132, 1199, 1132},
};

static struct platform_device msm_tsens_device = {
	.name   = "tsens8960-tm",
	.id = -1,
};

static struct msm_thermal_data msm_thermal_pdata = {
	.sensor_id = 7,
	.poll_ms = 250,
	.limit_temp_degC = 60,
	.temp_hysteresis_degC = 10,
	.freq_step = 2,
	.core_limit_temp_degC = 80,
	.core_temp_hysteresis_degC = 10,
	.core_control_mask = 0xe,
};

#define MSM_SHARED_RAM_PHYS 0x80000000
static void __init apq8064_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_apq8064_io();
	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}

static void __init apq8064_init_irq(void)
{
	struct msm_mpm_device_data *data = NULL;

#ifdef CONFIG_MSM_MPM
	data = &apq8064_mpm_dev_data;
#endif

	msm_mpm_irq_extn_init(data);
	gic_init(0, GIC_PPI_START, MSM_QGIC_DIST_BASE,
						(void *)MSM_QGIC_CPU_BASE);
}

static struct platform_device msm8064_device_saw_regulator_core0 = {
	.name	= "saw-regulator",
	.id	= 0,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8921_s5,
	},
};

static struct platform_device msm8064_device_saw_regulator_core1 = {
	.name	= "saw-regulator",
	.id	= 1,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8921_s6,
	},
};

static struct platform_device msm8064_device_saw_regulator_core2 = {
	.name	= "saw-regulator",
	.id	= 2,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8821_s0,
	},
};

static struct platform_device msm8064_device_saw_regulator_core3 = {
	.name	= "saw-regulator",
	.id	= 3,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8821_s1,

	},
};

static struct msm_rpmrs_level msm_rpmrs_levels[] = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1, 784, 180000, 100,
	},

	{
		MSM_PM_SLEEP_MODE_RETENTION,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		415, 715, 340827, 475,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1300, 228, 1200000, 2000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, GDHS, MAX, ACTIVE),
		false,
		2000, 138, 1208400, 3200,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		6000, 119, 1850300, 9000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, GDHS, MAX, ACTIVE),
		false,
		9200, 68, 2839200, 16400,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MAX, ACTIVE),
		false,
		10300, 63, 3128000, 18200,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		18000, 10, 4602600, 27000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, RET_HIGH, RET_LOW),
		false,
		20000, 2, 5752000, 32000,
	},
};

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_TZ,
};

static struct msm_rpmrs_platform_data msm_rpmrs_data __initdata = {
	.levels = &msm_rpmrs_levels[0],
	.num_levels = ARRAY_SIZE(msm_rpmrs_levels),
	.vdd_mem_levels  = {
		[MSM_RPMRS_VDD_MEM_RET_LOW]	= 750000,
		[MSM_RPMRS_VDD_MEM_RET_HIGH]	= 750000,
		[MSM_RPMRS_VDD_MEM_ACTIVE]	= 1050000,
		[MSM_RPMRS_VDD_MEM_MAX]		= 1150000,
	},
	.vdd_dig_levels = {
		[MSM_RPMRS_VDD_DIG_RET_LOW]	= 500000,
		[MSM_RPMRS_VDD_DIG_RET_HIGH]	= 750000,
		[MSM_RPMRS_VDD_DIG_ACTIVE]	= 950000,
		[MSM_RPMRS_VDD_DIG_MAX]		= 1150000,
	},
	.vdd_mask = 0x7FFFFF,
	.rpmrs_target_id = {
		[MSM_RPMRS_ID_PXO_CLK]		= MSM_RPM_ID_PXO_CLK,
		[MSM_RPMRS_ID_L2_CACHE_CTL]	= MSM_RPM_ID_LAST,
		[MSM_RPMRS_ID_VDD_DIG_0]	= MSM_RPM_ID_PM8921_S3_0,
		[MSM_RPMRS_ID_VDD_DIG_1]	= MSM_RPM_ID_PM8921_S3_1,
		[MSM_RPMRS_ID_VDD_MEM_0]	= MSM_RPM_ID_PM8921_L24_0,
		[MSM_RPMRS_ID_VDD_MEM_1]	= MSM_RPM_ID_PM8921_L24_1,
		[MSM_RPMRS_ID_RPM_CTL]		= MSM_RPM_ID_RPM_CTL,
	},
};

static uint8_t spm_wfi_cmd_sequence[] __initdata = {
	0x03, 0x0f,
};

static uint8_t spm_power_collapse_without_rpm[] __initdata = {
	0x00, 0x24, 0x54, 0x10,
	0x09, 0x03, 0x01,
	0x10, 0x54, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static uint8_t spm_retention_cmd_sequence[] __initdata = {
	0x00, 0x05, 0x03, 0x0D,
	0x0B, 0x00, 0x0f,
};

static uint8_t spm_retention_with_krait_v3_cmd_sequence[] __initdata = {
	0x42, 0x1B, 0x00,
	0x05, 0x03, 0x0D, 0x0B,
	0x00, 0x42, 0x1B,
	0x0f,
};

static uint8_t spm_power_collapse_with_rpm[] __initdata = {
	0x00, 0x24, 0x54, 0x10,
	0x09, 0x07, 0x01, 0x0B,
	0x10, 0x54, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

/* 8064AB has a different command to assert apc_pdn */
static uint8_t spm_power_collapse_without_rpm_krait_v3[] __initdata = {
	0x00, 0x24, 0x84, 0x10,
	0x09, 0x03, 0x01,
	0x10, 0x84, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static uint8_t spm_power_collapse_with_rpm_krait_v3[] __initdata = {
	0x00, 0x24, 0x84, 0x10,
	0x09, 0x07, 0x01, 0x0B,
	0x10, 0x84, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static struct msm_spm_seq_entry msm_spm_boot_cpu_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_RETENTION,
		.notify_rpm = false,
		.cmd = spm_retention_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[3] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};
static struct msm_spm_seq_entry msm_spm_nonboot_cpu_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_RETENTION,
		.notify_rpm = false,
		.cmd = spm_retention_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[3] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};

static uint8_t l2_spm_wfi_cmd_sequence[] __initdata = {
	0x00, 0x20, 0x03, 0x20,
	0x00, 0x0f,
};

static uint8_t l2_spm_gdhs_cmd_sequence[] __initdata = {
	0x00, 0x20, 0x34, 0x64,
	0x48, 0x07, 0x48, 0x20,
	0x50, 0x64, 0x04, 0x34,
	0x50, 0x0f,
};
static uint8_t l2_spm_power_off_cmd_sequence[] __initdata = {
	0x00, 0x10, 0x34, 0x64,
	0x48, 0x07, 0x48, 0x10,
	0x50, 0x64, 0x04, 0x34,
	0x50, 0x0F,
};

static struct msm_spm_seq_entry msm_spm_l2_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_L2_MODE_RETENTION,
		.notify_rpm = false,
		.cmd = l2_spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_L2_MODE_GDHS,
		.notify_rpm = true,
		.cmd = l2_spm_gdhs_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_L2_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = l2_spm_power_off_cmd_sequence,
	},
};


static struct msm_spm_platform_data msm_spm_l2_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW_L2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x00A000AE,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A00020,
		.modes = msm_spm_l2_seq_list,
		.num_modes = ARRAY_SIZE(msm_spm_l2_seq_list),
	},
};

static struct msm_spm_platform_data msm_spm_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_boot_cpu_seq_list),
		.modes = msm_spm_boot_cpu_seq_list,
	},
	[1] = {
		.reg_base_addr = MSM_SAW1_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
	[2] = {
		.reg_base_addr = MSM_SAW2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
	[3] = {
		.reg_base_addr = MSM_SAW3_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
};

static void __init apq8064ab_update_krait_spm(void)
{
	int i;

	/* Update the SPM sequences for SPC and PC */
	for (i = 0; i < ARRAY_SIZE(msm_spm_data); i++) {
		int j;
		struct msm_spm_platform_data *pdata = &msm_spm_data[i];
		for (j = 0; j < pdata->num_modes; j++) {
			if (pdata->modes[j].cmd ==
					spm_power_collapse_without_rpm)
				pdata->modes[j].cmd =
				spm_power_collapse_without_rpm_krait_v3;
			else if (pdata->modes[j].cmd ==
					spm_power_collapse_with_rpm)
				pdata->modes[j].cmd =
				spm_power_collapse_with_rpm_krait_v3;
		}
	}
}

static void __init apq8064_init_buses(void)
{
	msm_bus_rpm_set_mt_mask();
	msm_bus_8064_apps_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_sys_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_mm_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_apps_fabric.dev.platform_data =
		&msm_bus_8064_apps_fabric_pdata;
	msm_bus_8064_sys_fabric.dev.platform_data =
		&msm_bus_8064_sys_fabric_pdata;
	msm_bus_8064_mm_fabric.dev.platform_data =
		&msm_bus_8064_mm_fabric_pdata;
	msm_bus_8064_sys_fpb.dev.platform_data = &msm_bus_8064_sys_fpb_pdata;
	msm_bus_8064_cpss_fpb.dev.platform_data = &msm_bus_8064_cpss_fpb_pdata;
}

/* PCIe gpios */
static struct msm_pcie_gpio_info_t msm_pcie_gpio_info[MSM_PCIE_MAX_GPIO] = {
	{"rst_n", PM8921_MPP_PM_TO_SYS(PCIE_RST_N_PMIC_MPP), 0},
	{"pwr_en", PM8921_GPIO_PM_TO_SYS(PCIE_PWR_EN_PMIC_GPIO), 1},
};

static struct msm_pcie_platform msm_pcie_platform_data = {
	.gpio = msm_pcie_gpio_info,
	.axi_addr = PCIE_AXI_BAR_PHYS,
	.axi_size = PCIE_AXI_BAR_SIZE,
	.wake_n = PM8921_GPIO_IRQ(PM8921_IRQ_BASE, PCIE_WAKE_N_PMIC_GPIO),
};

static int __init mpq8064_pcie_enabled(void)
{
	return !((readl_relaxed(QFPROM_RAW_FEAT_CONFIG_ROW0_MSB) & BIT(21)) ||
		(readl_relaxed(QFPROM_RAW_OEM_CONFIG_ROW0_LSB) & BIT(4)));
}

static void __init mpq8064_pcie_init(void)
{
	if (mpq8064_pcie_enabled()) {
		msm_device_pcie.dev.platform_data = &msm_pcie_platform_data;
		platform_device_register(&msm_device_pcie);
	}
}

static struct platform_device apq8064_device_ext_5v_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_MPP_PM_TO_SYS(7),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_5V],
	},
};

static struct platform_device apq8064_device_ext_mpp8_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_MPP_PM_TO_SYS(8),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_MPP8],
	},
};

static struct platform_device apq8064_device_ext_3p3v_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= APQ8064_EXT_3P3V_REG_EN_GPIO,
	.dev	= {
		.platform_data =
			&apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_3P3V],
	},
};

static struct platform_device apq8064_device_ext_ts_sw_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_GPIO_PM_TO_SYS(23),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_TS_SW],
	},
};

static struct platform_device apq8064_device_rpm_regulator __devinitdata = {
	.name	= "rpm-regulator",
	.id	= 0,
	.dev	= {
		.platform_data = &apq8064_rpm_regulator_pdata,
	},
};
/*
static struct platform_device
apq8064_pm8921_device_rpm_regulator __devinitdata = {
	.name	= "rpm-regulator",
	.id	= 1,
	.dev	= {
		.platform_data = &apq8064_rpm_regulator_pm8921_pdata,
	},
};
*/
static struct gpio_ir_recv_platform_data gpio_ir_recv_pdata = {
	.gpio_nr = 88,
	.active_low = 1,
};

static struct platform_device gpio_ir_recv_pdev = {
	.name = "gpio-rc-recv",
	.dev = {
		.platform_data = &gpio_ir_recv_pdata,
	},
};

static struct platform_device *common_not_mpq_devices[] __initdata = {
	&apq8064_device_qup_i2c_gsbi1,
	&apq8064_device_qup_i2c_gsbi2,         //added by fanjiankang begin
	&apq8064_device_qup_i2c_gsbi3,
	&apq8064_device_qup_i2c_gsbi4,
};

static struct platform_device *early_common_devices[] __initdata = {
	&apq8064_device_acpuclk,
	&apq8064_device_dmov,
//shihuiqin modified
#ifndef SPI_SW_CONTROL    /*SPI_HW_CONTROL*/
	&apq8064_device_qup_spi_gsbi5,
#endif
};

static struct platform_device *pm8921_common_devices[] __initdata = {
	&apq8064_device_ext_5v_vreg,
	&apq8064_device_ext_mpp8_vreg,
	&apq8064_device_ext_3p3v_vreg,
	&apq8064_device_ssbi_pmic1,
	&apq8064_device_ssbi_pmic2,
};

static struct platform_device *pm8917_common_devices[] __initdata = {
	&apq8064_device_ext_mpp8_vreg,
	&apq8064_device_ext_3p3v_vreg,
	&apq8064_device_ssbi_pmic1,
	&apq8064_device_ssbi_pmic2,
};

static struct platform_device *common_devices[] __initdata = {
	&msm_device_smd_apq8064,
	&apq8064_device_otg,
	&apq8064_device_gadget_peripheral,
	&apq8064_device_hsusb_host,
	&android_usb_device,
	&msm_device_wcnss_wlan,
	/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20121010 start*/
#if !defined(CONFIG_PROJECT_P864A20) && !defined(CONFIG_PROJECT_P864G02) 
	&msm_device_iris_fm,
#endif
	/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20121010 end*/
	&apq8064_fmem_device,
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	&apq8064_android_pmem_device,
	&apq8064_android_pmem_adsp_device,
	&apq8064_android_pmem_audio_device,
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/
#ifdef CONFIG_ION_MSM
	&apq8064_ion_dev,
#endif
	&msm8064_device_watchdog,
	&msm8064_device_saw_regulator_core0,
	&msm8064_device_saw_regulator_core1,
	&msm8064_device_saw_regulator_core2,
	&msm8064_device_saw_regulator_core3,
#if defined(CONFIG_QSEECOM)
	&qseecom_device,
#endif

	&msm_8064_device_tsif[0],
	&msm_8064_device_tsif[1],

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
	&qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
	&qcedev_device,
#endif

#ifdef CONFIG_HW_RANDOM_MSM
	&apq8064_device_rng,
#endif
	&apq_pcm,
	&apq_pcm_routing,
	&apq_cpudai0,
	&apq_cpudai1,
	&mpq_cpudai_sec_i2s_rx,
	&mpq_cpudai_mi2s_tx,
	&apq_cpudai_hdmi_rx,
	&apq_cpudai_bt_rx,
	&apq_cpudai_bt_tx,
	&apq_cpudai_fm_rx,
	&apq_cpudai_fm_tx,
	&apq_cpu_fe,
	&apq_stub_codec,
	&apq_voice,
	&apq_voip,
	&apq_lpa_pcm,
	&apq_compr_dsp,
	&apq_multi_ch_pcm,
	&apq_lowlatency_pcm,
	&apq_pcm_hostless,
	&apq_cpudai_afe_01_rx,
	&apq_cpudai_afe_01_tx,
	&apq_cpudai_afe_02_rx,
	&apq_cpudai_afe_02_tx,
	&apq_pcm_afe,
	&apq_cpudai_auxpcm_rx,
	&apq_cpudai_auxpcm_tx,
	&apq_cpudai_stub,
	&apq_cpudai_slimbus_1_rx,
	&apq_cpudai_slimbus_1_tx,
	&apq_cpudai_slimbus_2_rx,
	&apq_cpudai_slimbus_2_tx,
	&apq_cpudai_slimbus_3_rx,
	&apq_cpudai_slimbus_3_tx,
	&apq8064_rpm_device,
	&apq8064_rpm_log_device,
	&apq8064_rpm_stat_device,
	&apq8064_rpm_master_stat_device,
	&apq_device_tz_log,
	&msm_bus_8064_apps_fabric,
	&msm_bus_8064_sys_fabric,
	&msm_bus_8064_mm_fabric,
	&msm_bus_8064_sys_fpb,
	&msm_bus_8064_cpss_fpb,
	&apq8064_msm_device_vidc,
	&msm_pil_dsps,
	&msm_8960_q6_lpass,
	&msm_pil_vidc,
	&msm_gss,
	&apq8064_rtb_device,
	&apq8064_dcvs_device,
	&apq8064_msm_gov_device,
	&apq8064_device_cache_erp,
	&msm8960_device_ebi1_ch0_erp,
	&msm8960_device_ebi1_ch1_erp,
	&epm_adc_device,
	&coresight_tpiu_device,
	&coresight_etb_device,
	&apq8064_coresight_funnel_device,
	&coresight_etm0_device,
	&coresight_etm1_device,
	&coresight_etm2_device,
	&coresight_etm3_device,
	&apq_cpudai_slim_4_rx,
	&apq_cpudai_slim_4_tx,
#ifdef CONFIG_MSM_GEMINI
	&msm8960_gemini_device,
#endif
	&apq8064_iommu_domain_device,
	&msm_tsens_device,
	&apq8064_cache_dump_device,
	&msm_8064_device_tspp,
#ifdef CONFIG_BATTERY_BCL
	&battery_bcl_device,
#endif
	&apq8064_msm_mpd_device,
};

static struct platform_device *sim_devices[] __initdata = {
	&apq8064_device_uart_gsbi3,
	&msm_device_sps_apq8064,
};

static struct platform_device *rumi3_devices[] __initdata = {
	&apq8064_device_uart_gsbi1,
	&msm_device_sps_apq8064,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
};

//wangtao fusion3-debug begin
/*
static struct platform_device *cdp_devices[] __initdata = {
	&apq8064_device_uart_gsbi1,
	&apq8064_device_uart_gsbi7,
	&msm_device_sps_apq8064,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
};
*/

static struct platform_device *cdp_devices[] __initdata = {
	&apq8064_device_uart_gsbi3,
	&msm_device_sps_apq8064,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
};
//wangtao fusion3-debug end

static struct platform_device
mpq8064_device_ext_1p2_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 2),
	.dev	= {
		.platform_data =
		 &mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_1P2V],
	},
};

static struct platform_device
mpq8064_device_ext_1p8_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 4),
	.dev	= {
		.platform_data =
		&mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_1P8V],
	},
};

static struct platform_device
mpq8064_device_ext_2p2_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 14),
	.dev	= {
		.platform_data =
		&mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_2P2V],
	},
};

static struct platform_device
mpq8064_device_ext_5v_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 3),
	.dev	= {
		.platform_data =
		 &mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_5V],
	},
};

static struct platform_device
mpq8064_device_ext_3p3v_ldo_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 15),
	.dev	= {
		.platform_data =
		&mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_3P3V],
	},
};

static struct platform_device rc_input_loopback_pdev = {
	.name	= "rc-user-input",
	.id	= -1,
};

static int rf4ce_gpio_init(void)
{
	if (!machine_is_mpq8064_cdp() &&
		!machine_is_mpq8064_hrd() &&
			!machine_is_mpq8064_dtv())
		return -EINVAL;

	/* CC2533 SRDY Input */
	if (!gpio_request(SX150X_GPIO(4, 6), "rf4ce_srdy")) {
		gpio_direction_input(SX150X_GPIO(4, 6));
		gpio_export(SX150X_GPIO(4, 6), true);
	}

	/* CC2533 MRDY Output */
	if (!gpio_request(SX150X_GPIO(4, 5), "rf4ce_mrdy")) {
		gpio_direction_output(SX150X_GPIO(4, 5), 1);
		gpio_export(SX150X_GPIO(4, 5), true);
	}

	/* CC2533 Reset Output */
	if (!gpio_request(SX150X_GPIO(4, 7), "rf4ce_reset")) {
		gpio_direction_output(SX150X_GPIO(4, 7), 0);
		gpio_export(SX150X_GPIO(4, 7), true);
	}

	return 0;
}
late_initcall(rf4ce_gpio_init);
/*
#ifdef CONFIG_SERIAL_MSM_HS
static struct msm_serial_hs_platform_data mpq8064_gsbi6_uartdm_pdata = {
	.config_gpio		= 4,
	.uart_tx_gpio		= 14,
	.uart_rx_gpio		= 15,
	.uart_cts_gpio		= 16,
	.uart_rfr_gpio		= 17,
	.inject_rx_on_wakeup	= 1,
	.rx_to_inject		= 0xFD,
};
#else
static struct msm_serial_hs_platform_data msm_uart_dm9_pdata;
#endif
*/
static struct platform_device *mpq_devices[] __initdata = {
	&msm_device_sps_apq8064,
	&mpq8064_device_qup_i2c_gsbi5,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
	&gpio_ir_recv_pdev,
	&mpq8064_device_ext_1p2_buck_vreg,
	&mpq8064_device_ext_1p8_buck_vreg,
	&mpq8064_device_ext_2p2_buck_vreg,
	&mpq8064_device_ext_5v_buck_vreg,
	&mpq8064_device_ext_3p3v_ldo_vreg,
#ifdef CONFIG_MSM_VCAP
	&msm8064_device_vcap,
#endif
	&rc_input_loopback_pdev,
};

/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 start*/
#ifndef SPI_SW_CONTROL   /*SPI_HW_CONTROL*/
static struct msm_spi_platform_data apq8064_qup_spi_gsbi5_pdata = {
	.max_clock_speed = 1100000,
};
#endif

/*[ECID:000000] ZTEBSP zhangbo gpio 43 is used by nfc, so change this to an unused gpio, start*/
#define KS8851_IRQ_GPIO		12
/*[ECID:000000] ZTEBSP zhangbo gpio 43 is used by nfc, so change this to an unused gpio, end*/

static struct spi_board_info spi_board_info[] __initdata = {
	{
		.modalias               = "ks8851",
		.irq                    = MSM_GPIO_TO_INT(KS8851_IRQ_GPIO),
		.max_speed_hz           = 19200000,
		.bus_num                = 0,
		.chip_select            = 2,
		.mode                   = SPI_MODE_0,
	},
	{
		.modalias		= "epm_adc",
		.max_speed_hz		= 1100000,
		.bus_num		= 0,
		.chip_select		= 3,
		.mode			= SPI_MODE_0,
	},
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120531 start*/
#ifndef SPI_SW_CONTROL   /*SPI_HW_CONTROL*/		
	{
		.modalias       = "lms501_spi",
		.mode           = SPI_MODE_3|SPI_CS_HIGH,
		.bus_num        = 1,
		.chip_select    = 0,
		.max_speed_hz   = 9963243,
	},		
#endif	
/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 2012531 end*/	
};

static struct slim_boardinfo apq8064_slim_devices[] = {
	{
		.bus_num = 1,
		.slim_slave = &apq8064_slim_tabla,
	},
	{
		.bus_num = 1,
		.slim_slave = &apq8064_slim_tabla20,
	},
	/* add more slimbus slaves as needed */
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi1_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
};

//added by fanjiankang begin for gsbi2
static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi2_pdata = {
	.clk_freq = 384000,
	.src_clk_rate = 24000000,
};
//added by fanjiankang end for gsbi2

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi3_pdata = {
	.clk_freq = 384000,
	.src_clk_rate = 24000000,
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi4_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
};

#if defined(CONFIG_PROJECT_P864A20) || defined(CONFIG_PROJECT_P864G02)
//jiaobaocun added for gsbi5
static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi5_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
};
#endif

static struct msm_i2c_platform_data mpq8064_i2c_qup_gsbi5_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
};

#define GSBI_DUAL_MODE_CODE 0x60
#define MSM_GSBI1_PHYS		0x12440000
/*[ECID:000000] ZTEBSP wangbing, for fusion uart, 20120713 */
#define MSM_GSBI3_PHYS		0x16200000

static void __init apq8064_i2c_init(void)
{
	void __iomem *gsbi_mem;

/*[ECID:000000] ZTEBSP wangbing, for fusion uart, 20120713 +++*/
//fusion3 using gsbi3 as uart compared to original gsbi1
#if 0
	apq8064_device_qup_i2c_gsbi1.dev.platform_data =
					&apq8064_i2c_qup_gsbi1_pdata;
	gsbi_mem = ioremap_nocache(MSM_GSBI1_PHYS, 4);
	writel_relaxed(GSBI_DUAL_MODE_CODE, gsbi_mem);
	/* Ensure protocol code is written before proceeding */
	wmb();
	iounmap(gsbi_mem);
	apq8064_i2c_qup_gsbi1_pdata.use_gsbi_shared_mode = 1;
#endif
	apq8064_device_qup_i2c_gsbi3.dev.platform_data =
					&apq8064_i2c_qup_gsbi3_pdata;
	gsbi_mem = ioremap_nocache(MSM_GSBI3_PHYS, 4);
	writel_relaxed(GSBI_DUAL_MODE_CODE, gsbi_mem);
	/* Ensure protocol code is written before proceeding */
	wmb();
	iounmap(gsbi_mem);
	apq8064_i2c_qup_gsbi3_pdata.use_gsbi_shared_mode = 1;
/*[ECID:000000] ZTEBSP wangbing, for fusion uart, 20120713 ---*/	
	apq8064_device_qup_i2c_gsbi1.dev.platform_data =
					&apq8064_i2c_qup_gsbi1_pdata;
   //added by fanjiankang for gsbi2 i2c begin
	apq8064_device_qup_i2c_gsbi2.dev.platform_data =
 				&apq8064_i2c_qup_gsbi2_pdata;
   //added by fanjiankang for gsbi2 i2c end

	apq8064_device_qup_i2c_gsbi4.dev.platform_data =
					&apq8064_i2c_qup_gsbi4_pdata;
#if defined(CONFIG_PROJECT_P864A20) ||defined(CONFIG_PROJECT_P864G02)
   	apq8064_device_qup_i2c_gsbi5.dev.platform_data =		//jiaobaocun added for gsbi5
					&apq8064_i2c_qup_gsbi5_pdata;
#endif
	mpq8064_device_qup_i2c_gsbi5.dev.platform_data =
					&mpq8064_i2c_qup_gsbi5_pdata;
}


#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static int ethernet_init(void)
{
	int ret;
	ret = gpio_request(KS8851_IRQ_GPIO, "ks8851_irq");
	if (ret) {
		pr_err("ks8851 gpio_request failed: %d\n", ret);
		goto fail;
	}

	return 0;
fail:
	return ret;
}
#else
static int ethernet_init(void)
{
	return 0;
}
#endif

#define GPIO_KEY_HOME			PM8921_GPIO_PM_TO_SYS(27)
//wangtao fusion3-debug begin

//#define GPIO_KEY_VOLUME_UP	PM8921_GPIO_PM_TO_SYS(35)
//#define GPIO_KEY_VOLUME_DOWN	PM8921_GPIO_PM_TO_SYS(38)
#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01)||defined(CONFIG_PROJECT_P864A20) \
|| defined(CONFIG_PROJECT_P864G02)
#define GPIO_KEY_VOLUME_UP	PM8921_GPIO_PM_TO_SYS(1)
#define GPIO_KEY_VOLUME_DOWN	PM8921_GPIO_PM_TO_SYS(2)
#else
#define GPIO_KEY_VOLUME_UP	PM8921_GPIO_PM_TO_SYS(2)
#define GPIO_KEY_VOLUME_DOWN	PM8921_GPIO_PM_TO_SYS(1)
#endif
//wangtao fusion3-debug end
#define GPIO_KEY_CAM_FOCUS		PM8921_GPIO_PM_TO_SYS(3)
#define GPIO_KEY_CAM_SNAP		PM8921_GPIO_PM_TO_SYS(4)
#define GPIO_KEY_ROTATION	PM8921_GPIO_PM_TO_SYS(42)

static struct gpio_keys_button cdp_keys[] = {
	{
		.code           = KEY_HOME,
		.gpio           = GPIO_KEY_HOME,
		.desc           = "home_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = SW_ROTATE_LOCK,
		.gpio           = GPIO_KEY_ROTATION,
		.desc           = "rotate_key",
		.active_low     = 1,
		.type		= EV_SW,
		.debounce_interval = 15,
	},
};

static struct gpio_keys_platform_data cdp_keys_data = {
	.buttons        = cdp_keys,
	.nbuttons       = ARRAY_SIZE(cdp_keys),
};

static struct platform_device cdp_kp_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &cdp_keys_data,
	},
};

#if defined(CONFIG_PROJECT_P864A10) || defined(CONFIG_PROJECT_P864H01) || defined(CONFIG_PROJECT_P864A20) \
   || defined(CONFIG_PROJECT_P864G02)
static struct gpio_keys_button mtp_keys[] = {
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
};
#else
static struct gpio_keys_button mtp_keys[] = {
	{
		.code           = KEY_CAMERA_FOCUS,
		.gpio           = GPIO_KEY_CAM_FOCUS,
		.desc           = "cam_focus_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_CAMERA_SNAPSHOT,
		.gpio           = GPIO_KEY_CAM_SNAP,
		.desc           = "cam_snap_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.debounce_interval = 15,
	},
};
#endif

static struct gpio_keys_platform_data mtp_keys_data = {
	.buttons        = mtp_keys,
	.nbuttons       = ARRAY_SIZE(mtp_keys),
};

static struct platform_device mtp_kp_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &mtp_keys_data,
	},
};

static struct gpio_keys_button mpq_keys[] = {
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
};

static struct gpio_keys_platform_data mpq_keys_data = {
	.buttons        = mpq_keys,
	.nbuttons       = ARRAY_SIZE(mpq_keys),
};

static struct platform_device mpq_gpio_keys_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &mpq_keys_data,
	},
};

#define MPQ_KP_ROW_BASE		SX150X_EXP2_GPIO_BASE
#define MPQ_KP_COL_BASE		(SX150X_EXP2_GPIO_BASE + 4)

static unsigned int mpq_row_gpios[] = {MPQ_KP_ROW_BASE, MPQ_KP_ROW_BASE + 1,
				MPQ_KP_ROW_BASE + 2, MPQ_KP_ROW_BASE + 3};
static unsigned int mpq_col_gpios[] = {MPQ_KP_COL_BASE, MPQ_KP_COL_BASE + 1,
				MPQ_KP_COL_BASE + 2};

static const unsigned int mpq_keymap[] = {
	KEY(0, 0, KEY_UP),
	KEY(0, 1, KEY_ENTER),
	KEY(0, 2, KEY_3),

	KEY(1, 0, KEY_DOWN),
	KEY(1, 1, KEY_EXIT),
	KEY(1, 2, KEY_4),

	KEY(2, 0, KEY_LEFT),
	KEY(2, 1, KEY_1),
	KEY(2, 2, KEY_5),

	KEY(3, 0, KEY_RIGHT),
	KEY(3, 1, KEY_2),
	KEY(3, 2, KEY_6),
};

static struct matrix_keymap_data mpq_keymap_data = {
	.keymap_size	= ARRAY_SIZE(mpq_keymap),
	.keymap		= mpq_keymap,
};

static struct matrix_keypad_platform_data mpq_keypad_data = {
	.keymap_data		= &mpq_keymap_data,
	.row_gpios		= mpq_row_gpios,
	.col_gpios		= mpq_col_gpios,
	.num_row_gpios		= ARRAY_SIZE(mpq_row_gpios),
	.num_col_gpios		= ARRAY_SIZE(mpq_col_gpios),
	.col_scan_delay_us	= 32000,
	.debounce_ms		= 20,
	.wakeup			= 1,
	.active_low		= 1,
	.no_autorepeat		= 1,
};

static struct platform_device mpq_keypad_device = {
	.name           = "matrix-keypad",
	.id             = -1,
	.dev            = {
		.platform_data  = &mpq_keypad_data,
	},
};

#ifdef CONFIG_PN544_NFC
#define ZTE_GPIO_PN544_IRQ 29
#define ZTE_GPIO_PN544_VEN_EN 31
#define ZTE_GPIO_PN544_FIRM 43

static struct pn544_i2c_platform_data pn544_data = {
	.irq_gpio = ZTE_GPIO_PN544_IRQ,
	.ven_gpio = ZTE_GPIO_PN544_VEN_EN,
	.firm_gpio = ZTE_GPIO_PN544_FIRM,
};

static struct i2c_board_info __initdata pn544_device_info[] = {
	{
		I2C_BOARD_INFO("pn544", 0x28),
		.irq = MSM_GPIO_TO_INT(ZTE_GPIO_PN544_IRQ),
		.platform_data = &pn544_data,
	},
};
static void nfc_init(void)
{
	gpio_tlmm_config(GPIO_CFG(pn544_data.irq_gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(pn544_data.ven_gpio, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(pn544_data.firm_gpio, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
}
#endif

/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20120929 start*/
#if defined(CONFIG_I2C_SI470X) || defined(CONFIG_I2C_SI470X_MODULE)
#define ZTE_GPIO_SI470X_IRQ 22
#define ZTE_GPIO_SI470X_RESET 28
#define SI470X_CLK			PM8921_GPIO_PM_TO_SYS(43)

static int si470x_clk_enable(bool on)
{
	int rc = 0;
	printk("jiaobaocun si470x_clk_enable\n");
	gpio_set_value_cansleep(SI470X_CLK, on);

	if (on) {
		rc = pm8xxx_aux_clk_control(CLK_MP3_1, XO_DIV_32, true);
		if (rc) {
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
			goto err_gpio_dis;
		}
	} else {
		rc = pm8xxx_aux_clk_control(CLK_MP3_1, XO_DIV_NONE, true);
		if (rc)
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
	}

	return rc;

err_gpio_dis:
	gpio_set_value_cansleep(SI470X_CLK, !on);
	return rc;
}

static int si470x_power(bool on)
{
	int rc = 0;
	static struct regulator *fm_regulator;
	/*gpio config*/
	printk("jiaobaocun si470x_power\n");
	/* Voting for 1.8V Regulator to enable fm rf power*/
	fm_regulator = regulator_get(NULL , "8921_lvs5");
	if (IS_ERR(fm_regulator)) {
		rc = PTR_ERR(fm_regulator);
		pr_err("%s: could not get regulator: %d\n", __func__, rc);
	}
	else{
		pr_err("%s: get regulator ok\n", __func__);
		}
	/* Enabling the 1.8V regulator */
	rc = regulator_enable(fm_regulator);
	if (rc) {		
		regulator_put(fm_regulator);
		fm_regulator = NULL;
		pr_err("%s: could not enable regulator: %d\n", __func__, rc);
	}
	else
		{
		pr_err("%s: fm  power switch has enabled!\n",__func__);
	}
	
	return rc;
	
}	

static struct radio_si470x_platform_data si470x_pdata = {
	.reset_pin = ZTE_GPIO_SI470X_RESET,
	.power_on = si470x_power,
	.clk_enable = si470x_clk_enable,
};


static struct i2c_board_info __initdata si470x_device_info[] = {
	{
		I2C_BOARD_INFO("si470x", 0x10),
		.irq = MSM_GPIO_TO_INT(ZTE_GPIO_SI470X_IRQ),
		.platform_data = &si470x_pdata,
	},
};
static void si470x_init(void)
{
	printk("jiaobaocun si470x_init\n");
	/*gpio config*/
	gpio_tlmm_config(GPIO_CFG(ZTE_GPIO_SI470X_IRQ, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(ZTE_GPIO_SI470X_RESET, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
}	
#endif
/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20120929 end*/

/* Sensors DSPS platform data */
#define DSPS_PIL_GENERIC_NAME		"dsps"
static void __init apq8064_init_dsps(void)
{
	struct msm_dsps_platform_data *pdata =
		msm_dsps_device_8064.dev.platform_data;
	pdata->pil_name = DSPS_PIL_GENERIC_NAME;
	pdata->gpios = NULL;
	pdata->gpios_num = 0;

	platform_device_register(&msm_dsps_device_8064);
}

#define I2C_SURF 1
#define I2C_FFA  (1 << 1)
#define I2C_RUMI (1 << 2)
#define I2C_SIM  (1 << 3)
#define I2C_LIQUID (1 << 4)
#define I2C_MPQ_CDP	BIT(5)
#define I2C_MPQ_HRD	BIT(6)
#define I2C_MPQ_DTV	BIT(7)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};

static struct i2c_registry apq8064_i2c_devices[] __initdata = {
	{
		I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		smb349_charger_i2c_info,
		ARRAY_SIZE(smb349_charger_i2c_info)
	},
	{////zhangzhao added synaptics for 865a10 2012-8-23
		I2C_SURF | I2C_LIQUID|I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		synaptics_ts_i2c_info,
		ARRAY_SIZE(synaptics_ts_i2c_info),
	},
/*[ECID:000000] ZTEBSP haoweiwei, for p864a20 touchscreen_atmel, 20120924*/
	{
		I2C_SURF | I2C_LIQUID|I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		atmel_mxt_ts_i2c_info,
		ARRAY_SIZE(atmel_mxt_ts_i2c_info),
	},	
/*[ECID:000000] ZTEBSP haoweiwei, for p864a20 touchscreen_atmel, 20120924*/

/*[ECID:000000] ZTEBSP shihuiqin, for touchscreen_mxt224, 20120216*/
	{
		I2C_SURF | I2C_LIQUID,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		mxt_device_info,
		ARRAY_SIZE(mxt_device_info),
	},
#if 1
	{
		I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		atmel_ts_i2c_info,
		ARRAY_SIZE(atmel_ts_i2c_info),
	},	
/*[ECID:000000] ZTEBSP shihuiqin, for touchscreen_mxt224, 20120216*/
#endif
/*[ECID:000000] ZTEBSP zhangqi, for mhl, 20120628*/
	{
		I2C_SURF | I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		sii_device_info,
		ARRAY_SIZE(sii_device_info),
	},
/*[ECID:000000] ZTEBSP zhangqi, for mhl, 20120628*/

/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 start*/
	{
		I2C_SURF | I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI5_QUP_I2C_BUS_ID,
		isa1200_board_info,
		ARRAY_SIZE(isa1200_board_info),
	},
/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 end*/
/*[ECID:000000] ZTEBSP fanjiankang, for sensor, 20120628*/
	{
           I2C_FFA,
            APQ_8064_GSBI2_QUP_I2C_BUS_ID,
 	    sensor_i2c0_boardinfo,
	    ARRAY_SIZE(sensor_i2c0_boardinfo),

	},
#if defined(CONFIG_PROJECT_P864A20) ||defined(CONFIG_PROJECT_P864G02)
	{
      I2C_FFA,
      APQ_8064_GSBI5_QUP_I2C_BUS_ID,
 	    taos_i2c_boardinfo,
	    ARRAY_SIZE(taos_i2c_boardinfo),	
	},
#else
	{
      I2C_FFA,
      APQ_8064_GSBI2_QUP_I2C_BUS_ID,
 	    taos_i2c_boardinfo,
	    ARRAY_SIZE(taos_i2c_boardinfo),	
	},
#endif	
/*[ECID:000000] ZTEBSP fanjiankang, for sensor, 20120628*/
	{
		I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		cyttsp_info,
		ARRAY_SIZE(cyttsp_info),
	},
/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 start*/
	{
		I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI2_QUP_I2C_BUS_ID,
		isa1200_board_info,
		ARRAY_SIZE(isa1200_board_info),
	},
/*[ECID:000000] ZTEBSP doumingming for vibrator, 20120814 end*/
	{
		I2C_MPQ_CDP,
		APQ_8064_GSBI5_QUP_I2C_BUS_ID,
		cs8427_device_info,
		ARRAY_SIZE(cs8427_device_info),
	},
/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20120929 start*/
#if defined(CONFIG_I2C_SI470X) || defined(CONFIG_I2C_SI470X_MODULE)
	{
		I2C_SURF | I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI5_QUP_I2C_BUS_ID,
		si470x_device_info,
		ARRAY_SIZE(si470x_device_info),
	},
#endif
/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20120929 end*/
#ifdef CONFIG_PN544_NFC
	{
		I2C_SURF | I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		pn544_device_info,
		ARRAY_SIZE(pn544_device_info),
	},
#endif
};

#define SX150X_EXP1_INT_N	PM8921_MPP_IRQ(PM8921_IRQ_BASE, 9)
#define SX150X_EXP2_INT_N	MSM_GPIO_TO_INT(81)

struct sx150x_platform_data mpq8064_sx150x_pdata[] = {
	[SX150X_EXP1] = {
		.gpio_base	= SX150X_EXP1_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= SX150X_EXP1_INT_N,
		.irq_base	= SX150X_EXP1_IRQ_BASE,
	},
	[SX150X_EXP2] = {
		.gpio_base	= SX150X_EXP2_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0f,
		.io_pulldn_ena	= 0x70,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= SX150X_EXP2_INT_N,
		.irq_base	= SX150X_EXP2_IRQ_BASE,
	},
	[SX150X_EXP3] = {
		.gpio_base	= SX150X_EXP3_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= -1,
	},
	[SX150X_EXP4] = {
		.gpio_base	= SX150X_EXP4_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= -1,
	},
};

static struct i2c_board_info sx150x_gpio_exp_info[] = {
	{
		I2C_BOARD_INFO("sx1509q", 0x70),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP1],
	},
	{
		I2C_BOARD_INFO("sx1508q", 0x23),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP2],
	},
	{
		I2C_BOARD_INFO("sx1508q", 0x22),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP3],
	},
	{
		I2C_BOARD_INFO("sx1509q", 0x3E),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP4],
	},
};

#define MPQ8064_I2C_GSBI5_BUS_ID	5

static struct i2c_registry mpq8064_i2c_devices[] __initdata = {
	{
		I2C_MPQ_CDP,
		MPQ8064_I2C_GSBI5_BUS_ID,
		sx150x_gpio_exp_info,
		ARRAY_SIZE(sx150x_gpio_exp_info),
	},
};

static void __init register_i2c_devices(void)
{
	u8 mach_mask = 0;
	int i;

#ifdef CONFIG_MSM_CAMERA
	struct i2c_registry apq8064_camera_i2c_devices = {
		I2C_SURF | I2C_FFA | I2C_LIQUID | I2C_RUMI,
		APQ_8064_GSBI4_QUP_I2C_BUS_ID,
		apq8064_camera_board_info.board_info,
		apq8064_camera_board_info.num_i2c_board_info,
	};
#endif
	/* Build the matching 'supported_machs' bitmask */
	if (machine_is_apq8064_cdp())
		mach_mask = I2C_SURF;
	else if (machine_is_apq8064_mtp())
		mach_mask = I2C_FFA;
	else if (machine_is_apq8064_liquid())
		mach_mask = I2C_LIQUID;
	else if (machine_is_apq8064_rumi3())
		mach_mask = I2C_RUMI;
	else if (machine_is_apq8064_sim())
		mach_mask = I2C_SIM;
	else if (PLATFORM_IS_MPQ8064())
		mach_mask = I2C_MPQ_CDP;
	else
		pr_err("unmatched machine ID in register_i2c_devices\n");

	/*[ECID:000000] ZTEBSP zhangzhao start  20120321, 8x25 bring up  */
	//#ifdef CONFIG_TOUCHSCREEN_VIRTUAL_KEYS
	touch_sysfs_init();
       //#endif

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(apq8064_i2c_devices); ++i) {
		if (apq8064_i2c_devices[i].machs & mach_mask)
			i2c_register_board_info(apq8064_i2c_devices[i].bus,
						apq8064_i2c_devices[i].info,
						apq8064_i2c_devices[i].len);
	}
#ifdef CONFIG_MSM_CAMERA
	if (apq8064_camera_i2c_devices.machs & mach_mask)
		i2c_register_board_info(apq8064_camera_i2c_devices.bus,
			apq8064_camera_i2c_devices.info,
			apq8064_camera_i2c_devices.len);
#endif

	for (i = 0; i < ARRAY_SIZE(mpq8064_i2c_devices); ++i) {
		if (mpq8064_i2c_devices[i].machs & mach_mask)
			i2c_register_board_info(
					mpq8064_i2c_devices[i].bus,
					mpq8064_i2c_devices[i].info,
					mpq8064_i2c_devices[i].len);
	}
/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20120929 start*/
#if defined(CONFIG_I2C_SI470X) || defined(CONFIG_I2C_SI470X_MODULE)
	si470x_init();
#endif
/*[ECID:000000] ZTEBSP jiaobaocun, for externel FM, 20120929 end*/
#ifdef CONFIG_PN544_NFC
	nfc_init();
#endif
}

static void enable_avc_i2c_bus(void)
{
	int avc_i2c_en_mpp = PM8921_MPP_PM_TO_SYS(8);
	int rc;

	rc = gpio_request(avc_i2c_en_mpp, "avc_i2c_en");
	if (rc)
		pr_err("request for avc_i2c_en mpp failed,"
						 "rc=%d\n", rc);
	else
		gpio_set_value_cansleep(avc_i2c_en_mpp, 1);
}

/* Modify platform data values to match requirements for PM8917. */
/*static void __init apq8064_pm8917_pdata_fixup(void)
{
	cdp_keys_data.buttons = cdp_keys_pm8917;
	cdp_keys_data.nbuttons = ARRAY_SIZE(cdp_keys_pm8917);
}*/
/*
#ifdef CONFIG_SERIAL_MSM_HS
static struct msm_serial_hs_platform_data apq8064_uartdm_gsbi4_pdata = {
	.config_gpio	= 4,
	.uart_tx_gpio	= 10,
	.uart_rx_gpio	= 11,
	.uart_cts_gpio	= 12,
	.uart_rfr_gpio	= 13,
};
#else
static struct msm_serial_hs_platform_data apq8064_uartdm_gsbi4_pdata;
#endif
*/
static void __init apq8064ab_update_retention_spm(void)
{
	int i;

	/* Update the SPM sequences for krait retention on all cores */
	for (i = 0; i < ARRAY_SIZE(msm_spm_data); i++) {
		int j;
		struct msm_spm_platform_data *pdata = &msm_spm_data[i];
		for (j = 0; j < pdata->num_modes; j++) {
			if (pdata->modes[j].cmd ==
					spm_retention_cmd_sequence)
				pdata->modes[j].cmd =
				spm_retention_with_krait_v3_cmd_sequence;
		}
	}
}

static void __init apq8064_common_init(void)
{
	u32 platform_version = socinfo_get_platform_version();

	/*if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
		apq8064_pm8917_pdata_fixup();*/
	platform_device_register(&msm_gpio_device);
	if (cpu_is_apq8064ab())
		apq8064ab_update_krait_spm();
	if (cpu_is_krait_v3()) {
		msm_pm_set_tz_retention_flag(0);
		apq8064ab_update_retention_spm();
	} else {
		msm_pm_set_tz_retention_flag(1);
	}
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_spm_l2_init(msm_spm_l2_data);
	msm_tsens_early_init(&apq_tsens_pdata);
	msm_thermal_init(&msm_thermal_pdata);
	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
	BUG_ON(msm_rpm_init(&apq8064_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(&msm_rpmrs_data));
	regulator_suppress_info_printing();
	//if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
	//	configure_apq8064_pm8917_power_grid();
	platform_device_register(&apq8064_device_rpm_regulator);
	//if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
	//	platform_device_register(&apq8064_pm8921_device_rpm_regulator);
	if (msm_xo_init())
		pr_err("Failed to initialize XO votes\n");
	msm_clock_init(&apq8064_clock_init_data);
	apq8064_init_gpiomux();
    sensor_power_init(1);    //added by fanjiankang
	apq8064_i2c_init();

	//added by fanjiankang for morata change
	#if defined(CONFIG_PROJECT_P864A20) ||defined(CONFIG_PROJECT_P864G02)
    morata_gpio_config();
  #endif

	//added by fanjiankang for taos gpio config
#if defined(CONFIG_TAOS_ALS) || defined(CONFIG_TAOS_27723)
	taos_gpio_config();
      #endif
	register_i2c_devices();

/*[ECID:000000] ZTEBSP shihuiqin, for video_lms501, 20120213 start*/
#ifndef SPI_SW_CONTROL   /*SPI_HW_CONTROL*/
	apq8064_device_qup_spi_gsbi5.dev.platform_data =
						&apq8064_qup_spi_gsbi5_pdata;
#endif

	apq8064_init_pmic();
	if (machine_is_apq8064_liquid())
		msm_otg_pdata.mhl_enable = true;

	android_usb_pdata.swfi_latency =
		msm_rpmrs_levels[0].latency_us;

	apq8064_device_otg.dev.platform_data = &msm_otg_pdata;
	apq8064_ehci_host_init();
	apq8064_init_buses();

	platform_add_devices(early_common_devices,
				ARRAY_SIZE(early_common_devices));
	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
		platform_add_devices(pm8921_common_devices,
					ARRAY_SIZE(pm8921_common_devices));
	else
		platform_add_devices(pm8917_common_devices,
					ARRAY_SIZE(pm8917_common_devices));

	if (!machine_is_apq8064_mtp())
		platform_device_register(&apq8064_device_ext_ts_sw_vreg);

	platform_add_devices(common_devices, ARRAY_SIZE(common_devices));
	if (!(machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
			machine_is_mpq8064_dtv())) {
		platform_add_devices(common_not_mpq_devices,
			ARRAY_SIZE(common_not_mpq_devices));

		/* Add GSBI4 I2C Device for non-fusion3 platform */
		if (socinfo_get_platform_subtype() !=
					PLATFORM_SUBTYPE_SGLTE2) {
			platform_device_register(&apq8064_device_qup_i2c_gsbi4);
		}
	}

	msm_hsic_pdata.swfi_latency =
		msm_rpmrs_levels[0].latency_us;
	if (machine_is_apq8064_mtp()) {
		msm_hsic_pdata.log2_irq_thresh = 5,
		apq8064_device_hsic_host.dev.platform_data = &msm_hsic_pdata;
		device_initialize(&apq8064_device_hsic_host.dev);
		if (socinfo_get_platform_subtype() == PLATFORM_SUBTYPE_DSDA2) {
			apq8064_device_ehci_host3.dev.platform_data =
				&msm_ehci_host_pdata3;
			device_initialize(&apq8064_device_ehci_host3.dev);
		}
	}
	apq8064_pm8xxx_gpio_mpp_init();
	apq8064_init_mmc();

	if (machine_is_apq8064_mtp()) {
		if (socinfo_get_platform_subtype() == PLATFORM_SUBTYPE_DSDA2) {
			amdm_8064_device.dev.platform_data =
				&amdm_platform_data;
			platform_device_register(&amdm_8064_device);
			bmdm_8064_device.dev.platform_data =
				&bmdm_platform_data;
			platform_device_register(&bmdm_8064_device);
		} else if (socinfo_get_platform_subtype() ==
				   PLATFORM_SUBTYPE_SGLTE2) {
			sglte_mdm_8064_device.dev.platform_data =
				&sglte2_mdm_platform_data;
			platform_device_register(&sglte_mdm_8064_device);
			sglte2_qsc_8064_device.dev.platform_data =
				&sglte2_qsc_platform_data;
			platform_device_register(&sglte2_qsc_8064_device);

			/* GSBI4 UART device for Primay IPC */
			//apq8064_uartdm_gsbi4_pdata.wakeup_irq = gpio_to_irq(11);
			//apq8064_device_uartdm_gsbi4.dev.platform_data =
			//			&apq8064_uartdm_gsbi4_pdata;
			//platform_device_register(&apq8064_device_uartdm_gsbi4);
		} else if (SOCINFO_VERSION_MINOR(platform_version) == 1) {
			i2s_mdm_8064_device.dev.platform_data =
				&mdm_platform_data;
			platform_device_register(&i2s_mdm_8064_device);
		} else {
			mdm_8064_device.dev.platform_data = &mdm_platform_data;
			platform_device_register(&mdm_8064_device);
		}
	}
	platform_device_register(&apq8064_slim_ctrl);
	slim_register_board_info(apq8064_slim_devices,
		ARRAY_SIZE(apq8064_slim_devices));
	if (!PLATFORM_IS_MPQ8064()) {
		apq8064_init_dsps();
		platform_device_register(&msm_8960_riva);
	}
	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));
	apq8064_epm_adc_init();
}

static void __init apq8064_allocate_memory_regions(void)
{
	apq8064_allocate_fb_region();
}

static void __init apq8064_sim_init(void)
{
	struct msm_watchdog_pdata *wdog_pdata = (struct msm_watchdog_pdata *)
		&msm8064_device_watchdog.dev.platform_data;

	wdog_pdata->bark_time = 15000;
	apq8064_common_init();
	platform_add_devices(sim_devices, ARRAY_SIZE(sim_devices));
}

static void __init apq8064_rumi3_init(void)
{
	apq8064_common_init();
	ethernet_init();
	msm_rotator_set_split_iommu_domain();
	platform_add_devices(rumi3_devices, ARRAY_SIZE(rumi3_devices));
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
}

static void __init apq8064_cdp_init(void)
{
	if (meminfo_init(SYS_MEMORY, SZ_256M) < 0)
		pr_err("meminfo_init() failed!\n");
	if (machine_is_apq8064_mtp() &&
		SOCINFO_VERSION_MINOR(socinfo_get_platform_version()) == 1)
			cyttsp_pdata.sleep_gpio = CYTTSP_TS_GPIO_SLEEP_ALT;
	apq8064_common_init();
	if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
		machine_is_mpq8064_dtv()) {
		enable_avc_i2c_bus();
		msm_rotator_set_split_iommu_domain();
		platform_add_devices(mpq_devices, ARRAY_SIZE(mpq_devices));
		mpq8064_pcie_init();
	} else {
		ethernet_init();
		msm_rotator_set_split_iommu_domain();
		platform_add_devices(cdp_devices, ARRAY_SIZE(cdp_devices));
		spi_register_board_info(spi_board_info,
						ARRAY_SIZE(spi_board_info));
	}
	apq8064_init_fb();
	apq8064_init_gpu();
	platform_add_devices(apq8064_footswitch, apq8064_num_footswitch);
#ifdef CONFIG_MSM_CAMERA
	apq8064_init_cam();
#endif
/*
	if (machine_is_mpq8064_hrd() || machine_is_mpq8064_dtv()) {
#ifdef CONFIG_SERIAL_MSM_HS
		mpq8064_gsbi6_uartdm_pdata.wakeup_irq = gpio_to_irq(15);
		mpq8064_device_uartdm_gsbi6.dev.platform_data =
					&mpq8064_gsbi6_uartdm_pdata;
#endif
		platform_device_register(&mpq8064_device_uartdm_gsbi6);
	}
*/
	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid())
		platform_device_register(&cdp_kp_pdev);

	if (machine_is_apq8064_mtp())
		platform_device_register(&mtp_kp_pdev);

	if (machine_is_mpq8064_cdp()) {
		platform_device_register(&mpq_gpio_keys_pdev);
		platform_device_register(&mpq_keypad_device);
	}
}

MACHINE_START(APQ8064_SIM, "QCT APQ8064 SIMULATOR")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_sim_init,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_RUMI3, "QCT APQ8064 RUMI3")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_rumi3_init,
	.init_early = apq8064_allocate_memory_regions,
	.restart = msm_restart,
MACHINE_END


MACHINE_START(APQ8064_CDP, "QCT APQ8064 CDP")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_MTP, "QCT APQ8064 MTP")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_LIQUID, "QCT APQ8064 LIQUID")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(MPQ8064_CDP, "QCT MPQ8064 CDP")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(MPQ8064_HRD, "QCT MPQ8064 HRD")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(MPQ8064_DTV, "QCT MPQ8064 DTV")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

