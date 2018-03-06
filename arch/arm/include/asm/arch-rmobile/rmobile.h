#ifndef __ASM_ARCH_RMOBILE_H
#define __ASM_ARCH_RMOBILE_H

#if defined(CONFIG_RMOBILE)
#if defined(CONFIG_SH73A0)
#include <asm/arch/sh73a0.h>
#elif defined(CONFIG_R8A7740)
#include <asm/arch/r8a7740.h>
#elif defined(CONFIG_R8A7790)
#include <asm/arch/r8a7790.h>
#elif defined(CONFIG_R8A7791)
#include <asm/arch/r8a7791.h>
#elif defined(CONFIG_R8A7793)
#include <asm/arch/r8a7793.h>
#elif defined(CONFIG_IWG22M)
#include <asm/arch/iwg22m.h>
#elif defined(CONFIG_R8A7794)
#include <asm/arch/r8a7794.h>
#elif defined(CONFIG_R8A7742)
#include <asm/arch/r8a7742.h>
#elif defined(CONFIG_IWG21M)
#include <asm/arch/iwg21m.h>
#elif defined(CONFIG_R8A7743)
#include <asm/arch/r8a7743.h>
#elif defined(CONFIG_R8A7745)
#include <asm/arch/r8a7745.h>
#elif defined(CONFIG_R8A7747X)
#include <asm/arch/r8a7747x.h>
#else
#error "SOC Name not defined"
#endif

#ifdef __KERNEL__

#ifndef __ASSEMBLY__

u32 rmobile_get_cpu_type(void);
u32 rmobile_get_cpu_rev_integer(void);
u32 rmobile_get_cpu_rev_fraction(void);

#endif /* __ASSEMBLY__ */

#endif /* __KERNEL__ */

#endif /* CONFIG_RMOBILE */

#endif /* __ASM_ARCH_RMOBILE_H */
