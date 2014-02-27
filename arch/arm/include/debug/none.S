		.macro	addruart, rp, rv, tmp
		ldr	\rp, =0
		ldr	\rv, =0
		.endm

		.macro	senduart,rd,rx
		.endm

		.macro	busyuart,rd,rx
		.endm

		.macro	waituart,rd,rx
		.endm
