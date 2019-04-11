/*
 * Copyright (C) 2006 Ben Dooks <ben@simtec.co.uk>
 * Copyright (C) 2006 Simtec Electronics
 *
 * KS8695 - UART register and bit definitions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef KS8695_UART_H
#define KS8695_UART_H

#define KS8695_UART_OFFSET	(0xF0000 + 0xE000)
#define KS8695_UART_VA		(KS8695_IO_VA + KS8695_UART_OFFSET)
#define KS8695_UART_PA		(KS8695_IO_PA + KS8695_UART_OFFSET)

#endif
