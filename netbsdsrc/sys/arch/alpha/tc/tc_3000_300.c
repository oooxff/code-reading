/* $NetBSD: tc_3000_300.c,v 1.15 1997/09/02 13:20:20 thorpej Exp $ */

/*
 * Copyright (c) 1994, 1995, 1996 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <sys/cdefs.h>			/* RCS ID & Copyright macro defns */

__KERNEL_RCSID(0, "$NetBSD: tc_3000_300.c,v 1.15 1997/09/02 13:20:20 thorpej Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <machine/autoconf.h>
#include <machine/pte.h>
#ifndef EVCNT_COUNTERS
#include <machine/intrcnt.h>
#endif

#include <dev/tc/tcvar.h>
#include <alpha/tc/tc_conf.h>
#include <alpha/tc/tc_3000_300.h>
#include <alpha/tc/ioasicreg.h>

void	tc_3000_300_intr_setup __P((void));
void	tc_3000_300_intr_establish __P((struct device *, void *,
	    tc_intrlevel_t, int (*)(void *), void *));
void	tc_3000_300_intr_disestablish __P((struct device *, void *));
void	tc_3000_300_iointr __P((void *, unsigned long));

int	tc_3000_300_intrnull __P((void *));

#define	C(x)	((void *)(u_long)x)
#define	KV(x)	(ALPHA_PHYS_TO_K0SEG(x))

/*
 * We have to read and modify the IOASIC registers directly, because
 * the TC option slot interrupt request and mask bits are stored there,
 * and the ioasic code isn't initted when we need to frob some interrupt
 * bits.
 */
#define	DEC_3000_300_IOASIC_ADDR	KV(0x1a0000000)

struct tc_slotdesc tc_3000_300_slots[] = {
	{ KV(0x100000000), C(TC_3000_300_DEV_OPT0), },	/* 0 - opt slot 0 */
	{ KV(0x120000000), C(TC_3000_300_DEV_OPT1), },	/* 1 - opt slot 1 */
	{ KV(0x180000000), C(TC_3000_300_DEV_BOGUS), },	/* 2 - TCDS ASIC */
	{ KV(0x1a0000000), C(TC_3000_300_DEV_BOGUS), },	/* 3 - IOCTL ASIC */
	{ KV(0x1c0000000), C(TC_3000_300_DEV_CXTURBO), }, /* 4 - CXTurbo */
};
int tc_3000_300_nslots =
    sizeof(tc_3000_300_slots) / sizeof(tc_3000_300_slots[0]);

struct tc_builtin tc_3000_300_builtins[] = {
	{ "PMAGB-BA",	4, 0x02000000, C(TC_3000_300_DEV_CXTURBO),	},
	{ "FLAMG-IO",	3, 0x00000000, C(TC_3000_300_DEV_IOASIC),	},
	{ "PMAZ-DS ",	2, 0x00000000, C(TC_3000_300_DEV_TCDS),		},
};
int tc_3000_300_nbuiltins =
    sizeof(tc_3000_300_builtins) / sizeof(tc_3000_300_builtins[0]);

struct tcintr {
	int	(*tci_func) __P((void *));
	void	*tci_arg;
} tc_3000_300_intr[TC_3000_300_NCOOKIES];

void
tc_3000_300_intr_setup()
{
	volatile u_int32_t *imskp;
	u_long i;

	/*
	 * Disable all interrupts that we can (can't disable builtins).
	 */
	imskp = (volatile u_int32_t *)IOASIC_REG_IMSK(DEC_3000_300_IOASIC_ADDR);
	*imskp &= ~(IOASIC_INTR_300_OPT0 | IOASIC_INTR_300_OPT1);

	/*
	 * Set up interrupt handlers.
	 */
	for (i = 0; i < TC_3000_300_NCOOKIES; i++) {
                tc_3000_300_intr[i].tci_func = tc_3000_300_intrnull;
                tc_3000_300_intr[i].tci_arg = (void *)i;
	}
}

void
tc_3000_300_intr_establish(tcadev, cookie, level, func, arg)
	struct device *tcadev;
	void *cookie, *arg;
	tc_intrlevel_t level;
	int (*func) __P((void *));
{
	volatile u_int32_t *imskp;
	u_long dev = (u_long)cookie;

#ifdef DIAGNOSTIC
	/* XXX bounds-check cookie. */
#endif

	if (tc_3000_300_intr[dev].tci_func != tc_3000_300_intrnull)
		panic("tc_3000_300_intr_establish: cookie %d twice", dev);

	tc_3000_300_intr[dev].tci_func = func;
	tc_3000_300_intr[dev].tci_arg = arg;

	imskp = (volatile u_int32_t *)IOASIC_REG_IMSK(DEC_3000_300_IOASIC_ADDR);
	switch (dev) {
	case TC_3000_300_DEV_OPT0:
		*imskp |= IOASIC_INTR_300_OPT0;
		break;
	case TC_3000_300_DEV_OPT1:
		*imskp |= IOASIC_INTR_300_OPT1;
		break;
	default:
		/* interrupts for builtins always enabled */
		break;
	}
}

void
tc_3000_300_intr_disestablish(tcadev, cookie)
	struct device *tcadev;
	void *cookie;
{
	volatile u_int32_t *imskp;
	u_long dev = (u_long)cookie;

#ifdef DIAGNOSTIC
	/* XXX bounds-check cookie. */
#endif

	if (tc_3000_300_intr[dev].tci_func == tc_3000_300_intrnull)
		panic("tc_3000_300_intr_disestablish: cookie %d bad intr",
		    dev);

	imskp = (volatile u_int32_t *)IOASIC_REG_IMSK(DEC_3000_300_IOASIC_ADDR);
	switch (dev) {
	case TC_3000_300_DEV_OPT0:
		*imskp &= ~IOASIC_INTR_300_OPT0;
		break;
	case TC_3000_300_DEV_OPT1:
		*imskp &= ~IOASIC_INTR_300_OPT1;
		break;
	default:
		/* interrupts for builtins always enabled */
		break;
	}

	tc_3000_300_intr[dev].tci_func = tc_3000_300_intrnull;
	tc_3000_300_intr[dev].tci_arg = (void *)dev;
}

int
tc_3000_300_intrnull(val)
	void *val;
{

	panic("tc_3000_300_intrnull: uncaught TC intr for cookie %ld\n",
	    (u_long)val);
}

void
tc_3000_300_iointr(framep, vec)
	void *framep;
	unsigned long vec;
{
	u_int32_t tcir, ioasicir, ioasicimr;
	int ifound;

#ifdef DIAGNOSTIC
	int s;
	if (vec != 0x800)
		panic("INVALID ASSUMPTION: vec 0x%lx, not 0x800", vec);
	s = splhigh();
	if (s != ALPHA_PSL_IPL_IO)
		panic("INVALID ASSUMPTION: IPL %d, not %d", s,
		    ALPHA_PSL_IPL_IO);
	splx(s);
#endif

	do {
		tc_syncbus();

		/* find out what interrupts/errors occurred */
		tcir = *(volatile u_int32_t *)TC_3000_300_IR;
		ioasicir = *(volatile u_int32_t *)
		    IOASIC_REG_INTR(DEC_3000_300_IOASIC_ADDR);
		ioasicimr = *(volatile u_int32_t *)
		    IOASIC_REG_IMSK(DEC_3000_300_IOASIC_ADDR);
		tc_mb();

		/* Ignore interrupts that aren't enabled out. */
		ioasicir &= ioasicimr;

		/* clear the interrupts/errors we found. */
		*(volatile u_int32_t *)TC_3000_300_IR = tcir;
		/* XXX can't clear TC option slot interrupts here? */
		tc_wmb();

		ifound = 0;

#ifdef EVCNT_COUNTERS
	/* No interrupt counting via evcnt counters */
	XXX BREAK HERE XXX
#else /* !EVCNT_COUNTERS */
#define	INCRINTRCNT(slot)	intrcnt[INTRCNT_KN16 + slot]++
#endif /* EVCNT_COUNTERS */

#define	CHECKINTR(slot, flag)						\
		if (flag) {						\
			ifound = 1;					\
			INCRINTRCNT(slot);				\
			(*tc_3000_300_intr[slot].tci_func)		\
			    (tc_3000_300_intr[slot].tci_arg);		\
		}
		/* Do them in order of priority; highest slot # first. */
		CHECKINTR(TC_3000_300_DEV_CXTURBO,
		    tcir & TC_3000_300_IR_CXTURBO);
		CHECKINTR(TC_3000_300_DEV_IOASIC,
		    (tcir & TC_3000_300_IR_IOASIC) &&
	            (ioasicir & ~(IOASIC_INTR_300_OPT1|IOASIC_INTR_300_OPT0)));
		CHECKINTR(TC_3000_300_DEV_TCDS, tcir & TC_3000_300_IR_TCDS);
		CHECKINTR(TC_3000_300_DEV_OPT1,
		    ioasicir & IOASIC_INTR_300_OPT1);
		CHECKINTR(TC_3000_300_DEV_OPT0,
		    ioasicir & IOASIC_INTR_300_OPT0);
#undef CHECKINTR

#ifdef DIAGNOSTIC
#define PRINTINTR(msg, bits)						\
	if (tcir & bits)						\
		printf(msg);
		PRINTINTR("BCache tag parity error\n",
		    TC_3000_300_IR_BCTAGPARITY);
		PRINTINTR("TC overrun error\n", TC_3000_300_IR_TCOVERRUN);
		PRINTINTR("TC I/O timeout\n", TC_3000_300_IR_TCTIMEOUT);
		PRINTINTR("Bcache parity error\n",
		    TC_3000_300_IR_BCACHEPARITY);
		PRINTINTR("Memory parity error\n", TC_3000_300_IR_MEMPARITY);
#undef PRINTINTR
#endif
	} while (ifound);
}
