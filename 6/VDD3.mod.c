#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xf448062f, "module_layout" },
	{ 0xf1b0eaee, "cdev_add" },
	{ 0x897f86ab, "cdev_init" },
	{ 0x2bc95bd4, "memset" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x86a4889a, "kmalloc_order_trace" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0x670c0597, "down_interruptible" },
	{ 0x57b09822, "up" },
	{ 0x50eedeb8, "printk" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x37a0cba, "kfree" },
	{ 0x5db556b8, "cdev_del" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "1712E98A7FD23BC21A55D9B");
