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

int bars;
void __iomem *hw_addr;
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

	err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (err)
	{
		printk("%s, %d\n", "error", __LINE__);
		return err;
	}

	return 0;
}

static void e1000_remove(struct pci_dev *pdev)
{
	printk("%s, %d\n", __FUNCTION__, __LINE__);
	pci_release_selected_regions(pdev, bars);
	iounmap(hw_addr);
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
