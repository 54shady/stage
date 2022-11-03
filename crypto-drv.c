#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/bitops.h>
#include <linux/pci.h>

/* drivers/net/ethernet/intel/e1000/e1000_main.c */

#define BAR_0 0
char e1000_driver_name[] = "mycrypto";
#define INTEL_E1000_ETHERNET_DEVICE(device_id) {\
	PCI_DEVICE(0x1111, device_id)}
static const struct pci_device_id e1000_pci_tbl[] = {
	INTEL_E1000_ETHERNET_DEVICE(0x2222),
	/* required last entry */
	{0,}
};

typedef enum tagIOField {
	ErrorCode = 0x00,
	State = 0x01,
	Command = 0x02,
	InterruptFlag = 0x03,
	DmaInAddress = 0x04,
	DmaInPagesCount = 0x08,
	DmaInSizeInBytes = 0x0c,
	DmaOutAddress = 0x10,
	DmaOutPagesCount = 0x14,
	DmaOutSizeInBytes = 0x18,
	MsiErrorFlag = 0x1c,
	MsiReadyFlag = 0x1d,
	MsiResetFlag = 0x1e,
	Unused = 0x1f,
} IoField;

#define NO2STR(n) case n: return #n
static const char *iofield2str(IoField io)
{
    switch (io)
	{
		NO2STR(ErrorCode);
		NO2STR(State);
		NO2STR(Command);
		NO2STR(InterruptFlag);
		NO2STR(DmaInAddress);
		NO2STR(DmaInPagesCount);
		NO2STR(DmaInSizeInBytes);
		NO2STR(DmaOutAddress);
		NO2STR(DmaOutPagesCount);
		NO2STR(DmaOutSizeInBytes);
		NO2STR(MsiErrorFlag);
		NO2STR(MsiReadyFlag);
		NO2STR(MsiResetFlag);
        default:
            return "UnknowIoFiled";
    }
}

int bit = 0; /* 0: byte, 1: word, 2: long */
int regIdx = 0;
int val = 0;
int bars;
void __iomem *hw_addr;

/* cat /sys/devices/pci0000:00/0000:00:03.0/mycrypto_debug */
static ssize_t mycrypto_debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	switch (bit)
	{
		case 0:
		default:
			printk("%s = 0x%08x\n", iofield2str(regIdx), readb(hw_addr + regIdx));
			break;
		case 1:
			printk("%s = 0x%08x\n", iofield2str(regIdx), readw(hw_addr + regIdx));
			break;
		case 2:
			printk("%s = 0x%08x\n", iofield2str(regIdx), readl(hw_addr + regIdx));
			break;
	}

    return 0;
}

/*
 * Enable interrupt flag to 2
 * devmem 0xfebf1003 b 2
 *
 * echo 0 3 2 > /sys/devices/pci0000\:00/0000\:00\:03.0/mycrypto_debug
 */
static ssize_t mycrypto_debug_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%d %d %d", &bit, &regIdx, &val);
	printk("1:write %d bit to 0x%x %d\n", bit, regIdx, val & 0xf);

	switch (bit)
	{
		case 0:
		default:
			/* byte write */
			writeb(val & 0xf, hw_addr + regIdx);
			break;
		case 1:
			/* word write */
			writew(val & 0xff, hw_addr + regIdx);
			break;
		case 2:
			/* long write */
			writel(val, hw_addr + regIdx);
			break;
	}

    return count;
}

static DEVICE_ATTR(mycrypto_debug, S_IRUGO | S_IWUSR, mycrypto_debug_show, mycrypto_debug_store);

static struct attribute *mycrypto_attrs[] = {
    &dev_attr_mycrypto_debug.attr,
    NULL,
};

static const struct attribute_group mycrypto_attr_group = {
    .attrs = mycrypto_attrs,
};

static int e1000_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int err;

	bars = pci_select_bars(pdev, IORESOURCE_MEM);
	printk("bars = %d, %s, %d\n", bars, __FUNCTION__, __LINE__);
	err = pci_enable_device_mem(pdev);
	if (err)
	{
		printk("%s, %d\n", "Error*****", __LINE__);
		return err;
	}

	err = pci_request_selected_regions(pdev, bars, e1000_driver_name);
	if (err)
	{
		printk("%s, %d\n", "error", __LINE__);
		return err;
	}

	pci_set_master(pdev);
	err = pci_save_state(pdev);
	if (err)
	{
		printk("%s, %d\n", "error", __LINE__);
		return err;
	}

	hw_addr = pci_ioremap_bar(pdev, BAR_0);
	if (!hw_addr)
	{
		printk("%s, %d\n", "error", __LINE__);
		return err;
	}
	printk("hw_addr = 0x%08x\n", hw_addr);
	printk("%08x\n", readb(hw_addr + ErrorCode));
	printk("%08x\n", readb(hw_addr + State));
	printk("%08x\n", readb(hw_addr + Command));
	printk("%08x\n", readb(hw_addr + InterruptFlag));

	err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (err)
	{
		printk("%s, %d\n", "error", __LINE__);
		return err;
	}

	/* sysfs for debug */
    err = sysfs_create_group(&pdev->dev.kobj, &mycrypto_attr_group);
    if (err) {
        printk("failed to create sysfs device attributes\n");
        return -1;
    }

	return 0;
}

static void e1000_remove(struct pci_dev *pdev)
{
	printk("%s, %d\n", __FUNCTION__, __LINE__);
	pci_release_selected_regions(pdev, bars);
	iounmap(hw_addr);

	sysfs_remove_group(&pdev->dev.kobj, &mycrypto_attr_group);
}

static void e1000_shutdown(struct pci_dev *pdev)
{
	printk("%s, %d\n", __FUNCTION__, __LINE__);
}

static struct pci_driver e1000_driver = {
	.name     = e1000_driver_name,
	.id_table = e1000_pci_tbl,
	.probe    = e1000_probe,
	.remove   = e1000_remove,
	.shutdown = e1000_shutdown,
};

static int __init e1000_init_module(void)
{
	int ret;

	ret = pci_register_driver(&e1000_driver);
	return ret;
}
module_init(e1000_init_module);

static void __exit e1000_exit_module(void)
{
	pci_unregister_driver(&e1000_driver);
}
module_exit(e1000_exit_module);
MODULE_LICENSE("GPL");
