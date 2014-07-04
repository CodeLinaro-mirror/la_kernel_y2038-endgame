pm-s5pv210-$(CONFIG_PM_SLEEP)	+= s5pv210-pm.o s5pv210-sleep.o
obj-$(CONFIG_CPU_S5PV210)	+= s5pv210.o $(pm-s5pv210-y)

ifdef CONFIG_ARCH_EXYNOS
obj-y				+= exynos.o pmu.o exynos-smc.o firmware.o s5p-dev-mfc.o
obj-$(CONFIG_PM_SLEEP)		+= suspend.o
obj-$(CONFIG_EXYNOS_CPU_SUSPEND) += pm.o sleep.o
obj-$(CONFIG_SMP)		+= platsmp.o headsmp.o
obj-$(CONFIG_PM_GENERIC_DOMAINS) += pm_domains.o
obj-$(CONFIG_EXYNOS5420_MCPM)	+= mcpm-exynos.o

plus_sec := $(call as-instr,.arch_extension sec,+sec)
AFLAGS_exynos-smc.o		:=-Wa,-march=armv7-a$(plus_sec)
AFLAGS_sleep.o			:=-Wa,-march=armv7-a$(plus_sec)
CFLAGS_mcpm-exynos.o		+= -march=armv7-a
endif

obj-$(CONFIG_S5P_PM_CHECK)	+= pm-check.o
obj-$(CONFIG_PM_SLEEP)          += pm-common.o
obj-$(CONFIG_S5P_PM_DEBUG)	+= pm-debug.o
