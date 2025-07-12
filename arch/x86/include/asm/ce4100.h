/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_CE4100_H_
#define _ASM_CE4100_H_

int ce4100_pci_init(void);

#if IS_BUILTIN(CONFIG_SERIAL_8250) && IS_ENABLED(CONFIG_SERIAL_8250_ISA)
void __init sdv_serial_fixup(void);
#else
static inline void sdv_serial_fixup(void) {};
#endif

#endif
