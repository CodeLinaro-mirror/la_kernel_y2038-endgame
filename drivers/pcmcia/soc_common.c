/*======================================================================

    Common support code for the PCMCIA control functionality of
    integrated SOCs like the SA-11x0 and PXA2xx microprocessors.

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.

    The initial developer of the original code is John G. Dorsey
    <john+@cs.cmu.edu>.  Portions created by John G. Dorsey are
    Copyright (C) 1999 John G. Dorsey.  All Rights Reserved.

    Alternatively, the contents of this file may be used under the
    terms of the GNU Public License version 2 (the "GPL"), in which
    case the provisions of the GPL are applicable instead of the
    above.  If you wish to allow the use of your version of this file
    only under the terms of the GPL and not to allow others to use
    your version of this file under the MPL, indicate your decision
    by deleting the provisions above and replace them with the notice
    and other provisions required by the GPL.  If you do not delete
    the provisions above, a recipient may use your version of this
    file under either the MPL or the GPL.

======================================================================*/

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/soc/pxa/cpu.h>
#include <linux/soc/pxa/smemc.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/cpufreq.h>

/* include the world */
#include <pcmcia/cistpl.h>
#include <pcmcia/ss.h>

#include <asm/hardware/scoop.h>
#include <asm/mach-types.h>

#ifdef CONFIG_ARCH_SA1100
#include <mach/hardware.h>
#include <mach/h3xxx.h>
#include <asm/hardware/sa1111.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#endif

struct soc_pcmcia_regulator {
	struct regulator	*reg;
	bool			on;
};

struct pcmcia_state {
  unsigned detect: 1,
            ready: 1,
             bvd1: 1,
             bvd2: 1,
           wrprot: 1,
            vs_3v: 1,
            vs_Xv: 1;
};

/*
 * This structure encapsulates per-socket state which we might need to
 * use when responding to a Card Services query of some kind.
 */
struct soc_pcmcia_socket {
	struct pcmcia_socket	socket;

	/*
	 * Info from low level handler
	 */
	unsigned int		nr;
	struct clk		*clk;

	/*
	 * Core PCMCIA state
	 */
	const struct pcmcia_low_level *ops;

	unsigned int		status;
	socket_state_t		cs_state;

	unsigned short		spd_io[MAX_IO_WIN];
	unsigned short		spd_mem[MAX_WIN];
	unsigned short		spd_attr[MAX_WIN];

	struct resource		res_skt;
	struct resource		res_io;
	struct resource		res_io_io;
	struct resource		res_mem;
	struct resource		res_attr;

	struct {
		int		gpio;
		struct gpio_desc *desc;
		unsigned int	irq;
		const char	*name;
	} stat[6];
#define SOC_STAT_CD		0	/* Card detect */
#define SOC_STAT_BVD1		1	/* BATDEAD / IOSTSCHG */
#define SOC_STAT_BVD2		2	/* BATWARN / IOSPKR */
#define SOC_STAT_RDY		3	/* Ready / Interrupt */
#define SOC_STAT_VS1		4	/* Voltage sense 1 */
#define SOC_STAT_VS2		5	/* Voltage sense 2 */

	struct gpio_desc	*gpio_reset;
	struct gpio_desc	*gpio_bus_enable;
	struct soc_pcmcia_regulator vcc;
	struct soc_pcmcia_regulator vpp;

	unsigned int		irq_state;

#ifdef CONFIG_CPU_FREQ
	struct notifier_block	cpufreq_nb;
#endif
	struct timer_list	poll_timer;
	struct list_head	node;
	void *driver_data;
};


struct pcmcia_low_level {
	struct module *owner;

	/* first socket in system */
	int first;
	/* nr of sockets */
	int nr;

	int (*hw_init)(struct soc_pcmcia_socket *);
	void (*hw_shutdown)(struct soc_pcmcia_socket *);

	void (*socket_state)(struct soc_pcmcia_socket *, struct pcmcia_state *);
	int (*configure_socket)(struct soc_pcmcia_socket *, const socket_state_t *);

	/*
	 * Enable card status IRQs on (re-)initialisation.  This can
	 * be called at initialisation, power management event, or
	 * pcmcia event.
	 */
	void (*socket_init)(struct soc_pcmcia_socket *);

	/*
	 * Disable card status IRQs and PCMCIA bus on suspend.
	 */
	void (*socket_suspend)(struct soc_pcmcia_socket *);

	/*
	 * Hardware specific timing routines.
	 * If provided, the get_timing routine overrides the SOC default.
	 */
	unsigned int (*get_timing)(struct soc_pcmcia_socket *, unsigned int, unsigned int);
	int (*set_timing)(struct soc_pcmcia_socket *);
	int (*show_timing)(struct soc_pcmcia_socket *, char *);

#ifdef CONFIG_CPU_FREQ
	/*
	 * CPUFREQ support.
	 */
	int (*frequency_change)(struct soc_pcmcia_socket *, unsigned long, struct cpufreq_freqs *);
#endif
};

struct skt_dev_info {
	int nskt;
	struct soc_pcmcia_socket skt[];
};

struct soc_pcmcia_timing {
	unsigned short io;
	unsigned short mem;
	unsigned short attr;
};

static const char *skt_names[] = {
	"PCMCIA socket 0",
	"PCMCIA socket 1",
};

/*
 * The PC Card Standard, Release 7, section 4.13.4, says that twIORD
 * has a minimum value of 165ns. Section 4.13.5 says that twIOWR has
 * a minimum value of 165ns, as well. Section 4.7.2 (describing
 * common and attribute memory write timing) says that twWE has a
 * minimum value of 150ns for a 250ns cycle time (for 5V operation;
 * see section 4.7.4), or 300ns for a 600ns cycle time (for 3.3V
 * operation, also section 4.7.4). Section 4.7.3 says that taOE
 * has a maximum value of 150ns for a 300ns cycle time (for 5V
 * operation), or 300ns for a 600ns cycle time (for 3.3V operation).
 *
 * When configuring memory maps, Card Services appears to adopt the policy
 * that a memory access time of "0" means "use the default." The default
 * PCMCIA I/O command width time is 165ns. The default PCMCIA 5V attribute
 * and memory command width time is 150ns; the PCMCIA 3.3V attribute and
 * memory command width time is 300ns.
 */
#define SOC_PCMCIA_IO_ACCESS		(165)
#define SOC_PCMCIA_5V_MEM_ACCESS	(150)
#define SOC_PCMCIA_3V_MEM_ACCESS	(300)
#define SOC_PCMCIA_ATTR_MEM_ACCESS	(300)

/*
 * The socket driver actually works nicely in interrupt-driven form,
 * so the (relatively infrequent) polling is "just to be sure."
 */
#define SOC_PCMCIA_POLL_PERIOD    (2*HZ)


/* I/O pins replacing memory pins
 * (PCMCIA System Architecture, 2nd ed., by Don Anderson, p.75)
 *
 * These signals change meaning when going from memory-only to
 * memory-or-I/O interface:
 */
#define iostschg bvd1
#define iospkr   bvd2


static irqreturn_t soc_common_pcmcia_interrupt(int irq, void *dev);

#ifdef CONFIG_PCMCIA_DEBUG
#define debug(skt, lvl, fmt, arg...) \
	soc_pcmcia_debug(skt, __func__, lvl, fmt , ## arg)


static int pc_debug;
module_param(pc_debug, int, 0644);

void soc_pcmcia_debug(struct soc_pcmcia_socket *skt, const char *func,
		      int lvl, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	if (pc_debug > lvl) {
		va_start(args, fmt);

		vaf.fmt = fmt;
		vaf.va = &args;

		printk(KERN_DEBUG "skt%u: %s: %pV", skt->nr, func, &vaf);

		va_end(args);
	}
}

#else
#define debug(skt, lvl, fmt, arg...) do { } while (0)

#endif

#define to_soc_pcmcia_socket(x)	\
	container_of(x, struct soc_pcmcia_socket, socket)

int soc_pcmcia_regulator_set(struct soc_pcmcia_socket *skt,
	struct soc_pcmcia_regulator *r, int v)
{
	bool on;
	int ret;

	if (!r->reg)
		return 0;

	on = v != 0;
	if (r->on == on)
		return 0;

	if (on) {
		ret = regulator_set_voltage(r->reg, v * 100000, v * 100000);
		if (ret) {
			int vout = regulator_get_voltage(r->reg) / 100000;

			dev_warn(&skt->socket.dev,
				 "CS requested %s=%u.%uV, applying %u.%uV\n",
				 r == &skt->vcc ? "Vcc" : "Vpp",
				 v / 10, v % 10, vout / 10, vout % 10);
		}

		ret = regulator_enable(r->reg);
	} else {
		ret = regulator_disable(r->reg);
	}
	if (ret == 0)
		r->on = on;

	return ret;
}

static unsigned short
calc_speed(unsigned short *spds, int num, unsigned short dflt)
{
	unsigned short speed = 0;
	int i;

	for (i = 0; i < num; i++)
		if (speed < spds[i])
			speed = spds[i];
	if (speed == 0)
		speed = dflt;

	return speed;
}

void soc_common_pcmcia_get_timing(struct soc_pcmcia_socket *skt,
	struct soc_pcmcia_timing *timing)
{
	timing->io =
		calc_speed(skt->spd_io, MAX_IO_WIN, SOC_PCMCIA_IO_ACCESS);
	timing->mem =
		calc_speed(skt->spd_mem, MAX_WIN, SOC_PCMCIA_3V_MEM_ACCESS);
	timing->attr =
		calc_speed(skt->spd_attr, MAX_WIN, SOC_PCMCIA_3V_MEM_ACCESS);
}

static void __soc_pcmcia_hw_shutdown(struct soc_pcmcia_socket *skt,
	unsigned int nr)
{
	unsigned int i;

	for (i = 0; i < nr; i++)
		if (skt->stat[i].irq)
			free_irq(skt->stat[i].irq, skt);

	if (skt->ops->hw_shutdown)
		skt->ops->hw_shutdown(skt);

	clk_disable_unprepare(skt->clk);
}

static void soc_pcmcia_hw_shutdown(struct soc_pcmcia_socket *skt)
{
	__soc_pcmcia_hw_shutdown(skt, ARRAY_SIZE(skt->stat));
}

int soc_pcmcia_request_gpiods(struct soc_pcmcia_socket *skt)
{
	struct device *dev = skt->socket.dev.parent;
	struct gpio_desc *desc;
	int i;

	for (i = 0; i < ARRAY_SIZE(skt->stat); i++) {
		if (!skt->stat[i].name)
			continue;

		desc = devm_gpiod_get(dev, skt->stat[i].name, GPIOD_IN);
		if (IS_ERR(desc)) {
			dev_err(dev, "Failed to get GPIO for %s: %ld\n",
				skt->stat[i].name, PTR_ERR(desc));
			return PTR_ERR(desc);
		}

		skt->stat[i].desc = desc;
	}

	return 0;
}

static int soc_pcmcia_hw_init(struct soc_pcmcia_socket *skt)
{
	int ret = 0, i;

	ret = clk_prepare_enable(skt->clk);
	if (ret)
		return ret;

	if (skt->ops->hw_init) {
		ret = skt->ops->hw_init(skt);
		if (ret) {
			clk_disable_unprepare(skt->clk);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(skt->stat); i++) {
		if (gpio_is_valid(skt->stat[i].gpio)) {
			unsigned long flags = GPIOF_IN;

			/* CD is active low by default */
			if (i == SOC_STAT_CD)
				flags |= GPIOF_ACTIVE_LOW;

			ret = devm_gpio_request_one(skt->socket.dev.parent,
						    skt->stat[i].gpio, flags,
						    skt->stat[i].name);
			if (ret) {
				__soc_pcmcia_hw_shutdown(skt, i);
				return ret;
			}

			skt->stat[i].desc = gpio_to_desc(skt->stat[i].gpio);
		}

		if (i < SOC_STAT_VS1 && skt->stat[i].desc) {
			int irq = gpiod_to_irq(skt->stat[i].desc);

			if (irq > 0) {
				if (i == SOC_STAT_RDY)
					skt->socket.pci_irq = irq;
				else
					skt->stat[i].irq = irq;
			}
		}

		if (skt->stat[i].irq) {
			ret = request_irq(skt->stat[i].irq,
					  soc_common_pcmcia_interrupt,
					  IRQF_TRIGGER_NONE,
					  skt->stat[i].name, skt);
			if (ret) {
				__soc_pcmcia_hw_shutdown(skt, i);
				return ret;
			}
		}
	}

	return ret;
}

static void soc_pcmcia_hw_enable(struct soc_pcmcia_socket *skt)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(skt->stat); i++)
		if (skt->stat[i].irq) {
			irq_set_irq_type(skt->stat[i].irq, IRQ_TYPE_EDGE_RISING);
			irq_set_irq_type(skt->stat[i].irq, IRQ_TYPE_EDGE_BOTH);
		}
}

static void soc_pcmcia_hw_disable(struct soc_pcmcia_socket *skt)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(skt->stat); i++)
		if (skt->stat[i].irq)
			irq_set_irq_type(skt->stat[i].irq, IRQ_TYPE_NONE);
}

/*
 * The CF 3.0 specification says that cards tie VS1 to ground and leave
 * VS2 open.  Many implementations do not wire up the VS signals, so we
 * provide hard-coded values as per the CF 3.0 spec.
 */
void soc_common_cf_socket_state(struct soc_pcmcia_socket *skt,
	struct pcmcia_state *state)
{
	state->vs_3v = 1;
}

static unsigned int soc_common_pcmcia_skt_state(struct soc_pcmcia_socket *skt)
{
	struct pcmcia_state state;
	unsigned int stat;

	memset(&state, 0, sizeof(struct pcmcia_state));

	/* Make battery voltage state report 'good' */
	state.bvd1 = 1;
	state.bvd2 = 1;

	if (skt->stat[SOC_STAT_CD].desc)
		state.detect = !!gpiod_get_value(skt->stat[SOC_STAT_CD].desc);
	if (skt->stat[SOC_STAT_RDY].desc)
		state.ready = !!gpiod_get_value(skt->stat[SOC_STAT_RDY].desc);
	if (skt->stat[SOC_STAT_BVD1].desc)
		state.bvd1 = !!gpiod_get_value(skt->stat[SOC_STAT_BVD1].desc);
	if (skt->stat[SOC_STAT_BVD2].desc)
		state.bvd2 = !!gpiod_get_value(skt->stat[SOC_STAT_BVD2].desc);
	if (skt->stat[SOC_STAT_VS1].desc)
		state.vs_3v = !!gpiod_get_value(skt->stat[SOC_STAT_VS1].desc);
	if (skt->stat[SOC_STAT_VS2].desc)
		state.vs_Xv = !!gpiod_get_value(skt->stat[SOC_STAT_VS2].desc);

	skt->ops->socket_state(skt, &state);

	stat = state.detect  ? SS_DETECT : 0;
	stat |= state.ready  ? SS_READY  : 0;
	stat |= state.wrprot ? SS_WRPROT : 0;
	stat |= state.vs_3v  ? SS_3VCARD : 0;
	stat |= state.vs_Xv  ? SS_XVCARD : 0;

	/* The power status of individual sockets is not available
	 * explicitly from the hardware, so we just remember the state
	 * and regurgitate it upon request:
	 */
	stat |= skt->cs_state.Vcc ? SS_POWERON : 0;

	if (skt->cs_state.flags & SS_IOCARD)
		stat |= state.bvd1 ? 0 : SS_STSCHG;
	else {
		if (state.bvd1 == 0)
			stat |= SS_BATDEAD;
		else if (state.bvd2 == 0)
			stat |= SS_BATWARN;
	}
	return stat;
}

/*
 * soc_common_pcmcia_config_skt
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * Convert PCMCIA socket state to our socket configure structure.
 */
static int soc_common_pcmcia_config_skt(
	struct soc_pcmcia_socket *skt, socket_state_t *state)
{
	int ret;

	ret = skt->ops->configure_socket(skt, state);
	if (ret < 0) {
		pr_err("soc_common_pcmcia: unable to configure socket %d\n",
		       skt->nr);
		/* restore the previous state */
		WARN_ON(skt->ops->configure_socket(skt, &skt->cs_state));
		return ret;
	}

	if (ret == 0) {
		struct gpio_desc *descs[2];
		DECLARE_BITMAP(values, 2);
		int n = 0;

		if (skt->gpio_reset) {
			descs[n] = skt->gpio_reset;
			__assign_bit(n++, values, state->flags & SS_RESET);
		}
		if (skt->gpio_bus_enable) {
			descs[n] = skt->gpio_bus_enable;
			__assign_bit(n++, values, state->flags & SS_OUTPUT_ENA);
		}

		if (n)
			gpiod_set_array_value_cansleep(n, descs, NULL, values);

		/*
		 * This really needs a better solution.  The IRQ
		 * may or may not be claimed by the driver.
		 */
		if (skt->irq_state != 1 && state->io_irq) {
			skt->irq_state = 1;
			irq_set_irq_type(skt->socket.pci_irq,
					 IRQ_TYPE_EDGE_FALLING);
		} else if (skt->irq_state == 1 && state->io_irq == 0) {
			skt->irq_state = 0;
			irq_set_irq_type(skt->socket.pci_irq, IRQ_TYPE_NONE);
		}

		skt->cs_state = *state;
	}

	return ret;
}

/* soc_common_pcmcia_sock_init()
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * (Re-)Initialise the socket, turning on status interrupts
 * and PCMCIA bus.  This must wait for power to stabilise
 * so that the card status signals report correctly.
 *
 * Returns: 0
 */
static int soc_common_pcmcia_sock_init(struct pcmcia_socket *sock)
{
	struct soc_pcmcia_socket *skt = to_soc_pcmcia_socket(sock);

	debug(skt, 2, "initializing socket\n");
	if (skt->ops->socket_init)
		skt->ops->socket_init(skt);
	soc_pcmcia_hw_enable(skt);
	return 0;
}


/*
 * soc_common_pcmcia_suspend()
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * Remove power on the socket, disable IRQs from the card.
 * Turn off status interrupts, and disable the PCMCIA bus.
 *
 * Returns: 0
 */
static int soc_common_pcmcia_suspend(struct pcmcia_socket *sock)
{
	struct soc_pcmcia_socket *skt = to_soc_pcmcia_socket(sock);

	debug(skt, 2, "suspending socket\n");

	soc_pcmcia_hw_disable(skt);
	if (skt->ops->socket_suspend)
		skt->ops->socket_suspend(skt);

	return 0;
}

static DEFINE_SPINLOCK(status_lock);

static void soc_common_check_status(struct soc_pcmcia_socket *skt)
{
	unsigned int events;

	debug(skt, 4, "entering PCMCIA monitoring thread\n");

	do {
		unsigned int status;
		unsigned long flags;

		status = soc_common_pcmcia_skt_state(skt);

		spin_lock_irqsave(&status_lock, flags);
		events = (status ^ skt->status) & skt->cs_state.csc_mask;
		skt->status = status;
		spin_unlock_irqrestore(&status_lock, flags);

		debug(skt, 4, "events: %s%s%s%s%s%s\n",
			events == 0         ? "<NONE>"   : "",
			events & SS_DETECT  ? "DETECT "  : "",
			events & SS_READY   ? "READY "   : "",
			events & SS_BATDEAD ? "BATDEAD " : "",
			events & SS_BATWARN ? "BATWARN " : "",
			events & SS_STSCHG  ? "STSCHG "  : "");

		if (events)
			pcmcia_parse_events(&skt->socket, events);
	} while (events);
}

/* Let's poll for events in addition to IRQs since IRQ only is unreliable... */
static void soc_common_pcmcia_poll_event(struct timer_list *t)
{
	struct soc_pcmcia_socket *skt = from_timer(skt, t, poll_timer);
	debug(skt, 4, "polling for events\n");

	mod_timer(&skt->poll_timer, jiffies + SOC_PCMCIA_POLL_PERIOD);

	soc_common_check_status(skt);
}


/*
 * Service routine for socket driver interrupts (requested by the
 * low-level PCMCIA init() operation via soc_common_pcmcia_thread()).
 * The actual interrupt-servicing work is performed by
 * soc_common_pcmcia_thread(), largely because the Card Services event-
 * handling code performs scheduling operations which cannot be
 * executed from within an interrupt context.
 */
static irqreturn_t soc_common_pcmcia_interrupt(int irq, void *dev)
{
	struct soc_pcmcia_socket *skt = dev;

	debug(skt, 3, "servicing IRQ %d\n", irq);

	soc_common_check_status(skt);

	return IRQ_HANDLED;
}


/*
 *  Implements the get_status() operation for the in-kernel PCMCIA
 * service (formerly SS_GetStatus in Card Services). Essentially just
 * fills in bits in `status' according to internal driver state or
 * the value of the voltage detect chipselect register.
 *
 * As a debugging note, during card startup, the PCMCIA core issues
 * three set_socket() commands in a row the first with RESET deasserted,
 * the second with RESET asserted, and the last with RESET deasserted
 * again. Following the third set_socket(), a get_status() command will
 * be issued. The kernel is looking for the SS_READY flag (see
 * setup_socket(), reset_socket(), and unreset_socket() in cs.c).
 *
 * Returns: 0
 */
static int
soc_common_pcmcia_get_status(struct pcmcia_socket *sock, unsigned int *status)
{
	struct soc_pcmcia_socket *skt = to_soc_pcmcia_socket(sock);

	skt->status = soc_common_pcmcia_skt_state(skt);
	*status = skt->status;

	return 0;
}


/*
 * Implements the set_socket() operation for the in-kernel PCMCIA
 * service (formerly SS_SetSocket in Card Services). We more or
 * less punt all of this work and let the kernel handle the details
 * of power configuration, reset, &c. We also record the value of
 * `state' in order to regurgitate it to the PCMCIA core later.
 */
static int soc_common_pcmcia_set_socket(
	struct pcmcia_socket *sock, socket_state_t *state)
{
	struct soc_pcmcia_socket *skt = to_soc_pcmcia_socket(sock);

	debug(skt, 2, "mask: %s%s%s%s%s%s flags: %s%s%s%s%s%s Vcc %d Vpp %d irq %d\n",
			(state->csc_mask == 0)		? "<NONE> " :	"",
			(state->csc_mask & SS_DETECT)	? "DETECT " :	"",
			(state->csc_mask & SS_READY)	? "READY " :	"",
			(state->csc_mask & SS_BATDEAD)	? "BATDEAD " :	"",
			(state->csc_mask & SS_BATWARN)	? "BATWARN " :	"",
			(state->csc_mask & SS_STSCHG)	? "STSCHG " :	"",
			(state->flags == 0)		? "<NONE> " :	"",
			(state->flags & SS_PWR_AUTO)	? "PWR_AUTO " :	"",
			(state->flags & SS_IOCARD)	? "IOCARD " :	"",
			(state->flags & SS_RESET)	? "RESET " :	"",
			(state->flags & SS_SPKR_ENA)	? "SPKR_ENA " :	"",
			(state->flags & SS_OUTPUT_ENA)	? "OUTPUT_ENA " : "",
			state->Vcc, state->Vpp, state->io_irq);

	return soc_common_pcmcia_config_skt(skt, state);
}


/*
 * Implements the set_io_map() operation for the in-kernel PCMCIA
 * service (formerly SS_SetIOMap in Card Services). We configure
 * the map speed as requested, but override the address ranges
 * supplied by Card Services.
 *
 * Returns: 0 on success, -1 on error
 */
static int soc_common_pcmcia_set_io_map(
	struct pcmcia_socket *sock, struct pccard_io_map *map)
{
	struct soc_pcmcia_socket *skt = to_soc_pcmcia_socket(sock);
	unsigned short speed = map->speed;

	debug(skt, 2, "map %u  speed %u start 0x%08llx stop 0x%08llx\n",
		map->map, map->speed, (unsigned long long)map->start,
		(unsigned long long)map->stop);
	debug(skt, 2, "flags: %s%s%s%s%s%s%s%s\n",
		(map->flags == 0)		? "<NONE>"	: "",
		(map->flags & MAP_ACTIVE)	? "ACTIVE "	: "",
		(map->flags & MAP_16BIT)	? "16BIT "	: "",
		(map->flags & MAP_AUTOSZ)	? "AUTOSZ "	: "",
		(map->flags & MAP_0WS)		? "0WS "	: "",
		(map->flags & MAP_WRPROT)	? "WRPROT "	: "",
		(map->flags & MAP_USE_WAIT)	? "USE_WAIT "	: "",
		(map->flags & MAP_PREFETCH)	? "PREFETCH "	: "");

	if (map->map >= MAX_IO_WIN) {
		printk(KERN_ERR "%s(): map (%d) out of range\n", __func__,
		       map->map);
		return -1;
	}

	if (map->flags & MAP_ACTIVE) {
		if (speed == 0)
			speed = SOC_PCMCIA_IO_ACCESS;
	} else {
		speed = 0;
	}

	skt->spd_io[map->map] = speed;
	skt->ops->set_timing(skt);

	if (map->stop == 1)
		map->stop = PAGE_SIZE-1;

	map->stop -= map->start;
	map->stop += skt->socket.io_offset;
	map->start = skt->socket.io_offset;

	return 0;
}


/*
 * Implements the set_mem_map() operation for the in-kernel PCMCIA
 * service (formerly SS_SetMemMap in Card Services). We configure
 * the map speed as requested, but override the address ranges
 * supplied by Card Services.
 *
 * Returns: 0 on success, -ERRNO on error
 */
static int soc_common_pcmcia_set_mem_map(
	struct pcmcia_socket *sock, struct pccard_mem_map *map)
{
	struct soc_pcmcia_socket *skt = to_soc_pcmcia_socket(sock);
	struct resource *res;
	unsigned short speed = map->speed;

	debug(skt, 2, "map %u speed %u card_start %08x\n",
		map->map, map->speed, map->card_start);
	debug(skt, 2, "flags: %s%s%s%s%s%s%s%s\n",
		(map->flags == 0)		? "<NONE>"	: "",
		(map->flags & MAP_ACTIVE)	? "ACTIVE "	: "",
		(map->flags & MAP_16BIT)	? "16BIT "	: "",
		(map->flags & MAP_AUTOSZ)	? "AUTOSZ "	: "",
		(map->flags & MAP_0WS)		? "0WS "	: "",
		(map->flags & MAP_WRPROT)	? "WRPROT "	: "",
		(map->flags & MAP_ATTRIB)	? "ATTRIB "	: "",
		(map->flags & MAP_USE_WAIT)	? "USE_WAIT "	: "");

	if (map->map >= MAX_WIN)
		return -EINVAL;

	if (map->flags & MAP_ACTIVE) {
		if (speed == 0)
			speed = 300;
	} else {
		speed = 0;
	}

	if (map->flags & MAP_ATTRIB) {
		res = &skt->res_attr;
		skt->spd_attr[map->map] = speed;
		skt->spd_mem[map->map] = 0;
	} else {
		res = &skt->res_mem;
		skt->spd_attr[map->map] = 0;
		skt->spd_mem[map->map] = speed;
	}

	skt->ops->set_timing(skt);

	map->static_start = res->start + map->card_start;

	return 0;
}

struct bittbl {
	unsigned int mask;
	const char *name;
};

static struct bittbl status_bits[] = {
	{ SS_WRPROT,		"SS_WRPROT"	},
	{ SS_BATDEAD,		"SS_BATDEAD"	},
	{ SS_BATWARN,		"SS_BATWARN"	},
	{ SS_READY,		"SS_READY"	},
	{ SS_DETECT,		"SS_DETECT"	},
	{ SS_POWERON,		"SS_POWERON"	},
	{ SS_STSCHG,		"SS_STSCHG"	},
	{ SS_3VCARD,		"SS_3VCARD"	},
	{ SS_XVCARD,		"SS_XVCARD"	},
};

static struct bittbl conf_bits[] = {
	{ SS_PWR_AUTO,		"SS_PWR_AUTO"	},
	{ SS_IOCARD,		"SS_IOCARD"	},
	{ SS_RESET,		"SS_RESET"	},
	{ SS_DMA_MODE,		"SS_DMA_MODE"	},
	{ SS_SPKR_ENA,		"SS_SPKR_ENA"	},
	{ SS_OUTPUT_ENA,	"SS_OUTPUT_ENA"	},
};

static void dump_bits(char **p, const char *prefix,
	unsigned int val, struct bittbl *bits, int sz)
{
	char *b = *p;
	int i;

	b += sprintf(b, "%-9s:", prefix);
	for (i = 0; i < sz; i++)
		if (val & bits[i].mask)
			b += sprintf(b, " %s", bits[i].name);
	*b++ = '\n';
	*p = b;
}

/*
 * Implements the /sys/class/pcmcia_socket/??/status file.
 *
 * Returns: the number of characters added to the buffer
 */
static ssize_t show_status(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	struct soc_pcmcia_socket *skt =
		container_of(dev, struct soc_pcmcia_socket, socket.dev);
	char *p = buf;

	p += sprintf(p, "slot     : %d\n", skt->nr);

	dump_bits(&p, "status", skt->status,
		  status_bits, ARRAY_SIZE(status_bits));
	dump_bits(&p, "csc_mask", skt->cs_state.csc_mask,
		  status_bits, ARRAY_SIZE(status_bits));
	dump_bits(&p, "cs_flags", skt->cs_state.flags,
		  conf_bits, ARRAY_SIZE(conf_bits));

	p += sprintf(p, "Vcc      : %d\n", skt->cs_state.Vcc);
	p += sprintf(p, "Vpp      : %d\n", skt->cs_state.Vpp);
	p += sprintf(p, "IRQ      : %d (%d)\n", skt->cs_state.io_irq,
		skt->socket.pci_irq);
	if (skt->ops->show_timing)
		p += skt->ops->show_timing(skt, p);

	return p-buf;
}
static DEVICE_ATTR(status, S_IRUGO, show_status, NULL);


static struct pccard_operations soc_common_pcmcia_operations = {
	.init			= soc_common_pcmcia_sock_init,
	.suspend		= soc_common_pcmcia_suspend,
	.get_status		= soc_common_pcmcia_get_status,
	.set_socket		= soc_common_pcmcia_set_socket,
	.set_io_map		= soc_common_pcmcia_set_io_map,
	.set_mem_map		= soc_common_pcmcia_set_mem_map,
};


#ifdef CONFIG_CPU_FREQ
static int soc_common_pcmcia_cpufreq_nb(struct notifier_block *nb,
	unsigned long val, void *data)
{
	struct soc_pcmcia_socket *skt = container_of(nb, struct soc_pcmcia_socket, cpufreq_nb);
	struct cpufreq_freqs *freqs = data;

	return skt->ops->frequency_change(skt, val, freqs);
}
#endif

void soc_pcmcia_init_one(struct soc_pcmcia_socket *skt,
	const struct pcmcia_low_level *ops, struct device *dev)
{
	int i;

	skt->ops = ops;
	skt->socket.owner = ops->owner;
	skt->socket.dev.parent = dev;
	skt->socket.pci_irq = NO_IRQ;

	for (i = 0; i < ARRAY_SIZE(skt->stat); i++)
		skt->stat[i].gpio = -EINVAL;
}

void soc_pcmcia_remove_one(struct soc_pcmcia_socket *skt)
{
	del_timer_sync(&skt->poll_timer);

	pcmcia_unregister_socket(&skt->socket);

#ifdef CONFIG_CPU_FREQ
	if (skt->ops->frequency_change)
		cpufreq_unregister_notifier(&skt->cpufreq_nb,
					    CPUFREQ_TRANSITION_NOTIFIER);
#endif

	soc_pcmcia_hw_shutdown(skt);

	/* should not be required; violates some lowlevel drivers */
	soc_common_pcmcia_config_skt(skt, &dead_socket);

	iounmap(PCI_IOBASE + skt->res_io_io.start);
	release_resource(&skt->res_attr);
	release_resource(&skt->res_mem);
	release_resource(&skt->res_io);
	release_resource(&skt->res_skt);
}

int soc_pcmcia_add_one(struct soc_pcmcia_socket *skt)
{
	int ret;

	skt->cs_state = dead_socket;

	timer_setup(&skt->poll_timer, soc_common_pcmcia_poll_event, 0);
	skt->poll_timer.expires = jiffies + SOC_PCMCIA_POLL_PERIOD;

	ret = request_resource(&iomem_resource, &skt->res_skt);
	if (ret)
		goto out_err_1;

	ret = request_resource(&skt->res_skt, &skt->res_io);
	if (ret)
		goto out_err_2;

	ret = request_resource(&skt->res_skt, &skt->res_mem);
	if (ret)
		goto out_err_3;

	ret = request_resource(&skt->res_skt, &skt->res_attr);
	if (ret)
		goto out_err_4;

	skt->res_io_io = (struct resource)
		 DEFINE_RES_IO_NAMED(skt->nr * 0x1000 + 0x10000, 0x1000,
				     "PCMCIA I/O");
	ret = pci_remap_iospace(&skt->res_io_io, skt->res_io.start);
	if (ret)
		goto out_err_5;

	/*
	 * We initialize default socket timing here, because
	 * we are not guaranteed to see a SetIOMap operation at
	 * runtime.
	 */
	skt->ops->set_timing(skt);

	ret = soc_pcmcia_hw_init(skt);
	if (ret)
		goto out_err_6;

	skt->socket.ops = &soc_common_pcmcia_operations;
	skt->socket.features = SS_CAP_STATIC_MAP|SS_CAP_PCCARD;
	skt->socket.resource_ops = &pccard_static_ops;
	skt->socket.irq_mask = 0;
	skt->socket.map_size = PAGE_SIZE;
	skt->socket.io_offset = (unsigned long)skt->res_io_io.start;

	skt->status = soc_common_pcmcia_skt_state(skt);

#ifdef CONFIG_CPU_FREQ
	if (skt->ops->frequency_change) {
		skt->cpufreq_nb.notifier_call = soc_common_pcmcia_cpufreq_nb;

		ret = cpufreq_register_notifier(&skt->cpufreq_nb,
						CPUFREQ_TRANSITION_NOTIFIER);
		if (ret < 0)
			dev_err(skt->socket.dev.parent,
				"unable to register CPU frequency change notifier for PCMCIA (%d)\n",
				ret);
	}
#endif

	ret = pcmcia_register_socket(&skt->socket);
	if (ret)
		goto out_err_7;

	ret = device_create_file(&skt->socket.dev, &dev_attr_status);
	if (ret)
		goto out_err_8;

	return ret;

 out_err_8:
	del_timer_sync(&skt->poll_timer);
	pcmcia_unregister_socket(&skt->socket);

 out_err_7:
	soc_pcmcia_hw_shutdown(skt);
 out_err_6:
	iounmap(PCI_IOBASE + skt->res_io_io.start);
 out_err_5:
	release_resource(&skt->res_attr);
 out_err_4:
	release_resource(&skt->res_mem);
 out_err_3:
	release_resource(&skt->res_io);
 out_err_2:
	release_resource(&skt->res_skt);
 out_err_1:

	return ret;
}

#define	NO_KEEP_VS 0x0001
#define SCOOP_DEV platform_scoop_config->devs

static void sharpsl_pcmcia_init_reset(struct soc_pcmcia_socket *skt)
{
	struct scoop_pcmcia_dev *scoopdev = &SCOOP_DEV[skt->nr];

	reset_scoop(scoopdev->dev);

	/* Shared power controls need to be handled carefully */
	if (platform_scoop_config->power_ctrl)
		platform_scoop_config->power_ctrl(scoopdev->dev, 0x0000, skt->nr);
	else
		write_scoop_reg(scoopdev->dev, SCOOP_CPR, 0x0000);

	scoopdev->keep_vs = NO_KEEP_VS;
	scoopdev->keep_rd = 0;
}

static int sharpsl_pcmcia_hw_init(struct soc_pcmcia_socket *skt)
{
	if (SCOOP_DEV[skt->nr].cd_irq >= 0) {
		skt->stat[SOC_STAT_CD].irq = SCOOP_DEV[skt->nr].cd_irq;
		skt->stat[SOC_STAT_CD].name = SCOOP_DEV[skt->nr].cd_irq_str;
	}

	skt->socket.pci_irq = SCOOP_DEV[skt->nr].irq;

	return 0;
}

static void sharpsl_pcmcia_socket_state(struct soc_pcmcia_socket *skt,
				    struct pcmcia_state *state)
{
	unsigned short cpr, csr;
	struct device *scoop = SCOOP_DEV[skt->nr].dev;

	cpr = read_scoop_reg(SCOOP_DEV[skt->nr].dev, SCOOP_CPR);

	write_scoop_reg(scoop, SCOOP_IRM, 0x00FF);
	write_scoop_reg(scoop, SCOOP_ISR, 0x0000);
	write_scoop_reg(scoop, SCOOP_IRM, 0x0000);
	csr = read_scoop_reg(scoop, SCOOP_CSR);
	if (csr & 0x0004) {
		/* card eject */
		write_scoop_reg(scoop, SCOOP_CDR, 0x0000);
		SCOOP_DEV[skt->nr].keep_vs = NO_KEEP_VS;
	}
	else if (!(SCOOP_DEV[skt->nr].keep_vs & NO_KEEP_VS)) {
		/* keep vs1,vs2 */
		write_scoop_reg(scoop, SCOOP_CDR, 0x0000);
		csr |= SCOOP_DEV[skt->nr].keep_vs;
	}
	else if (cpr & 0x0003) {
		/* power on */
		write_scoop_reg(scoop, SCOOP_CDR, 0x0000);
		SCOOP_DEV[skt->nr].keep_vs = (csr & 0x00C0);
	}
	else {
		/* card detect */
	        if ((machine_is_spitz() || machine_is_borzoi()) && skt->nr == 1) {
	                write_scoop_reg(scoop, SCOOP_CDR, 0x0000);
	        } else {
		        write_scoop_reg(scoop, SCOOP_CDR, 0x0002);
	        }
	}

	state->detect = (csr & 0x0004) ? 0 : 1;
	state->ready  = (csr & 0x0002) ? 1 : 0;
	state->bvd1   = (csr & 0x0010) ? 1 : 0;
	state->bvd2   = (csr & 0x0020) ? 1 : 0;
	state->wrprot = (csr & 0x0008) ? 1 : 0;
	state->vs_3v  = (csr & 0x0040) ? 0 : 1;
	state->vs_Xv  = (csr & 0x0080) ? 0 : 1;

	if ((cpr & 0x0080) && ((cpr & 0x8040) != 0x8040)) {
		printk(KERN_ERR "sharpsl_pcmcia_socket_state(): CPR=%04X, Low voltage!\n", cpr);
	}
}


static int sharpsl_pcmcia_configure_socket(struct soc_pcmcia_socket *skt,
				       const socket_state_t *state)
{
	unsigned long flags;
	struct device *scoop = SCOOP_DEV[skt->nr].dev;

	unsigned short cpr, ncpr, ccr, nccr, mcr, nmcr, imr, nimr;

	switch (state->Vcc) {
	case	0:  	break;
	case 	33: 	break;
	case	50: 	break;
	default:
		 printk(KERN_ERR "sharpsl_pcmcia_configure_socket(): bad Vcc %u\n", state->Vcc);
		 return -1;
	}

	if ((state->Vpp!=state->Vcc) && (state->Vpp!=0)) {
		printk(KERN_ERR "CF slot cannot support Vpp %u\n", state->Vpp);
		return -1;
	}

	local_irq_save(flags);

	nmcr = (mcr = read_scoop_reg(scoop, SCOOP_MCR)) & ~0x0010;
	ncpr = (cpr = read_scoop_reg(scoop, SCOOP_CPR)) & ~0x0083;
	nccr = (ccr = read_scoop_reg(scoop, SCOOP_CCR)) & ~0x0080;
	nimr = (imr = read_scoop_reg(scoop, SCOOP_IMR)) & ~0x003E;

	if ((machine_is_spitz() || machine_is_borzoi() || machine_is_akita()) && skt->nr == 0) {
	        ncpr |= (state->Vcc == 33) ? 0x0002 :
		        (state->Vcc == 50) ? 0x0002 : 0;
	} else {
	        ncpr |= (state->Vcc == 33) ? 0x0001 :
		        (state->Vcc == 50) ? 0x0002 : 0;
	}
	nmcr |= (state->flags&SS_IOCARD) ? 0x0010 : 0;
	ncpr |= (state->flags&SS_OUTPUT_ENA) ? 0x0080 : 0;
	nccr |= (state->flags&SS_RESET)? 0x0080: 0;
	nimr |=	((skt->status&SS_DETECT) ? 0x0004 : 0)|
			((skt->status&SS_READY)  ? 0x0002 : 0)|
			((skt->status&SS_BATDEAD)? 0x0010 : 0)|
			((skt->status&SS_BATWARN)? 0x0020 : 0)|
			((skt->status&SS_STSCHG) ? 0x0010 : 0)|
			((skt->status&SS_WRPROT) ? 0x0008 : 0);

	if (!(ncpr & 0x0003)) {
		SCOOP_DEV[skt->nr].keep_rd = 0;
	} else if (!SCOOP_DEV[skt->nr].keep_rd) {
		if (nccr & 0x0080)
			SCOOP_DEV[skt->nr].keep_rd = 1;
		else
			nccr |= 0x0080;
	}

	if (mcr != nmcr)
		write_scoop_reg(scoop, SCOOP_MCR, nmcr);
	if (cpr != ncpr) {
		if (platform_scoop_config->power_ctrl)
			platform_scoop_config->power_ctrl(scoop, ncpr , skt->nr);
		else
		        write_scoop_reg(scoop, SCOOP_CPR, ncpr);
	}
	if (ccr != nccr)
		write_scoop_reg(scoop, SCOOP_CCR, nccr);
	if (imr != nimr)
		write_scoop_reg(scoop, SCOOP_IMR, nimr);

	local_irq_restore(flags);

	return 0;
}

static void sharpsl_pcmcia_socket_init(struct soc_pcmcia_socket *skt)
{
	sharpsl_pcmcia_init_reset(skt);

	/* Enable interrupt */
	write_scoop_reg(SCOOP_DEV[skt->nr].dev, SCOOP_IMR, 0x00C0);
	write_scoop_reg(SCOOP_DEV[skt->nr].dev, SCOOP_MCR, 0x0101);
	SCOOP_DEV[skt->nr].keep_vs = NO_KEEP_VS;
}

static void sharpsl_pcmcia_socket_suspend(struct soc_pcmcia_socket *skt)
{
	sharpsl_pcmcia_init_reset(skt);
}

static struct pcmcia_low_level sharpsl_pcmcia_ops = {
	.owner                  = THIS_MODULE,
	.hw_init                = sharpsl_pcmcia_hw_init,
	.socket_state           = sharpsl_pcmcia_socket_state,
	.configure_socket       = sharpsl_pcmcia_configure_socket,
	.socket_init            = sharpsl_pcmcia_socket_init,
	.socket_suspend         = sharpsl_pcmcia_socket_suspend,
	.first                  = 0,
	.nr                     = 0,
};

#ifdef CONFIG_PXA_SHARPSL
static struct platform_device *sharpsl_pcmcia_device;

static int __init sharpsl_pcmcia_init(void)
{
	int ret;

	if (!platform_scoop_config)
		return -ENODEV;

	sharpsl_pcmcia_ops.nr = platform_scoop_config->num_devs;
	sharpsl_pcmcia_device = platform_device_alloc("pxa2xx-pcmcia", -1);

	if (!sharpsl_pcmcia_device)
		return -ENOMEM;

	ret = platform_device_add_data(sharpsl_pcmcia_device,
			&sharpsl_pcmcia_ops, sizeof(sharpsl_pcmcia_ops));
	if (ret == 0) {
		sharpsl_pcmcia_device->dev.parent = platform_scoop_config->devs[0].dev;
		ret = platform_device_add(sharpsl_pcmcia_device);
	}

	if (ret)
		platform_device_put(sharpsl_pcmcia_device);

	return ret;
}

static void __exit sharpsl_pcmcia_exit(void)
{
	platform_device_unregister(sharpsl_pcmcia_device);
}

fs_initcall(sharpsl_pcmcia_init);
module_exit(sharpsl_pcmcia_exit);
#endif

/* SA-1100 PCMCIA Memory and I/O timing
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * The SA-1110 Developer's Manual, section 10.2.5, says the following:
 *
 *  "To calculate the recommended BS_xx value for each address space:
 *   divide the command width time (the greater of twIOWR and twIORD,
 *   or the greater of twWE and twOE) by processor cycle time; divide
 *   by 2; divide again by 3 (number of BCLK's per command assertion);
 *   round up to the next whole number; and subtract 1."
 */

/* MECR: Expansion Memory Configuration Register
 * (SA-1100 Developers Manual, p.10-13; SA-1110 Developers Manual, p.10-24)
 *
 * MECR layout is:
 *
 *   FAST1 BSM1<4:0> BSA1<4:0> BSIO1<4:0> FAST0 BSM0<4:0> BSA0<4:0> BSIO0<4:0>
 *
 * (This layout is actually true only for the SA-1110; the FASTn bits are
 * reserved on the SA-1100.)
 */

#define MECR_SOCKET_0_SHIFT (0)
#define MECR_SOCKET_1_SHIFT (16)

#define MECR_BS_MASK        (0x1f)
#define MECR_FAST_MODE_MASK (0x01)

#define MECR_BSIO_SHIFT (0)
#define MECR_BSA_SHIFT  (5)
#define MECR_BSM_SHIFT  (10)
#define MECR_FAST_SHIFT (15)

#define MECR_SET(mecr, sock, shift, mask, bs) \
((mecr)=((mecr)&~(((mask)<<(shift))<<\
                  ((sock)==0?MECR_SOCKET_0_SHIFT:MECR_SOCKET_1_SHIFT)))|\
        (((bs)<<(shift))<<((sock)==0?MECR_SOCKET_0_SHIFT:MECR_SOCKET_1_SHIFT)))

#define MECR_GET(mecr, sock, shift, mask) \
((((mecr)>>(((sock)==0)?MECR_SOCKET_0_SHIFT:MECR_SOCKET_1_SHIFT))>>\
 (shift))&(mask))

#define MECR_BSIO_SET(mecr, sock, bs) \
MECR_SET((mecr), (sock), MECR_BSIO_SHIFT, MECR_BS_MASK, (bs))

#define MECR_BSIO_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_BSIO_SHIFT, MECR_BS_MASK)

#define MECR_BSA_SET(mecr, sock, bs) \
MECR_SET((mecr), (sock), MECR_BSA_SHIFT, MECR_BS_MASK, (bs))

#define MECR_BSA_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_BSA_SHIFT, MECR_BS_MASK)

#define MECR_BSM_SET(mecr, sock, bs) \
MECR_SET((mecr), (sock), MECR_BSM_SHIFT, MECR_BS_MASK, (bs))

#define MECR_BSM_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_BSM_SHIFT, MECR_BS_MASK)

#define MECR_FAST_SET(mecr, sock, fast) \
MECR_SET((mecr), (sock), MECR_FAST_SHIFT, MECR_FAST_MODE_MASK, (fast))

#define MECR_FAST_GET(mecr, sock) \
MECR_GET((mecr), (sock), MECR_FAST_SHIFT, MECR_FAST_MODE_MASK)

/*
 * Personal Computer Memory Card International Association (PCMCIA) sockets
 */

#define PCMCIAPrtSp	0x04000000	/* PCMCIA Partition Space [byte]   */
#define PCMCIASp	(4*PCMCIAPrtSp)	/* PCMCIA Space [byte]             */
#define PCMCIAIOSp	PCMCIAPrtSp	/* PCMCIA I/O Space [byte]         */
#define PCMCIAAttrSp	PCMCIAPrtSp	/* PCMCIA Attribute Space [byte]   */
#define PCMCIAMemSp	PCMCIAPrtSp	/* PCMCIA Memory Space [byte]      */

#define PCMCIA0Sp	PCMCIASp	/* PCMCIA 0 Space [byte]           */
#define PCMCIA0IOSp	PCMCIAIOSp	/* PCMCIA 0 I/O Space [byte]       */
#define PCMCIA0AttrSp	PCMCIAAttrSp	/* PCMCIA 0 Attribute Space [byte] */
#define PCMCIA0MemSp	PCMCIAMemSp	/* PCMCIA 0 Memory Space [byte]    */

#define PCMCIA1Sp	PCMCIASp	/* PCMCIA 1 Space [byte]           */
#define PCMCIA1IOSp	PCMCIAIOSp	/* PCMCIA 1 I/O Space [byte]       */
#define PCMCIA1AttrSp	PCMCIAAttrSp	/* PCMCIA 1 Attribute Space [byte] */
#define PCMCIA1MemSp	PCMCIAMemSp	/* PCMCIA 1 Memory Space [byte]    */

#define _PCMCIA(Nb)			/* PCMCIA [0..1]                   */ \
			(0x20000000 + (Nb) * PCMCIASp)
#define _PCMCIAIO(Nb)	_PCMCIA(Nb)	/* PCMCIA I/O [0..1]               */
#define _PCMCIAAttr(Nb)			/* PCMCIA Attribute [0..1]         */ \
			(_PCMCIA(Nb) + 2 * PCMCIAPrtSp)
#define _PCMCIAMem(Nb)			/* PCMCIA Memory [0..1]            */ \
			(_PCMCIA(Nb) + 3 * PCMCIAPrtSp)

#define _PCMCIA0	_PCMCIA(0)	/* PCMCIA 0                        */
#define _PCMCIA0IO	_PCMCIAIO(0)	/* PCMCIA 0 I/O                    */
#define _PCMCIA0Attr	_PCMCIAAttr(0)	/* PCMCIA 0 Attribute              */
#define _PCMCIA0Mem	_PCMCIAMem(0)	/* PCMCIA 0 Memory                 */

#define _PCMCIA1	_PCMCIA(1)	/* PCMCIA 1                        */
#define _PCMCIA1IO	_PCMCIAIO(1)	/* PCMCIA 1 I/O                    */
#define _PCMCIA1Attr	_PCMCIAAttr(1)	/* PCMCIA 1 Attribute              */
#define _PCMCIA1Mem	_PCMCIAMem(1)	/* PCMCIA 1 Memory                 */

#define MCXX_SETUP_MASK     (0x7f)
#define MCXX_ASST_MASK      (0x1f)
#define MCXX_HOLD_MASK      (0x3f)
#define MCXX_SETUP_SHIFT    (0)
#define MCXX_ASST_SHIFT     (7)
#define MCXX_HOLD_SHIFT     (14)

#ifdef CONFIG_ARCH_SA1100
/* This function implements the BS value calculation for setting the MECR
 * using integer arithmetic:
 */
static inline unsigned int sa1100_pcmcia_mecr_bs(unsigned int pcmcia_cycle_ns,
						 unsigned int cpu_clock_khz){
  unsigned int t = ((pcmcia_cycle_ns * cpu_clock_khz) / 6) - 1000000;
  return (t / 1000000) + (((t % 1000000) == 0) ? 0 : 1);
}

/* This function returns the (approximate) command assertion period, in
 * nanoseconds, for a given CPU clock frequency and MECR BS value:
 */
static inline unsigned int sa1100_pcmcia_cmd_time(unsigned int cpu_clock_khz,
						  unsigned int pcmcia_mecr_bs){
  return (((10000000 * 2) / cpu_clock_khz) * (3 * (pcmcia_mecr_bs + 1))) / 10;
}


int sa11xx_drv_pcmcia_add_one(struct soc_pcmcia_socket *skt)
{
	skt->res_skt.start = _PCMCIA(skt->nr);
	skt->res_skt.end = _PCMCIA(skt->nr) + PCMCIASp - 1;
	skt->res_skt.name = skt_names[skt->nr];
	skt->res_skt.flags = IORESOURCE_MEM;

	skt->res_io.start = _PCMCIAIO(skt->nr);
	skt->res_io.end = _PCMCIAIO(skt->nr) + PCMCIAIOSp - 1;
	skt->res_io.name = "io";
	skt->res_io.flags = IORESOURCE_MEM | IORESOURCE_BUSY;

	skt->res_mem.start = _PCMCIAMem(skt->nr);
	skt->res_mem.end = _PCMCIAMem(skt->nr) + PCMCIAMemSp - 1;
	skt->res_mem.name = "memory";
	skt->res_mem.flags = IORESOURCE_MEM;

	skt->res_attr.start = _PCMCIAAttr(skt->nr);
	skt->res_attr.end = _PCMCIAAttr(skt->nr) + PCMCIAAttrSp - 1;
	skt->res_attr.name = "attribute";
	skt->res_attr.flags = IORESOURCE_MEM;

	return soc_pcmcia_add_one(skt);
}

/*
 * sa1100_pcmcia_default_mecr_timing
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * Calculate MECR clock wait states for given CPU clock
 * speed and command wait state. This function can be over-
 * written by a board specific version.
 *
 * The default is to simply calculate the BS values as specified in
 * the INTEL SA1100 development manual
 * "Expansion Memory (PCMCIA) Configuration Register (MECR)"
 * that's section 10.2.5 in _my_ version of the manual ;)
 */
static unsigned int
sa1100_pcmcia_default_mecr_timing(struct soc_pcmcia_socket *skt,
				  unsigned int cpu_speed,
				  unsigned int cmd_time)
{
	return sa1100_pcmcia_mecr_bs(cmd_time, cpu_speed);
}

/* sa1100_pcmcia_set_mecr()
 * ^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * set MECR value for socket <sock> based on this sockets
 * io, mem and attribute space access speed.
 * Call board specific BS value calculation to allow boards
 * to tweak the BS values.
 */
static int
sa1100_pcmcia_set_mecr(struct soc_pcmcia_socket *skt, unsigned int cpu_clock)
{
	struct soc_pcmcia_timing timing;
	u32 mecr, old_mecr;
	unsigned long flags;
	unsigned int bs_io, bs_mem, bs_attr;

	soc_common_pcmcia_get_timing(skt, &timing);

	bs_io = skt->ops->get_timing(skt, cpu_clock, timing.io);
	bs_mem = skt->ops->get_timing(skt, cpu_clock, timing.mem);
	bs_attr = skt->ops->get_timing(skt, cpu_clock, timing.attr);

	local_irq_save(flags);

	old_mecr = mecr = MECR;
	MECR_FAST_SET(mecr, skt->nr, 0);
	MECR_BSIO_SET(mecr, skt->nr, bs_io);
	MECR_BSA_SET(mecr, skt->nr, bs_attr);
	MECR_BSM_SET(mecr, skt->nr, bs_mem);
	if (old_mecr != mecr)
		MECR = mecr;

	local_irq_restore(flags);

	debug(skt, 2, "FAST %X  BSM %X  BSA %X  BSIO %X\n",
	      MECR_FAST_GET(mecr, skt->nr),
	      MECR_BSM_GET(mecr, skt->nr), MECR_BSA_GET(mecr, skt->nr),
	      MECR_BSIO_GET(mecr, skt->nr));

	return 0;
}

#ifdef CONFIG_CPU_FREQ
static int
sa1100_pcmcia_frequency_change(struct soc_pcmcia_socket *skt,
			       unsigned long val,
			       struct cpufreq_freqs *freqs)
{
	switch (val) {
	case CPUFREQ_PRECHANGE:
		if (freqs->new > freqs->old)
			sa1100_pcmcia_set_mecr(skt, freqs->new);
		break;

	case CPUFREQ_POSTCHANGE:
		if (freqs->new < freqs->old)
			sa1100_pcmcia_set_mecr(skt, freqs->new);
		break;
	}

	return 0;
}

#endif

static int
sa1100_pcmcia_set_timing(struct soc_pcmcia_socket *skt)
{
	unsigned long clk = clk_get_rate(skt->clk);

	return sa1100_pcmcia_set_mecr(skt, clk / 1000);
}

static int
sa1100_pcmcia_show_timing(struct soc_pcmcia_socket *skt, char *buf)
{
	struct soc_pcmcia_timing timing;
	unsigned int clock = clk_get_rate(skt->clk) / 1000;
	unsigned long mecr = MECR;
	char *p = buf;

	soc_common_pcmcia_get_timing(skt, &timing);

	p+=sprintf(p, "I/O      : %uns (%uns)\n", timing.io,
		   sa1100_pcmcia_cmd_time(clock, MECR_BSIO_GET(mecr, skt->nr)));

	p+=sprintf(p, "attribute: %uns (%uns)\n", timing.attr,
		   sa1100_pcmcia_cmd_time(clock, MECR_BSA_GET(mecr, skt->nr)));

	p+=sprintf(p, "common   : %uns (%uns)\n", timing.mem,
		   sa1100_pcmcia_cmd_time(clock, MECR_BSM_GET(mecr, skt->nr)));

	return p - buf;
}

void sa11xx_drv_pcmcia_ops(struct pcmcia_low_level *ops)
{
	/*
	 * set default MECR calculation if the board specific
	 * code did not specify one...
	 */
	if (!ops->get_timing)
		ops->get_timing = sa1100_pcmcia_default_mecr_timing;

	/* Provide our SA11x0 specific timing routines. */
	ops->set_timing  = sa1100_pcmcia_set_timing;
	ops->show_timing = sa1100_pcmcia_show_timing;
#ifdef CONFIG_CPU_FREQ
	ops->frequency_change = sa1100_pcmcia_frequency_change;
#endif
}

#define SKT_DEV_INFO_SIZE(n) \
	(sizeof(struct skt_dev_info) + (n)*sizeof(struct soc_pcmcia_socket))

int sa11xx_drv_pcmcia_probe(struct device *dev, struct pcmcia_low_level *ops,
			    int first, int nr)
{
	struct skt_dev_info *sinfo;
	struct soc_pcmcia_socket *skt;
	int i, ret = 0;
	struct clk *clk;

	clk = devm_clk_get(dev, NULL);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	sa11xx_drv_pcmcia_ops(ops);

	sinfo = devm_kzalloc(dev, SKT_DEV_INFO_SIZE(nr), GFP_KERNEL);
	if (!sinfo)
		return -ENOMEM;

	sinfo->nskt = nr;

	/* Initialize processor specific parameters */
	for (i = 0; i < nr; i++) {
		skt = &sinfo->skt[i];

		skt->nr = first + i;
		skt->clk = clk;
		soc_pcmcia_init_one(skt, ops, dev);

		ret = sa11xx_drv_pcmcia_add_one(skt);
		if (ret)
			break;
	}

	if (ret) {
		while (--i >= 0)
			soc_pcmcia_remove_one(&sinfo->skt[i]);
	} else {
		dev_set_drvdata(dev, sinfo);
	}

	return ret;
}

/*
 * These are offsets from the above base.
 */
#define PCCR	0x0000
#define PCSSR	0x0004
#define PCSR	0x0008

#define PCSR_S0_READY	(1<<0)
#define PCSR_S1_READY	(1<<1)
#define PCSR_S0_DETECT	(1<<2)
#define PCSR_S1_DETECT	(1<<3)
#define PCSR_S0_VS1	(1<<4)
#define PCSR_S0_VS2	(1<<5)
#define PCSR_S1_VS1	(1<<6)
#define PCSR_S1_VS2	(1<<7)
#define PCSR_S0_WP	(1<<8)
#define PCSR_S1_WP	(1<<9)
#define PCSR_S0_BVD1	(1<<10)
#define PCSR_S0_BVD2	(1<<11)
#define PCSR_S1_BVD1	(1<<12)
#define PCSR_S1_BVD2	(1<<13)

#define PCCR_S0_RST	(1<<0)
#define PCCR_S1_RST	(1<<1)
#define PCCR_S0_FLT	(1<<2)
#define PCCR_S1_FLT	(1<<3)
#define PCCR_S0_PWAITEN	(1<<4)
#define PCCR_S1_PWAITEN	(1<<5)
#define PCCR_S0_PSE	(1<<6)
#define PCCR_S1_PSE	(1<<7)

#define PCSSR_S0_SLEEP	(1<<0)
#define PCSSR_S1_SLEEP	(1<<1)

#define IDX_IRQ_S0_READY_NINT	(0)
#define IDX_IRQ_S0_CD_VALID	(1)
#define IDX_IRQ_S0_BVD1_STSCHG	(2)
#define IDX_IRQ_S1_READY_NINT	(3)
#define IDX_IRQ_S1_CD_VALID	(4)
#define IDX_IRQ_S1_BVD1_STSCHG	(5)
#define NUM_IRQS		(6)

struct sa1111_pcmcia_socket {
	struct soc_pcmcia_socket soc;
	struct sa1111_dev *dev;
	struct sa1111_pcmcia_socket *next;
};

static inline struct sa1111_pcmcia_socket *to_skt(struct soc_pcmcia_socket *s)
{
	return container_of(s, struct sa1111_pcmcia_socket, soc);
}

void sa1111_pcmcia_socket_state(struct soc_pcmcia_socket *skt, struct pcmcia_state *state)
{
	struct sa1111_pcmcia_socket *s = to_skt(skt);
	u32 status = readl_relaxed(s->dev->mapbase + PCSR);

	switch (skt->nr) {
	case 0:
		state->detect = status & PCSR_S0_DETECT ? 0 : 1;
		state->ready  = status & PCSR_S0_READY  ? 1 : 0;
		state->bvd1   = status & PCSR_S0_BVD1   ? 1 : 0;
		state->bvd2   = status & PCSR_S0_BVD2   ? 1 : 0;
		state->wrprot = status & PCSR_S0_WP     ? 1 : 0;
		state->vs_3v  = status & PCSR_S0_VS1    ? 0 : 1;
		state->vs_Xv  = status & PCSR_S0_VS2    ? 0 : 1;
		break;

	case 1:
		state->detect = status & PCSR_S1_DETECT ? 0 : 1;
		state->ready  = status & PCSR_S1_READY  ? 1 : 0;
		state->bvd1   = status & PCSR_S1_BVD1   ? 1 : 0;
		state->bvd2   = status & PCSR_S1_BVD2   ? 1 : 0;
		state->wrprot = status & PCSR_S1_WP     ? 1 : 0;
		state->vs_3v  = status & PCSR_S1_VS1    ? 0 : 1;
		state->vs_Xv  = status & PCSR_S1_VS2    ? 0 : 1;
		break;
	}
}

int sa1111_pcmcia_configure_socket(struct soc_pcmcia_socket *skt, const socket_state_t *state)
{
	struct sa1111_pcmcia_socket *s = to_skt(skt);
	u32 pccr_skt_mask, pccr_set_mask, val;
	unsigned long flags;

	switch (skt->nr) {
	case 0:
		pccr_skt_mask = PCCR_S0_RST|PCCR_S0_FLT|PCCR_S0_PWAITEN|PCCR_S0_PSE;
		break;

	case 1:
		pccr_skt_mask = PCCR_S1_RST|PCCR_S1_FLT|PCCR_S1_PWAITEN|PCCR_S1_PSE;
		break;

	default:
		return -1;
	}

	pccr_set_mask = 0;

	if (state->Vcc != 0)
		pccr_set_mask |= PCCR_S0_PWAITEN|PCCR_S1_PWAITEN;
	if (state->Vcc == 50)
		pccr_set_mask |= PCCR_S0_PSE|PCCR_S1_PSE;
	if (state->flags & SS_RESET)
		pccr_set_mask |= PCCR_S0_RST|PCCR_S1_RST;
	if (state->flags & SS_OUTPUT_ENA)
		pccr_set_mask |= PCCR_S0_FLT|PCCR_S1_FLT;

	local_irq_save(flags);
	val = readl_relaxed(s->dev->mapbase + PCCR);
	val &= ~pccr_skt_mask;
	val |= pccr_set_mask & pccr_skt_mask;
	writel_relaxed(val, s->dev->mapbase + PCCR);
	local_irq_restore(flags);

	return 0;
}

enum {
	MAX1600_GPIO_0VCC = 0,
	MAX1600_GPIO_1VCC,
	MAX1600_GPIO_0VPP,
	MAX1600_GPIO_1VPP,
	MAX1600_GPIO_MAX,

	MAX1600_CHAN_A,
	MAX1600_CHAN_B,

	MAX1600_CODE_LOW,
	MAX1600_CODE_HIGH,
};

struct max1600 {
	struct gpio_desc *gpio[MAX1600_GPIO_MAX];
	struct device *dev;
	unsigned int code;
};

static const char *max1600_gpio_name[2][MAX1600_GPIO_MAX] = {
	{ "a0vcc", "a1vcc", "a0vpp", "a1vpp" },
	{ "b0vcc", "b1vcc", "b0vpp", "b1vpp" },
};

int max1600_init(struct device *dev, struct max1600 **ptr,
	unsigned int channel, unsigned int code)
{
	struct max1600 *m;
	int chan;
	int i;

	switch (channel) {
	case MAX1600_CHAN_A:
		chan = 0;
		break;
	case MAX1600_CHAN_B:
		chan = 1;
		break;
	default:
		return -EINVAL;
	}

	if (code != MAX1600_CODE_LOW && code != MAX1600_CODE_HIGH)
		return -EINVAL;

	m = devm_kzalloc(dev, sizeof(*m), GFP_KERNEL);
	if (!m)
		return -ENOMEM;

	m->dev = dev;
	m->code = code;

	for (i = 0; i < MAX1600_GPIO_MAX; i++) {
		const char *name;

		name = max1600_gpio_name[chan][i];
		if (i != MAX1600_GPIO_0VPP) {
			m->gpio[i] = devm_gpiod_get(dev, name, GPIOD_OUT_LOW);
		} else {
			m->gpio[i] = devm_gpiod_get_optional(dev, name,
							     GPIOD_OUT_LOW);
			if (!m->gpio[i])
				break;
		}
		if (IS_ERR(m->gpio[i]))
			return PTR_ERR(m->gpio[i]);
	}

	*ptr = m;

	return 0;
}

int max1600_configure(struct max1600 *m, unsigned int vcc, unsigned int vpp)
{
	DECLARE_BITMAP(values, MAX1600_GPIO_MAX) = { 0, };
	int n = MAX1600_GPIO_0VPP;

	if (m->gpio[MAX1600_GPIO_0VPP]) {
		if (vpp == 0) {
			__assign_bit(MAX1600_GPIO_0VPP, values, 0);
			__assign_bit(MAX1600_GPIO_1VPP, values, 0);
		} else if (vpp == 120) {
			__assign_bit(MAX1600_GPIO_0VPP, values, 0);
			__assign_bit(MAX1600_GPIO_1VPP, values, 1);
		} else if (vpp == vcc) {
			__assign_bit(MAX1600_GPIO_0VPP, values, 1);
			__assign_bit(MAX1600_GPIO_1VPP, values, 0);
		} else {
			dev_err(m->dev, "unrecognised Vpp %u.%uV\n",
				vpp / 10, vpp % 10);
			return -EINVAL;
		}
		n = MAX1600_GPIO_MAX;
	} else if (vpp != vcc && vpp != 0) {
		dev_err(m->dev, "no VPP control\n");
		return -EINVAL;
	}

	if (vcc == 0) {
		__assign_bit(MAX1600_GPIO_0VCC, values, 0);
		__assign_bit(MAX1600_GPIO_1VCC, values, 0);
	} else if (vcc == 33) { /* VY */
		__assign_bit(MAX1600_GPIO_0VCC, values, 1);
		__assign_bit(MAX1600_GPIO_1VCC, values, 0);
	} else if (vcc == 50) { /* VX */
		__assign_bit(MAX1600_GPIO_0VCC, values, 0);
		__assign_bit(MAX1600_GPIO_1VCC, values, 1);
	} else {
		dev_err(m->dev, "unrecognised Vcc %u.%uV\n",
			vcc / 10, vcc % 10);
		return -EINVAL;
	}

	if (m->code == MAX1600_CODE_HIGH) {
		/*
		 * Cirrus mode appears to be the same as Intel mode,
		 * except the VCC pins are inverted.
		 */
		__change_bit(MAX1600_GPIO_0VCC, values);
		__change_bit(MAX1600_GPIO_1VCC, values);
	}

	return gpiod_set_array_value_cansleep(n, m->gpio, NULL, values);
}
EXPORT_SYMBOL_GPL(max1600_configure);


static int sa1111_pcmcia_add(struct sa1111_dev *dev, struct pcmcia_low_level *ops,
	int (*add)(struct soc_pcmcia_socket *))
{
	struct sa1111_pcmcia_socket *s;
	struct clk *clk;
	int i, ret = 0, irqs[NUM_IRQS];

	clk = devm_clk_get(&dev->dev, NULL);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	for (i = 0; i < NUM_IRQS; i++) {
		irqs[i] = sa1111_get_irq(dev, i);
		if (irqs[i] <= 0)
			return irqs[i] ? : -ENXIO;
	}

	ops->socket_state = sa1111_pcmcia_socket_state;

	for (i = 0; i < ops->nr; i++) {
		s = kzalloc(sizeof(*s), GFP_KERNEL);
		if (!s)
			return -ENOMEM;

		s->soc.nr = ops->first + i;
		s->soc.clk = clk;

		soc_pcmcia_init_one(&s->soc, ops, &dev->dev);
		s->dev = dev;
		if (s->soc.nr) {
			s->soc.socket.pci_irq = irqs[IDX_IRQ_S1_READY_NINT];
			s->soc.stat[SOC_STAT_CD].irq = irqs[IDX_IRQ_S1_CD_VALID];
			s->soc.stat[SOC_STAT_CD].name = "SA1111 CF card detect";
			s->soc.stat[SOC_STAT_BVD1].irq = irqs[IDX_IRQ_S1_BVD1_STSCHG];
			s->soc.stat[SOC_STAT_BVD1].name = "SA1111 CF BVD1";
		} else {
			s->soc.socket.pci_irq = irqs[IDX_IRQ_S0_READY_NINT];
			s->soc.stat[SOC_STAT_CD].irq = irqs[IDX_IRQ_S0_CD_VALID];
			s->soc.stat[SOC_STAT_CD].name = "SA1111 PCMCIA card detect";
			s->soc.stat[SOC_STAT_BVD1].irq = irqs[IDX_IRQ_S0_BVD1_STSCHG];
			s->soc.stat[SOC_STAT_BVD1].name = "SA1111 PCMCIA BVD1";
		}

		ret = add(&s->soc);
		if (ret == 0) {
			s->next = dev_get_drvdata(&dev->dev);
			dev_set_drvdata(&dev->dev, s);
		} else
			kfree(s);
	}

	return ret;
}

/*
 * Neponset uses the Maxim MAX1600, with the following connections:
 *
 *   MAX1600      Neponset
 *
 *    A0VCC        SA-1111 GPIO A<1>
 *    A1VCC        SA-1111 GPIO A<0>
 *    A0VPP        CPLD NCR A0VPP
 *    A1VPP        CPLD NCR A1VPP
 *    B0VCC        SA-1111 GPIO A<2>
 *    B1VCC        SA-1111 GPIO A<3>
 *    B0VPP        ground (slot B is CF)
 *    B1VPP        ground (slot B is CF)
 *
 *     VX          VCC (5V)
 *     VY          VCC3_3 (3.3V)
 *     12INA       12V
 *     12INB       ground (slot B is CF)
 *
 * The MAX1600 CODE pin is tied to ground, placing the device in 
 * "Standard Intel code" mode. Refer to the Maxim data sheet for
 * the corresponding truth table.
 */
static int neponset_pcmcia_hw_init(struct soc_pcmcia_socket *skt)
{
	struct max1600 *m;
	int ret;

	ret = max1600_init(skt->socket.dev.parent, &m,
			   skt->nr ? MAX1600_CHAN_B : MAX1600_CHAN_A,
			   MAX1600_CODE_LOW);
	if (ret == 0)
		skt->driver_data = m;

	return ret;
}

static int
neponset_pcmcia_configure_socket(struct soc_pcmcia_socket *skt, const socket_state_t *state)
{
	struct max1600 *m = skt->driver_data;
	int ret;

	ret = sa1111_pcmcia_configure_socket(skt, state);
	if (ret == 0)
		ret = max1600_configure(m, state->Vcc, state->Vpp);

	return ret;
}

static struct pcmcia_low_level neponset_pcmcia_ops = {
	.owner			= THIS_MODULE,
	.hw_init		= neponset_pcmcia_hw_init,
	.configure_socket	= neponset_pcmcia_configure_socket,
	.first			= 0,
	.nr			= 2,
};

static int pcmcia_neponset_init(struct sa1111_dev *sadev)
{
	sa11xx_drv_pcmcia_ops(&neponset_pcmcia_ops);
	return sa1111_pcmcia_add(sadev, &neponset_pcmcia_ops,
				 sa11xx_drv_pcmcia_add_one);
}

/*
 * Socket 0 power: GPIO A0
 * Socket 0 3V: GPIO A2
 * Socket 1 power: GPIO A1 & GPIO A3
 * Socket 1 3V: GPIO A3
 * Does Socket 1 3V actually do anything?
 */
enum {
	J720_GPIO_PWR,
	J720_GPIO_3V,
	J720_GPIO_MAX,
};
struct jornada720_data {
	struct gpio_desc *gpio[J720_GPIO_MAX];
};

static int jornada720_pcmcia_hw_init(struct soc_pcmcia_socket *skt)
{
	struct device *dev = skt->socket.dev.parent;
	struct jornada720_data *j;

	j = devm_kzalloc(dev, sizeof(*j), GFP_KERNEL);
	if (!j)
		return -ENOMEM;

	j->gpio[J720_GPIO_PWR] = devm_gpiod_get(dev, skt->nr ? "s1-power" :
						"s0-power", GPIOD_OUT_LOW);
	if (IS_ERR(j->gpio[J720_GPIO_PWR]))
		return PTR_ERR(j->gpio[J720_GPIO_PWR]);

	j->gpio[J720_GPIO_3V] = devm_gpiod_get(dev, skt->nr ? "s1-3v" :
					       "s0-3v", GPIOD_OUT_LOW);
	if (IS_ERR(j->gpio[J720_GPIO_3V]))
		return PTR_ERR(j->gpio[J720_GPIO_3V]);

	skt->driver_data = j;

	return 0;
}

static int
jornada720_pcmcia_configure_socket(struct soc_pcmcia_socket *skt, const socket_state_t *state)
{
	struct jornada720_data *j = skt->driver_data;
	DECLARE_BITMAP(values, J720_GPIO_MAX) = { 0, };
	int ret;

	printk(KERN_INFO "%s(): config socket %d vcc %d vpp %d\n", __func__,
		skt->nr, state->Vcc, state->Vpp);

	switch (skt->nr) {
	case 0:
		switch (state->Vcc) {
		default:
		case  0:
			__assign_bit(J720_GPIO_PWR, values, 0);
			__assign_bit(J720_GPIO_3V, values, 0);
			break;
		case 33:
			__assign_bit(J720_GPIO_PWR, values, 1);
			__assign_bit(J720_GPIO_3V, values, 1);
			break;
		case 50:
			__assign_bit(J720_GPIO_PWR, values, 1);
			__assign_bit(J720_GPIO_3V, values, 0);
			break;
		}
		break;

	case 1:
		switch (state->Vcc) {
		default:
		case 0:
			__assign_bit(J720_GPIO_PWR, values, 0);
			__assign_bit(J720_GPIO_3V, values, 0);
			break;
		case 33:
		case 50:
			__assign_bit(J720_GPIO_PWR, values, 1);
			__assign_bit(J720_GPIO_3V, values, 1);
			break;
		}
		break;

	default:
		return -1;
	}

	if (state->Vpp != state->Vcc && state->Vpp != 0) {
		printk(KERN_ERR "%s(): slot cannot support VPP %u\n",
			__func__, state->Vpp);
		return -EPERM;
	}

	ret = sa1111_pcmcia_configure_socket(skt, state);
	if (ret == 0)
		ret = gpiod_set_array_value_cansleep(J720_GPIO_MAX, j->gpio,
						     NULL, values);

	return ret;
}

static struct pcmcia_low_level jornada720_pcmcia_ops = {
	.owner			= THIS_MODULE,
	.hw_init		= jornada720_pcmcia_hw_init,
	.configure_socket	= jornada720_pcmcia_configure_socket,
	.first			= 0,
	.nr			= 2,
};

static int pcmcia_jornada720_init(struct sa1111_dev *sadev)
{
	/* Fixme: why messing around with SA11x0's GPIO1? */
	GRER |= 0x00000002;

	sa11xx_drv_pcmcia_ops(&jornada720_pcmcia_ops);
	return sa1111_pcmcia_add(sadev, &jornada720_pcmcia_ops,
				 sa11xx_drv_pcmcia_add_one);
}

static int h3600_pcmcia_hw_init(struct soc_pcmcia_socket *skt)
{
	int err;

	skt->stat[SOC_STAT_CD].name = skt->nr ? "pcmcia1-detect" : "pcmcia0-detect";
	skt->stat[SOC_STAT_RDY].name = skt->nr ? "pcmcia1-ready" : "pcmcia0-ready";

	err = soc_pcmcia_request_gpiods(skt);
	if (err)
		return err;

	switch (skt->nr) {
	case 0:
		err = gpio_request(H3XXX_EGPIO_OPT_NVRAM_ON, "OPT NVRAM ON");
		if (err)
			goto err01;
		err = gpio_direction_output(H3XXX_EGPIO_OPT_NVRAM_ON, 0);
		if (err)
			goto err03;
		err = gpio_request(H3XXX_EGPIO_OPT_ON, "OPT ON");
		if (err)
			goto err03;
		err = gpio_direction_output(H3XXX_EGPIO_OPT_ON, 0);
		if (err)
			goto err04;
		err = gpio_request(H3XXX_EGPIO_OPT_RESET, "OPT RESET");
		if (err)
			goto err04;
		err = gpio_direction_output(H3XXX_EGPIO_OPT_RESET, 0);
		if (err)
			goto err05;
		err = gpio_request(H3XXX_EGPIO_CARD_RESET, "PCMCIA CARD RESET");
		if (err)
			goto err05;
		err = gpio_direction_output(H3XXX_EGPIO_CARD_RESET, 0);
		if (err)
			goto err06;
		break;
	case 1:
		break;
	}
	return 0;

err06:	gpio_free(H3XXX_EGPIO_CARD_RESET);
err05:	gpio_free(H3XXX_EGPIO_OPT_RESET);
err04:	gpio_free(H3XXX_EGPIO_OPT_ON);
err03:	gpio_free(H3XXX_EGPIO_OPT_NVRAM_ON);
err01:	gpio_free(H3XXX_GPIO_PCMCIA_IRQ0);
	return err;
}

static void h3600_pcmcia_hw_shutdown(struct soc_pcmcia_socket *skt)
{
	switch (skt->nr) {
	case 0:
		/* Disable CF bus: */
		gpio_set_value(H3XXX_EGPIO_OPT_NVRAM_ON, 0);
		gpio_set_value(H3XXX_EGPIO_OPT_ON, 0);
		gpio_set_value(H3XXX_EGPIO_OPT_RESET, 1);

		gpio_free(H3XXX_EGPIO_CARD_RESET);
		gpio_free(H3XXX_EGPIO_OPT_RESET);
		gpio_free(H3XXX_EGPIO_OPT_ON);
		gpio_free(H3XXX_EGPIO_OPT_NVRAM_ON);
		break;
	case 1:
		break;
	}
}

static void
h3600_pcmcia_socket_state(struct soc_pcmcia_socket *skt, struct pcmcia_state *state)
{
	state->bvd1 = 0;
	state->bvd2 = 0;
	state->vs_3v = 0;
	state->vs_Xv = 0;
}

static int
h3600_pcmcia_configure_socket(struct soc_pcmcia_socket *skt, const socket_state_t *state)
{
	if (state->Vcc != 0 && state->Vcc != 33 && state->Vcc != 50) {
		printk(KERN_ERR "h3600_pcmcia: unrecognized Vcc %u.%uV\n",
		       state->Vcc / 10, state->Vcc % 10);
		return -1;
	}

	gpio_set_value(H3XXX_EGPIO_CARD_RESET, !!(state->flags & SS_RESET));

	/* Silently ignore Vpp, output enable, speaker enable. */

	return 0;
}

static void h3600_pcmcia_socket_init(struct soc_pcmcia_socket *skt)
{
	/* Enable CF bus: */
	gpio_set_value(H3XXX_EGPIO_OPT_NVRAM_ON, 1);
	gpio_set_value(H3XXX_EGPIO_OPT_ON, 1);
	gpio_set_value(H3XXX_EGPIO_OPT_RESET, 0);

	msleep(10);
}

static void h3600_pcmcia_socket_suspend(struct soc_pcmcia_socket *skt)
{
	/*
	 * FIXME:  This doesn't fit well.  We don't have the mechanism in
	 * the generic PCMCIA layer to deal with the idea of two sockets
	 * on one bus.  We rely on the cs.c behaviour shutting down
	 * socket 0 then socket 1.
	 */
	if (skt->nr == 1) {
		gpio_set_value(H3XXX_EGPIO_OPT_ON, 0);
		gpio_set_value(H3XXX_EGPIO_OPT_NVRAM_ON, 0);
		/* hmm, does this suck power? */
		gpio_set_value(H3XXX_EGPIO_OPT_RESET, 1);
	}
}

struct pcmcia_low_level h3600_pcmcia_ops = { 
	.owner			= THIS_MODULE,
	.hw_init		= h3600_pcmcia_hw_init,
	.hw_shutdown		= h3600_pcmcia_hw_shutdown,
	.socket_state		= h3600_pcmcia_socket_state,
	.configure_socket	= h3600_pcmcia_configure_socket,

	.socket_init		= h3600_pcmcia_socket_init,
	.socket_suspend		= h3600_pcmcia_socket_suspend,
};

int pcmcia_h3600_init(struct device *dev)
{
	int ret = -ENODEV;

	if (machine_is_h3600())
		ret = sa11xx_drv_pcmcia_probe(dev, &h3600_pcmcia_ops, 0, 2);

	return ret;
}

static int sa1111_pcmcia_probe(struct sa1111_dev *dev)
{
	void __iomem *base;
	int ret;

	ret = sa1111_enable_device(dev);
	if (ret)
		return ret;

	dev_set_drvdata(&dev->dev, NULL);

	if (!request_mem_region(dev->res.start, 512, SA1111_DRIVER_NAME(dev))) {
		sa1111_disable_device(dev);
		return -EBUSY;
	}

	base = dev->mapbase;

	/*
	 * Initialise the suspend state.
	 */
	writel_relaxed(PCSSR_S0_SLEEP | PCSSR_S1_SLEEP, base + PCSSR);
	writel_relaxed(PCCR_S0_FLT | PCCR_S1_FLT, base + PCCR);

	ret = -ENODEV;
	if (machine_is_jornada720())
		ret = pcmcia_jornada720_init(dev);
	if (machine_is_assabet())
		ret = pcmcia_neponset_init(dev);

	if (ret) {
		release_mem_region(dev->res.start, 512);
		sa1111_disable_device(dev);
	}

	return ret;
}

static void sa1111_pcmcia_remove(struct sa1111_dev *dev)
{
	struct sa1111_pcmcia_socket *next, *s = dev_get_drvdata(&dev->dev);

	dev_set_drvdata(&dev->dev, NULL);

	for (; s; s = next) {
		next = s->next;
		soc_pcmcia_remove_one(&s->soc);
		kfree(s);
	}

	release_mem_region(dev->res.start, 512);
	sa1111_disable_device(dev);
}

static struct sa1111_driver pcmcia_driver = {
	.drv = {
		.name	= "sa1111-pcmcia",
	},
	.devid		= SA1111_DEVID_PCMCIA,
	.probe		= sa1111_pcmcia_probe,
	.remove		= sa1111_pcmcia_remove,
};

static int __init sa1111_drv_pcmcia_init(void)
{
	if (!IS_ENABLED(CONFIG_SA1111))
		return 0;

	return sa1111_driver_register(&pcmcia_driver);
}

static void __exit sa1111_drv_pcmcia_exit(void)
{
	if (!IS_ENABLED(CONFIG_SA1111))
		return;

	sa1111_driver_unregister(&pcmcia_driver);
}

fs_initcall(sa1111_drv_pcmcia_init);
module_exit(sa1111_drv_pcmcia_exit);

static const char *sa11x0_cf_gpio_names[] = {
	[SOC_STAT_CD] = "detect",
	[SOC_STAT_BVD1] = "bvd1",
	[SOC_STAT_BVD2] = "bvd2",
	[SOC_STAT_RDY] = "ready",
};

static int sa11x0_cf_hw_init(struct soc_pcmcia_socket *skt)
{
	struct device *dev = skt->socket.dev.parent;
	int i;

	skt->gpio_reset = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(skt->gpio_reset))
		return PTR_ERR(skt->gpio_reset);

	skt->gpio_bus_enable = devm_gpiod_get_optional(dev, "bus-enable",
						       GPIOD_OUT_HIGH);
	if (IS_ERR(skt->gpio_bus_enable))
		return PTR_ERR(skt->gpio_bus_enable);

	skt->vcc.reg = devm_regulator_get_optional(dev, "vcc");
	if (IS_ERR(skt->vcc.reg))
		return PTR_ERR(skt->vcc.reg);

	if (!skt->vcc.reg)
		dev_warn(dev,
			 "no Vcc regulator provided, ignoring Vcc controls\n");

	for (i = 0; i < ARRAY_SIZE(sa11x0_cf_gpio_names); i++) {
		skt->stat[i].name = sa11x0_cf_gpio_names[i];
		skt->stat[i].desc = devm_gpiod_get_optional(dev,
					sa11x0_cf_gpio_names[i], GPIOD_IN);
		if (IS_ERR(skt->stat[i].desc))
			return PTR_ERR(skt->stat[i].desc);
	}
	return 0;
}

static int sa11x0_cf_configure_socket(struct soc_pcmcia_socket *skt,
	const socket_state_t *state)
{
	return soc_pcmcia_regulator_set(skt, &skt->vcc, state->Vcc);
}

static struct pcmcia_low_level sa11x0_cf_ops = {
	.owner = THIS_MODULE,
	.hw_init = sa11x0_cf_hw_init,
	.socket_state = soc_common_cf_socket_state,
	.configure_socket = sa11x0_cf_configure_socket,
};

int pcmcia_collie_init(struct device *dev)
{
       int ret = -ENODEV;

       if (machine_is_collie())
               ret = sa11xx_drv_pcmcia_probe(dev, &sharpsl_pcmcia_ops, 0, 1);

       return ret;
}

static int (*sa11x0_pcmcia_legacy_hw_init[])(struct device *dev) = {
#ifdef CONFIG_SA1100_H3600
	pcmcia_h3600_init,
#endif
#ifdef CONFIG_SA1100_COLLIE
       pcmcia_collie_init,
#endif
};

static int sa11x0_drv_pcmcia_legacy_probe(struct platform_device *dev)
{
	int i, ret = -ENODEV;

	/*
	 * Initialise any "on-board" PCMCIA sockets.
	 */
	for (i = 0; i < ARRAY_SIZE(sa11x0_pcmcia_legacy_hw_init); i++) {
		ret = sa11x0_pcmcia_legacy_hw_init[i](&dev->dev);
		if (ret == 0)
			break;
	}

	return ret;
}

static void sa11x0_drv_pcmcia_legacy_remove(struct platform_device *dev)
{
	struct skt_dev_info *sinfo = platform_get_drvdata(dev);
	int i;

	platform_set_drvdata(dev, NULL);

	for (i = 0; i < sinfo->nskt; i++)
		soc_pcmcia_remove_one(&sinfo->skt[i]);
}

static int sa11x0_drv_pcmcia_probe(struct platform_device *pdev)
{
	struct soc_pcmcia_socket *skt;
	struct device *dev = &pdev->dev;

	if (pdev->id == -1)
		return sa11x0_drv_pcmcia_legacy_probe(pdev);

	skt = devm_kzalloc(dev, sizeof(*skt), GFP_KERNEL);
	if (!skt)
		return -ENOMEM;

	platform_set_drvdata(pdev, skt);

	skt->nr = pdev->id;
	skt->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(skt->clk))
		return PTR_ERR(skt->clk);

	sa11xx_drv_pcmcia_ops(&sa11x0_cf_ops);
	soc_pcmcia_init_one(skt, &sa11x0_cf_ops, dev);

	return sa11xx_drv_pcmcia_add_one(skt);
}

static int sa11x0_drv_pcmcia_remove(struct platform_device *dev)
{
	struct soc_pcmcia_socket *skt;

	if (dev->id == -1) {
		sa11x0_drv_pcmcia_legacy_remove(dev);
		return 0;
	}

	skt = platform_get_drvdata(dev);

	soc_pcmcia_remove_one(skt);

	return 0;
}

static struct platform_driver sa11x0_pcmcia_driver = {
	.driver = {
		.name		= "sa11x0-pcmcia",
	},
	.probe		= sa11x0_drv_pcmcia_probe,
	.remove		= sa11x0_drv_pcmcia_remove,
};

/* sa11x0_pcmcia_init()
 * ^^^^^^^^^^^^^^^^^^^^
 *
 * This routine performs low-level PCMCIA initialization and then
 * registers this socket driver with Card Services.
 *
 * Returns: 0 on success, -ve error code on failure
 */
static int __init sa11x0_pcmcia_init(void)
{
	return platform_driver_register(&sa11x0_pcmcia_driver);
}

/* sa11x0_pcmcia_exit()
 * ^^^^^^^^^^^^^^^^^^^^
 * Invokes the low-level kernel service to free IRQs associated with this
 * socket controller and reset GPIO edge detection.
 */
static void __exit sa11x0_pcmcia_exit(void)
{
	platform_driver_unregister(&sa11x0_pcmcia_driver);
}

fs_initcall(sa11x0_pcmcia_init);
module_exit(sa11x0_pcmcia_exit);
#endif

#ifdef CONFIG_ARCH_PXA
static inline u_int pxa2xx_mcxx_hold(u_int pcmcia_cycle_ns,
				     u_int mem_clk_10khz)
{
	u_int code = pcmcia_cycle_ns * mem_clk_10khz;
	return (code / 300000) + ((code % 300000) ? 1 : 0) - 1;
}

static inline u_int pxa2xx_mcxx_asst(u_int pcmcia_cycle_ns,
				     u_int mem_clk_10khz)
{
	u_int code = pcmcia_cycle_ns * mem_clk_10khz;
	return (code / 300000) + ((code % 300000) ? 1 : 0) + 1;
}

static inline u_int pxa2xx_mcxx_setup(u_int pcmcia_cycle_ns,
				      u_int mem_clk_10khz)
{
	u_int code = pcmcia_cycle_ns * mem_clk_10khz;
	return (code / 100000) + ((code % 100000) ? 1 : 0) - 1;
}

/* This function returns the (approximate) command assertion period, in
 * nanoseconds, for a given CPU clock frequency and MCXX_ASST value:
 */
static inline u_int pxa2xx_pcmcia_cmd_time(u_int mem_clk_10khz,
					   u_int pcmcia_mcxx_asst)
{
	return (300000 * (pcmcia_mcxx_asst + 1) / mem_clk_10khz);
}

static uint32_t pxa2xx_pcmcia_mcmem(int sock, int speed, int clock)
{
	uint32_t val;

	val = ((pxa2xx_mcxx_setup(speed, clock)
		& MCXX_SETUP_MASK) << MCXX_SETUP_SHIFT)
		| ((pxa2xx_mcxx_asst(speed, clock)
		& MCXX_ASST_MASK) << MCXX_ASST_SHIFT)
		| ((pxa2xx_mcxx_hold(speed, clock)
		& MCXX_HOLD_MASK) << MCXX_HOLD_SHIFT);

	return val;
}

static int pxa2xx_pcmcia_mcio(int sock, int speed, int clock)
{
	uint32_t val;

	val = ((pxa2xx_mcxx_setup(speed, clock)
		& MCXX_SETUP_MASK) << MCXX_SETUP_SHIFT)
		| ((pxa2xx_mcxx_asst(speed, clock)
		& MCXX_ASST_MASK) << MCXX_ASST_SHIFT)
		| ((pxa2xx_mcxx_hold(speed, clock)
		& MCXX_HOLD_MASK) << MCXX_HOLD_SHIFT);


	return val;
}

static int pxa2xx_pcmcia_mcatt(int sock, int speed, int clock)
{
	uint32_t val;

	val = ((pxa2xx_mcxx_setup(speed, clock)
		& MCXX_SETUP_MASK) << MCXX_SETUP_SHIFT)
		| ((pxa2xx_mcxx_asst(speed, clock)
		& MCXX_ASST_MASK) << MCXX_ASST_SHIFT)
		| ((pxa2xx_mcxx_hold(speed, clock)
		& MCXX_HOLD_MASK) << MCXX_HOLD_SHIFT);


	return val;
}

static int pxa2xx_pcmcia_set_timing(struct soc_pcmcia_socket *skt)
{
	unsigned long clk = clk_get_rate(skt->clk) / 10000;
	struct soc_pcmcia_timing timing;
	int sock = skt->nr;

	soc_common_pcmcia_get_timing(skt, &timing);

	pxa_smemc_set_pcmcia_timing(sock,
		pxa2xx_pcmcia_mcmem(sock, timing.mem, clk),
		pxa2xx_pcmcia_mcatt(sock, timing.attr, clk),
		pxa2xx_pcmcia_mcio(sock, timing.io, clk));

	return 0;
}

#ifdef CONFIG_CPU_FREQ

static int
pxa2xx_pcmcia_frequency_change(struct soc_pcmcia_socket *skt,
			       unsigned long val,
			       struct cpufreq_freqs *freqs)
{
	switch (val) {
	case CPUFREQ_PRECHANGE:
		if (freqs->new > freqs->old) {
			debug(skt, 2, "new frequency %u.%uMHz > %u.%uMHz, "
			       "pre-updating\n",
			       freqs->new / 1000, (freqs->new / 100) % 10,
			       freqs->old / 1000, (freqs->old / 100) % 10);
			pxa2xx_pcmcia_set_timing(skt);
		}
		break;

	case CPUFREQ_POSTCHANGE:
		if (freqs->new < freqs->old) {
			debug(skt, 2, "new frequency %u.%uMHz < %u.%uMHz, "
			       "post-updating\n",
			       freqs->new / 1000, (freqs->new / 100) % 10,
			       freqs->old / 1000, (freqs->old / 100) % 10);
			pxa2xx_pcmcia_set_timing(skt);
		}
		break;
	}
	return 0;
}
#endif

void pxa2xx_configure_sockets(struct device *dev, struct pcmcia_low_level *ops)
{
	pxa_smemc_set_pcmcia_socket(1);
}

#define SKT_DEV_INFO_SIZE(n) \
	(sizeof(struct skt_dev_info) + (n)*sizeof(struct soc_pcmcia_socket))

int pxa2xx_drv_pcmcia_add_one(struct soc_pcmcia_socket *skt)
{
	skt->res_skt.start = _PCMCIA(skt->nr);
	skt->res_skt.end = _PCMCIA(skt->nr) + PCMCIASp - 1;
	skt->res_skt.name = skt_names[skt->nr];
	skt->res_skt.flags = IORESOURCE_MEM;

	skt->res_io.start = _PCMCIAIO(skt->nr);
	skt->res_io.end = _PCMCIAIO(skt->nr) + PCMCIAIOSp - 1;
	skt->res_io.name = "io";
	skt->res_io.flags = IORESOURCE_MEM | IORESOURCE_BUSY;

	skt->res_mem.start = _PCMCIAMem(skt->nr);
	skt->res_mem.end = _PCMCIAMem(skt->nr) + PCMCIAMemSp - 1;
	skt->res_mem.name = "memory";
	skt->res_mem.flags = IORESOURCE_MEM;

	skt->res_attr.start = _PCMCIAAttr(skt->nr);
	skt->res_attr.end = _PCMCIAAttr(skt->nr) + PCMCIAAttrSp - 1;
	skt->res_attr.name = "attribute";
	skt->res_attr.flags = IORESOURCE_MEM;

	return soc_pcmcia_add_one(skt);
}

void pxa2xx_drv_pcmcia_ops(struct pcmcia_low_level *ops)
{
	/* Provide our PXA2xx specific timing routines. */
	ops->set_timing  = pxa2xx_pcmcia_set_timing;
#ifdef CONFIG_CPU_FREQ
	ops->frequency_change = pxa2xx_pcmcia_frequency_change;
#endif
}

static int pxa2xx_drv_pcmcia_probe(struct platform_device *dev)
{
	int i, ret = 0;
	struct pcmcia_low_level *ops;
	struct skt_dev_info *sinfo;
	struct soc_pcmcia_socket *skt;
	struct clk *clk;

	ops = (struct pcmcia_low_level *)dev->dev.platform_data;
	if (!ops) {
		ret = -ENODEV;
		goto err0;
	}

	if (cpu_is_pxa320() && ops->nr > 1) {
		dev_err(&dev->dev, "pxa320 supports only one pcmcia slot");
		ret = -EINVAL;
		goto err0;
	}

	clk = devm_clk_get(&dev->dev, NULL);
	if (IS_ERR(clk))
		return -ENODEV;

	pxa2xx_drv_pcmcia_ops(ops);

	sinfo = devm_kzalloc(&dev->dev, SKT_DEV_INFO_SIZE(ops->nr),
			     GFP_KERNEL);
	if (!sinfo)
		return -ENOMEM;

	sinfo->nskt = ops->nr;

	/* Initialize processor specific parameters */
	for (i = 0; i < ops->nr; i++) {
		skt = &sinfo->skt[i];

		skt->nr = ops->first + i;
		skt->clk = clk;
		soc_pcmcia_init_one(skt, ops, &dev->dev);

		ret = pxa2xx_drv_pcmcia_add_one(skt);
		if (ret)
			goto err1;
	}

	pxa2xx_configure_sockets(&dev->dev, ops);
	dev_set_drvdata(&dev->dev, sinfo);

	return 0;

err1:
	while (--i >= 0)
		soc_pcmcia_remove_one(&sinfo->skt[i]);

err0:
	return ret;
}

static int pxa2xx_drv_pcmcia_remove(struct platform_device *dev)
{
	struct skt_dev_info *sinfo = platform_get_drvdata(dev);
	int i;

	for (i = 0; i < sinfo->nskt; i++)
		soc_pcmcia_remove_one(&sinfo->skt[i]);

	return 0;
}

static int pxa2xx_drv_pcmcia_resume(struct device *dev)
{
	struct pcmcia_low_level *ops = (struct pcmcia_low_level *)dev->platform_data;

	pxa2xx_configure_sockets(dev, ops);
	return 0;
}

static const struct dev_pm_ops pxa2xx_drv_pcmcia_pm_ops = {
	.resume		= pxa2xx_drv_pcmcia_resume,
};

static struct platform_driver pxa2xx_pcmcia_driver = {
	.probe		= pxa2xx_drv_pcmcia_probe,
	.remove		= pxa2xx_drv_pcmcia_remove,
	.driver		= {
		.name	= "pxa2xx-pcmcia",
		.pm	= &pxa2xx_drv_pcmcia_pm_ops,
	},
};

static int __init pxa2xx_pcmcia_init(void)
{
	return platform_driver_register(&pxa2xx_pcmcia_driver);
}

static void __exit pxa2xx_pcmcia_exit(void)
{
	platform_driver_unregister(&pxa2xx_pcmcia_driver);
}

fs_initcall(pxa2xx_pcmcia_init);
module_exit(pxa2xx_pcmcia_exit);

#endif

MODULE_AUTHOR("John Dorsey <john+@cs.cmu.edu>");
MODULE_DESCRIPTION("Linux PCMCIA Card Services: Common SoC support");
MODULE_LICENSE("Dual MPL/GPL");
