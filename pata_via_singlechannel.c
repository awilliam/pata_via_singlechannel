/*
 * kprobe to trap pata_via registration and replace id_table with one
 * that specifies single channel operation.  This avoids registering
 * an unused second channel and introducing an unused shared interrupt
 * with the ethernet controller on IRQ 15 on an HP t5500 thin client.
 *
 * Copyright 2013 Red Hat, Inc
 *
 * Author: Alex Williamson <alex.williamson@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/pci.h>

enum {
	VIA_IDFLAG_SINGLE = (1 << 0), /* single channel controller) */
};

static const struct pci_device_id via[] = {
	{ PCI_VDEVICE(VIA, 0x0571), VIA_IDFLAG_SINGLE},
	{ },
};

static int kp_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct pci_driver *drv;

#ifdef CONFIG_X86_64
	drv = (struct pci_driver *)regs->di;
#else
	drv = (struct pci_driver *)regs->ax;
#endif

	if (!strcmp(drv->name, "pata_via"))
		drv->id_table = via;

	return 0;
}

static struct kprobe kp = {
	.symbol_name = "__pci_register_driver",
	.pre_handler = kp_pre_handler,
};

int __init pata_via_singlechannel_init(void)
{
	int ret = register_kprobe(&kp);
	if (ret < 0)
		printk("Failed register_kprobe %d\n", ret);

	return ret;
}

void __exit pata_via_singlechannel_exit(void)
{
	unregister_kprobe(&kp);
}

module_init(pata_via_singlechannel_init);
module_exit(pata_via_singlechannel_exit);

MODULE_AUTHOR("Alex Williamson <alex.williamson@redhat.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.1");
