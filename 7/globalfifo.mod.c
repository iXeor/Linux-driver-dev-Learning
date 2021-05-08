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
	{ 0xa90c928a, "param_ops_int" },
	{ 0x2e60bace, "memcpy" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xf09c7f68, "__wake_up" },
	{ 0xb89e62ec, "remove_wait_queue" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x4292364c, "schedule" },
	{ 0xda5661a3, "add_wait_queue" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0x671edcd4, "current_task" },
	{ 0x57b09822, "up" },
	{ 0x1a925a66, "down" },
	{ 0x50eedeb8, "printk" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xe174aa7, "__init_waitqueue_head" },
	{ 0xf1b0eaee, "cdev_add" },
	{ 0x897f86ab, "cdev_init" },
	{ 0x4634a332, "kmem_cache_alloc_trace" },
	{ 0x793808e8, "kmalloc_caches" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x37a0cba, "kfree" },
	{ 0x5db556b8, "cdev_del" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "6CA7F47331ED3FB35890AF0");
