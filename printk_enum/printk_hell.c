#include <stdio.h>

/* printk base */
#define printk printf
#define KERN_SOH "\001"
#define KERN_ERR KERN_SOH "3"

/* printk wrapping macros from printk.h and deps */
#define P_CONCAT_INTERNAL(a, b) a##b
#define __UNIQUE_ID(prefix) P_CONCAT_INTERNAL(prefix, __COUNTER__)

#define _printk_store_fmt(var, fmt, ...)                                       \
	({                                                                     \
		static const char var[]                                        \
			__attribute__((section("printk_fmts"))) = fmt;         \
		printk(var, ##__VA_ARGS__);                                    \
	})

#define printk_store_fmt(fmt, ...)                                             \
	_printk_store_fmt(__UNIQUE_ID(__pr_), fmt, ##__VA_ARGS__)

#define pr_err(fmt, ...)                                                       \
	do {                                                                   \
		if (__builtin_constant_p(fmt))                                 \
			printk_store_fmt(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__); \
		else                                                           \
			printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__);           \
	} while (0)

/* *NOT* changeable. set externally to printk */
#define pr_fmt(fmt) "debug_vm_pgtable: [%-25s]: " fmt, __func__
//#define pr_fmt(fmt) "debug_vm_pgtable: " fmt

int main(int argc, char *argv[])
{
	pr_err("macros for world peace\n");
	pr_err("macros for world peace: %d\n", 123);
}
