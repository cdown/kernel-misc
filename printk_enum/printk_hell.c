#include <stdio.h>

/* printk base */
#define printk printf
#define KERN_SOH "\001"
#define KERN_ERR KERN_SOH "3"

/* printk wrapping macros from printk.h and deps */
#define P_CONCAT_INTERNAL(a, b) a##b
#define __UNIQUE_ID(prefix) P_CONCAT_INTERNAL(prefix, __COUNTER__)
#define EXPAND(...) __VA_ARGS__

#define printk_store_fmt(fmt, ...)                                             \
	({                                                                     \
		static const char __UNIQUE_ID(__pr_)[]                         \
			__attribute__((section("printk_fmts"), unused)) = fmt; \
		printk(fmt, ##__VA_ARGS__);                                    \
	})

#define pr_err(fmt, ...)                                                       \
	do {                                                                   \
		if (__builtin_constant_p(fmt))                                 \
			printk_store_fmt(KERN_ERR EXPAND(pr_fmt(fmt)),         \
					 ##__VA_ARGS__);                       \
		else                                                           \
			printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__);           \
	} while (0)

/* translation unit specific */
#define pr_fmt(fmt) "debug_vm_pgtable: [%-25s]: " fmt, __func__

int main(int argc, char *argv[])
{
	pr_err("macros for world peace\n");
}
