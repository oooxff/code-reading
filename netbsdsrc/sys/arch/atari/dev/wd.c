/*	$NetBSD: wd.c,v 1.7 1997/10/08 23:41:18 thorpej Exp $	*/

/*
 * Copyright (c) 1994, 1995 Charles M. Hannum.  All rights reserved.
 *
 * DMA and multi-sector PIO handling are derived from code contributed by
 * Onno van der Linden.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Charles M. Hannum.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	from: wd.c,v 1.156 1997/01/17 20:45:29 perry Exp
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/disklabel.h>
#include <sys/disk.h>
#include <sys/syslog.h>
#include <sys/proc.h>

#include <vm/vm.h>

#include <machine/cpu.h>
#include <machine/bus.h>
#include <machine/iomap.h>
#include <machine/mfp.h>
#include <machine/dma.h>

#include <m68k/asm_single.h>

#include <atari/atari/device.h>

#include <atari/dev/ym2149reg.h>
#include <atari/dev/wdreg.h>
#include <atari/dev/wdvar.h>

#include "locators.h"

static int wdresethack = 1;		/* Leftover from arm32		 */

#define DRQUNK		-1		/* Leftover from ISA...		 */
#define	WAITTIME	(4 * hz)	/* time to wait for a completion */
#define	RECOVERYTIME	(hz / 2)	/* time to recover from an error */

#define WDCDELAY	100
#define WDCNDELAY	100000		/* delay = 100us; so 10s for a controller state change */
#if 0
/* If you enable this, it will report any delays more than 100us * N long. */
#define WDCNDELAY_DEBUG	10
#endif

#define	WDIORETRIES	5		/* number of retries before giving up */

#define	WDUNIT(dev)			DISKUNIT(dev)
#define	WDPART(dev)			DISKPART(dev)
#define	MAKEWDDEV(maj, unit, part)	MAKEDISKDEV(maj, unit, part)

#define	WDLABELDEV(dev)	(MAKEWDDEV(major(dev), WDUNIT(dev), RAW_PART))

struct wd_softc {
	struct device sc_dev;
	struct disk sc_dk;

	/* Information about the current transfer: */
	daddr_t sc_blkno;	/* starting block number */
	int sc_bcount;		/* byte count left */
	int sc_skip;		/* bytes already transferred */
	int sc_nblks;		/* number of blocks currently transferring */
	int sc_nbytes;		/* number of bytes currently transferring */

	/* Long-term state: */
	int sc_drive;			/* physical unit number */
	int sc_state;			/* control state */
#define	RECAL		0		/* recalibrate */
#define	RECAL_WAIT	1		/* done recalibrating */
#define	GEOMETRY	2		/* upload geometry */
#define	GEOMETRY_WAIT	3		/* done uploading geometry */
#define	MULTIMODE	4		/* set multiple mode */
#define	MULTIMODE_WAIT	5		/* done setting multiple mode */
#define	READY		6		/* ready for use */
	int sc_mode;			/* transfer mode */
#define	WDM_PIOSINGLE	0		/* single-sector PIO */
#define	WDM_PIOMULTI	1		/* multi-sector PIO */
#define	WDM_DMA		2		/* DMA */
	int sc_multiple;		/* multiple for WDM_PIOMULTI */
	int sc_flags;			/* drive characteistics found */
#define	WDF_LOCKED	0x01
#define	WDF_WANTED	0x02
#define	WDF_WLABEL	0x04		/* label is writable */
#define	WDF_LABELLING	0x08		/* writing label */
/* XXX Nothing resets this yet, but disk change sensing will when ATAPI is
   implemented. */
#define	WDF_LOADED	0x10		/* parameters loaded */
#define	WDF_32BIT	0x20		/* can do 32-bit transfer */

	struct wdparams sc_params;	/* ESDI/ATA drive parameters */
	daddr_t	sc_badsect[127];	/* 126 plus trailing -1 marker */

	TAILQ_ENTRY(wd_softc) sc_drivechain;
	struct buf sc_q;
};

int	wdcprobe 	__P((struct device *, struct cfdata *, void *));
void	wdcattach 	__P((struct device *, struct device *, void *));
int	wdcintr		__P((void *));

/*
 * Handle Falcon DMA locking.
 */
static int	ide_lock;

int	claimed_dma __P((void *, void *));
void	free_dma __P((void *));

extern __inline__ int
claimed_dma(softc, callback)
void	*softc, *callback;
{
	if (ide_lock != DMA_LOCK_GRANT) {
		if (ide_lock == DMA_LOCK_REQ) {
			/*
			 * DMA access is being claimed.
			 */
			return(0);
		}
		if (!st_dmagrab((dma_farg)wdcintr, (dma_farg)callback,
						softc, &ide_lock, 1))
			return(0);
	}
	return(1);
}

extern __inline__ void
free_dma(softc)
void	*softc;
{
	single_inst_bclr_b(MFP->mf_iprb, IB_DINT);

	/*
	 * Only free the lock on a Falcon. On the Hades, keep it.
	 */
	if (machineid & ATARI_FALCON)
		st_dmafree(softc, &ide_lock);
}

/*
 * XXX: Because of name-clashes with the wd-driver in files.isa, we
 *      call the driver idec/ide in the config files. As the source
 *	is copied from ISA/arm32, we like to keep the function names...
 *	This cruft should automagically disappear when a 'true' MI driver
 *	becomes available.
 */
#define wdc_ca	idec_ca
#define wdc_cd	idec_cd
#define wd_ca	ide_ca
#define wd_cd	ide_cd

struct cfattach wdc_ca = {
	sizeof(struct wdc_softc), wdcprobe, wdcattach
};

struct cfdriver wdc_cd = {
	NULL, "wdc", DV_DULL
};

int wdprobe	__P((struct device *, struct cfdata *, void *));
void wdattach	__P((struct device *, struct device *, void *));
int wdprint	__P((void *, const char *));

struct cfattach wd_ca = {
	sizeof(struct wd_softc), wdprobe, wdattach
};

struct cfdriver wd_cd = {
	NULL, "wd", DV_DISK
};

void	wdgetdefaultlabel __P((struct wd_softc *, struct disklabel *lp));
void	wdgetdisklabel	__P((struct wd_softc *));
int	wd_get_parms	__P((struct wd_softc *));
void	wdstrategy	__P((struct buf *));
void	wdstart		__P((struct wd_softc *));

struct dkdriver wddkdriver = { wdstrategy };

/* XXX: these should go elsewhere */
cdev_decl(wd);
bdev_decl(wd);

void	wdfinish	__P((struct wd_softc *, struct buf *));
int 	dcintr		__P((void *));
void	wdcstart	__P((struct wdc_softc *));
int	wdcommand	__P((struct wd_softc *, int, int, int, int, int));
int	wdcommandshort	__P((struct wdc_softc *, int, int));
int	wdcontrol	__P((struct wd_softc *));
int	wdsetctlr	__P((struct wd_softc *));
#if 0 /* LWP */
static void bad144intern __P((struct wd_softc *));
#endif
int	wdcreset	__P((struct wdc_softc *));
void	wdcrestart	__P((void *arg));
void	wdcunwedge	__P((struct wdc_softc *));
void	wdctimeout	__P((void *arg));
void	wderror		__P((void *, struct buf *, char *));
int	wdcwait		__P((struct wdc_softc *, int));
int	wdlock		__P((struct wd_softc *));
void	wdunlock	__P((struct wd_softc *));

/* ST506 spec says that if READY or SEEKCMPLT go off, then the read or write
   command is aborted. */
#define	wait_for_drq(d)		wdcwait(d, WDCS_DRDY | WDCS_DSC | WDCS_DRQ)
#define	wait_for_ready(d)	wdcwait(d, WDCS_DRDY | WDCS_DSC)
#define	wait_for_unbusy(d)	wdcwait(d, 0)

int
wdcprobe_internal(iot, ioh, aux_ioh, data_ioh, data32_ioh, name)
	bus_space_tag_t		iot;
	bus_space_handle_t	ioh;
	bus_space_handle_t	aux_ioh;
	bus_space_handle_t	data_ioh;
	bus_space_handle_t	data32_ioh;
	char *name;
{
	struct wdc_softc wdc;

	/* fake a wdc structure for the benefit of the routines called */
	
	wdc.sc_iot = iot;
	wdc.sc_ioh = ioh;
	wdc.sc_auxioh = aux_ioh;
	wdc.sc_dataioh = data_ioh;
	wdc.sc_data32ioh = data32_ioh;
	wdc.sc_inten = NULL;
	wdc.sc_flags = WDCF_QUIET;	/* Supress warning during probe */
	strcpy(wdc.sc_dev.dv_xname, name);

	/* Check if we have registers that work. */
	bus_space_write_1(iot, ioh, wd_error, 0x5a);	/* Error register not writable, */
	bus_space_write_1(iot, ioh, wd_cyl_lo, 0xa5);	/* but all of cyllo are. */
	if (bus_space_read_1(iot, ioh, wd_error) == 0x5a
	    || bus_space_read_1(iot, ioh, wd_cyl_lo) != 0xa5)
		return(0);

	if (wdcreset(&wdc) != 0) {
		delay(500000);
		if (wdcreset(&wdc) != 0)
			return(0);
	}

	/* Select drive 0. */
	bus_space_write_1(iot, ioh, wd_sdh, WDSD_IBM | 0);

	/* Wait for controller to become ready. */
	if (wait_for_unbusy(&wdc) < 0)
		return(0);
    
	/* Start drive diagnostics. */
	bus_space_write_1(iot, ioh, wd_command, WDCC_DIAGNOSE);

	/* Wait for command to complete. */
	if (wait_for_unbusy(&wdc) < 0)
		return(0);

	return(1);
}

int
wdcprobe(parent, cfp, aux)
	struct device	*parent;
	struct cfdata	*cfp;
	void		*aux;
{
	bus_space_handle_t	ioh;
	u_char			sv_ierb;
	int			rv = 0;

	if((machineid & ATARI_TT) || strcmp("wdc", aux) || cfp->cf_unit != 0)
		return(0);
	if (!atari_realconfig)
		return 0;

	if (bus_space_map(NULL, 0xfff00000, 0x40, 0, &ioh))
		return(0);

	/*
	 * Make sure IDE interrupts are disabled during probing.
	 */
	sv_ierb = MFP->mf_ierb;
	MFP->mf_ierb &= ~IB_DINT;

	/*
	 * Make sure that IDE is turned on on the Falcon.
	 */
	if(machineid & ATARI_FALCON)
		ym2149_ser2(0);

	rv = wdcprobe_internal(NULL, ioh, ioh, ioh, ioh,
				cfp->cf_driver->cd_name);

	bus_space_unmap(NULL, ioh, 8);
	MFP->mf_ierb = sv_ierb;

	return(rv);
}

struct wdc_attach_args {
	int wa_drive;
	int wa_cf_flags;
};

/*
 * Flags that might be passed from 'config'
 */
#define	WACF_NOMULTI	0x01		/* No multi-sector comands	*/

int
wdprint(aux, wdc)
	void *aux;
	const char *wdc;
{
	struct wdc_attach_args *wa = aux;

	if (!wdc)
		printf(" drive %d", wa->wa_drive);
	return QUIET;
}

void
wdcattach_internal(wdc, iot, ioh, aux_ioh, data_ioh, data32_ioh, drq)
	struct wdc_softc *wdc;
	bus_space_tag_t iot;
	bus_space_handle_t ioh;
	bus_space_handle_t aux_ioh;
	bus_space_handle_t data_ioh;
	bus_space_handle_t data32_ioh;
	int drq;
{
	struct wdc_attach_args wa;

	TAILQ_INIT(&wdc->sc_drives);
	wdc->sc_drq = drq;
	wdc->sc_iot = iot;
	wdc->sc_ioh = ioh;
	wdc->sc_auxioh = aux_ioh;
	wdc->sc_dataioh = data_ioh;
	wdc->sc_data32ioh = data32_ioh;
	wdc->sc_inten = NULL;

	printf("\n");

	for (wa.wa_drive = 0; wa.wa_drive < 2; wa.wa_drive++)
		(void)config_found((struct device *)wdc, (void *)&wa, wdprint);
}

void
wdcattach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct wdc_softc *wdc = (void *)self;
	bus_space_tag_t iot = NULL;
	bus_space_handle_t ioh;

	TAILQ_INIT(&wdc->sc_drives);

	if (bus_space_map(iot, 0xfff00000, 0x40, 0, &ioh))
		panic("%s: Cannot map IO\n", wdc->sc_dev.dv_xname);

	/*
	 * Play a nasty trick here. Normally we only manipulate the
	 * interrupt *mask*. However to defeat wd_get_parms(), we
	 * disable the interrupts here using the *enable* register.
	 */
	MFP->mf_ierb &= ~IB_DINT;

	wdcattach_internal(wdc, iot, ioh, ioh, ioh, ioh, DRQUNK);

	/*
	 * Setup & enable disk related interrupts.
	 */
	MFP->mf_ierb  |= IB_DINT;
	MFP->mf_iprb  &= ~IB_DINT;
	MFP->mf_imrb  |= IB_DINT;
}

int
wdprobe(parent, cf, aux)
	struct device	*parent;
	struct cfdata	*cf;
	void		*aux;
{
	struct wdc_softc *wdc = (void *)parent;
	struct wdc_attach_args *wa = aux;
	int drive = wa->wa_drive;

	if (cf->cf_loc[IDECCF_DRIVE] != IDECCF_DRIVE_DEFAULT &&
	    cf->cf_loc[IDECCF_DRIVE] != drive)
		return 0;

	/*
	 * Get the flags from 'config' a byte for each drive
	 */
	wa->wa_cf_flags = (cf->cf_loc[1] >> (drive * 8)) & 0xff;
wa->wa_cf_flags = 0;
	
	if (wdcommandshort(wdc, drive, WDCC_RECAL) != 0 ||
	    wait_for_ready(wdc) != 0)
		return 0;

	return 1;
}

void
wdattach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct wd_softc *wd = (void *)self;
	struct wdc_softc *wdc = (void *)parent;
	struct wdc_attach_args *wa = aux;
	int i, blank;
	char buf[41], c, *p, *q;

	wd->sc_drive = wa->wa_drive;

	/*
	 * Initialize and attach the disk structure.
	 */
	wd->sc_flags = ((wdc->sc_flags & WDCF_32BIT) ? WDF_32BIT : 0);
	wd->sc_dk.dk_driver = &wddkdriver;
	wd->sc_dk.dk_name = wd->sc_dev.dv_xname;
	disk_attach(&wd->sc_dk);

	wd_get_parms(wd);
	for (blank = 0, p = wd->sc_params.wdp_model, q = buf, i = 0;
	     i < sizeof(wd->sc_params.wdp_model); i++) {
		c = *p++;
		if (c == '\0')
			break;
		if (c != ' ') {
			if (blank) {
				*q++ = ' ';
				blank = 0;
			}
			*q++ = c;
		} else
			blank = 1;
	}
	*q++ = '\0';

	printf(": <%s>\n%s: %dMB, %d cyl, %d head, %d sec, %d bytes/sec\n",
	    buf, wd->sc_dev.dv_xname,
	    wd->sc_params.wdp_cylinders *
	      (wd->sc_params.wdp_heads * wd->sc_params.wdp_sectors) /
	      (1048576 / DEV_BSIZE),
	    wd->sc_params.wdp_cylinders,
	    wd->sc_params.wdp_heads,
	    wd->sc_params.wdp_sectors,
	    DEV_BSIZE);

	if ((wd->sc_params.wdp_capabilities & WD_CAP_DMA) != 0 &&
	    wdc->sc_drq != DRQUNK) {
		wd->sc_mode = WDM_DMA;
	} else if ((wd->sc_params.wdp_maxmulti > 1) &&
			!(wa->wa_cf_flags & WACF_NOMULTI)) {
		wd->sc_mode = WDM_PIOMULTI;
		wd->sc_multiple = min(wd->sc_params.wdp_maxmulti, 16);
	} else {
		wd->sc_mode = WDM_PIOSINGLE;
		wd->sc_multiple = 1;
	}

	printf("%s: using", wd->sc_dev.dv_xname);
	if (wd->sc_mode == WDM_DMA)
		printf(" dma transfers,");
	else
		printf(" %d-sector %d-bit pio transfers,",
		    wd->sc_multiple, (wd->sc_flags & WDF_32BIT) == 0 ? 16 : 32);
	if ((wd->sc_params.wdp_capabilities & WD_CAP_LBA) != 0)
		printf(" lba addressing\n");
	else
		printf(" chs addressing\n");
}

/*
 * Read/write routine for a buffer.  Validates the arguments and schedules the
 * transfer.  Does not wait for the transfer to complete.
 */
void
wdstrategy(bp)
	struct buf *bp;
{
	struct wd_softc *wd = wd_cd.cd_devs[WDUNIT(bp->b_dev)];
	int s;
    
	/* Valid request?  */
	if (bp->b_blkno < 0 ||
	    (bp->b_bcount % wd->sc_dk.dk_label->d_secsize) != 0 ||
	    (bp->b_bcount / wd->sc_dk.dk_label->d_secsize) >= (1 << NBBY)) {
		bp->b_error = EINVAL;
		goto bad;
	}
    
	/* If device invalidated (e.g. media change, door open), error. */
	if ((wd->sc_flags & WDF_LOADED) == 0) {
		bp->b_error = EIO;
		goto bad;
	}

	/* If it's a null transfer, return immediately. */
	if (bp->b_bcount == 0)
		goto done;

	/*
	 * Do bounds checking, adjust transfer. if error, process.
	 * If end of partition, just return.
	 */
	if (WDPART(bp->b_dev) != RAW_PART &&
	    bounds_check_with_label(bp, wd->sc_dk.dk_label,
	    (wd->sc_flags & (WDF_WLABEL|WDF_LABELLING)) != 0) <= 0)
		goto done;
    
	/* Queue transfer on drive, activate drive and controller if idle. */
	s = splbio();
	disksort(&wd->sc_q, bp);
	if (!wd->sc_q.b_active)
		wdstart(wd);
#if 0
	else {
		struct wdc_softc *wdc = (void *)wd->sc_dev.dv_parent;
		if ((wdc->sc_flags & (WDCF_ACTIVE|WDCF_ERROR)) == 0) {
			printf("wdstrategy: controller inactive\n");
			wdcstart(wdc);
		}
	}
#endif
	splx(s);
	return;
    
bad:
	bp->b_flags |= B_ERROR;
done:
	/* Toss transfer; we're done early. */
	bp->b_resid = bp->b_bcount;
	biodone(bp);
}

/*
 * Queue a drive for I/O.
 */
void
wdstart(wd)
	struct wd_softc *wd;
{
	struct wdc_softc *wdc = (void *)wd->sc_dev.dv_parent;
	int active = wdc->sc_drives.tqh_first != 0;

	/* Link onto controller queue. */
	wd->sc_q.b_active = 1;
	TAILQ_INSERT_TAIL(&wdc->sc_drives, wd, sc_drivechain);

	disk_busy(&wd->sc_dk);
    
	/* If controller not already active, start it. */
	if (!active)
		wdcstart(wdc);
}

/*
 * Finish an I/O operation.  Clean up the drive and controller state, set the
 * residual count, and inform the upper layers that the operation is complete.
 */
void
wdfinish(wd, bp)
	struct wd_softc *wd;
	struct buf *bp;
{
	struct wdc_softc *wdc = (void *)wd->sc_dev.dv_parent;

	free_dma(wdc);

	wdc->sc_flags &= ~(WDCF_SINGLE | WDCF_ERROR);
	wdc->sc_errors = 0;
	/*
	 * Move this drive to the end of the queue to give others a `fair'
	 * chance.
	 */
	if (wd->sc_drivechain.tqe_next) {
		TAILQ_REMOVE(&wdc->sc_drives, wd, sc_drivechain);
		if (bp->b_actf) {
			TAILQ_INSERT_TAIL(&wdc->sc_drives, wd, sc_drivechain);
		} else
			wd->sc_q.b_active = 0;
	}
	bp->b_resid = wd->sc_bcount;
	wd->sc_skip = 0;
	wd->sc_q.b_actf = bp->b_actf;

	disk_unbusy(&wd->sc_dk, (bp->b_bcount - bp->b_resid));

	if (!wd->sc_q.b_actf) {
		TAILQ_REMOVE(&wdc->sc_drives, wd, sc_drivechain);
		wd->sc_q.b_active = 0;
	} else
		disk_busy(&wd->sc_dk);

	biodone(bp);
}

int
wdread(dev, uio, flags)
	dev_t dev;
	struct uio *uio;
	int flags;
{

	return (physio(wdstrategy, NULL, dev, B_READ, minphys, uio));
}

int
wdwrite(dev, uio, flags)
	dev_t dev;
	struct uio *uio;
	int flags;
{

	return (physio(wdstrategy, NULL, dev, B_WRITE, minphys, uio));
}

/*
 * Start I/O on a controller.  This does the calculation, and starts a read or
 * write operation.  Called to from wdstart() to start a transfer, from
 * wdcintr() to continue a multi-sector transfer or start the next transfer, or
 * wdcrestart() after recovering from an error.
 */
void
wdcstart(wdc)
	struct wdc_softc *wdc;
{
	struct wd_softc *wd;
	struct buf *bp;
	struct disklabel *lp;
	int nblks;

#ifdef DIAGNOSTIC
	if ((wdc->sc_flags & WDCF_ACTIVE) != 0)
		panic("wdcstart: controller still active");
#endif

	/*
	 * XXX
	 * This is a kluge.  See comments in wd_get_parms().
	 */
	if ((wdc->sc_flags & WDCF_WANTED) != 0) {
		wdc->sc_flags &= ~WDCF_WANTED;
		wakeup(wdc);
		return;
	}

loop:
	/* Is there a drive for the controller to do a transfer with? */
	wd = wdc->sc_drives.tqh_first;
	if (wd == NULL)
		return;
    
	/* Is there a transfer to this drive?  If not, deactivate drive. */
	bp = wd->sc_q.b_actf;
    
	if (wdc->sc_errors >= WDIORETRIES) {
		wderror(wd, bp, "wdcstart hard error");
		bp->b_error = EIO;
		bp->b_flags |= B_ERROR;
		wdfinish(wd, bp);
		goto loop;
	}

	if (!claimed_dma(wdc, wdcstart))
		return;

	/* Do control operations specially. */
	if (wd->sc_state < READY) {
		/*
		 * Actually, we want to be careful not to mess with the control
		 * state if the device is currently busy, but we can assume
		 * that we never get to this point if that's the case.
		 */
		if (wdcontrol(wd) == 0) {
			/* The drive is busy.  Wait. */
			return;
		}
	}

	/*
	 * WDCF_ERROR is set by wdcunwedge() and wdcintr() when an error is
	 * encountered.  If we are in multi-sector mode, then we switch to
	 * single-sector mode and retry the operation from the start.
	 */
	if (wdc->sc_flags & WDCF_ERROR) {
		wdc->sc_flags &= ~WDCF_ERROR;
		if ((wdc->sc_flags & WDCF_SINGLE) == 0) {
			wdc->sc_flags |= WDCF_SINGLE;
			wd->sc_skip = 0;
		}
	}

	lp = wd->sc_dk.dk_label;

	/* When starting a transfer... */
	if (wd->sc_skip == 0) {
		int part = WDPART(bp->b_dev);
		daddr_t blkno;

#ifdef WDDEBUG
		printf("\n%s: wdcstart %s %ld@%d; map ", wd->sc_dev.dv_xname,
		    (bp->b_flags & B_READ) ? "read" : "write", bp->b_bcount,
		    bp->b_blkno);
#endif
		wd->sc_bcount = bp->b_bcount;
		blkno = bp->b_blkno;
		if (part != RAW_PART)
			blkno += lp->d_partitions[part].p_offset;
		wd->sc_blkno = blkno / (lp->d_secsize / DEV_BSIZE);
	} else {
#ifdef WDDEBUG
		printf(" %d)%x", wd->sc_skip, bus_space_read_1(wdc->sc_iot,
		    wdc->sc_auxioh, wd_altsts));
#endif
	}

	/* When starting a multi-sector transfer, or doing single-sector
	    transfers... */
	if (wd->sc_skip == 0 || (wdc->sc_flags & WDCF_SINGLE) != 0 ||
	    wd->sc_mode == WDM_DMA) {
		daddr_t blkno = wd->sc_blkno;
		long cylin, head, sector;
		int command;

		if ((wdc->sc_flags & WDCF_SINGLE) != 0)
			nblks = 1;
		else if (wd->sc_mode != WDM_DMA)
			nblks = wd->sc_bcount / lp->d_secsize;
		else
			nblks = min(wd->sc_bcount / lp->d_secsize, 8);

		/* Check for bad sectors and adjust transfer, if necessary. */
		if ((lp->d_flags & D_BADSECT) != 0
#ifdef B_FORMAT
		    && (bp->b_flags & B_FORMAT) == 0
#endif
		    ) {
			long blkdiff;
			int i;

			for (i = 0; (blkdiff = wd->sc_badsect[i]) != -1; i++) {
				blkdiff -= blkno;
				if (blkdiff < 0)
					continue;
				if (blkdiff == 0) {
					/* Replace current block of transfer. */
					blkno =
					    lp->d_secperunit - lp->d_nsectors - i - 1;
				}
				if (blkdiff < nblks) {
					/* Bad block inside transfer. */
					wdc->sc_flags |= WDCF_SINGLE;
					nblks = 1;
				}
				break;
			}
			/* Tranfer is okay now. */
		}

		if ((wd->sc_params.wdp_capabilities & WD_CAP_LBA) != 0) {
			sector = (blkno >> 0) & 0xff;
			cylin = (blkno >> 8) & 0xffff;
			head = (blkno >> 24) & 0xf;
			head |= WDSD_LBA;
		} else {
			sector = blkno % lp->d_nsectors;
			sector++;	/* Sectors begin with 1, not 0. */
			blkno /= lp->d_nsectors;
			head = blkno % lp->d_ntracks;
			blkno /= lp->d_ntracks;
			cylin = blkno;
			head |= WDSD_CHS;
		}

		if (wd->sc_mode == WDM_PIOSINGLE ||
		    (wdc->sc_flags & WDCF_SINGLE) != 0)
			wd->sc_nblks = 1;
		else if (wd->sc_mode == WDM_PIOMULTI)
			wd->sc_nblks = min(nblks, wd->sc_multiple);
		else
			wd->sc_nblks = nblks;
		wd->sc_nbytes = wd->sc_nblks * lp->d_secsize;
    
#ifdef B_FORMAT
		if (bp->b_flags & B_FORMAT) {
			sector = lp->d_gap3;
			nblks = lp->d_nsectors;
			command = WDCC_FORMAT;
		} else
#endif
		switch (wd->sc_mode) {
		case WDM_DMA:
#if 0
			command = (bp->b_flags & B_READ) ?
			    WDCC_READDMA : WDCC_WRITEDMA;
			/* Start the DMA channel and bounce the buffer if
			   necessary. */
			isa_dmastart(
			    bp->b_flags & B_READ ? DMAMODE_READ : DMAMODE_WRITE,
			    bp->b_data + wd->sc_skip,
			    wd->sc_nbytes, wdc->sc_drq);
#endif
			panic("wd cannot do DMA yet\n");
			break;
		case WDM_PIOMULTI:
			command = (bp->b_flags & B_READ) ?
			    WDCC_READMULTI : WDCC_WRITEMULTI;
			break;
		case WDM_PIOSINGLE:
			command = (bp->b_flags & B_READ) ?
			    WDCC_READ : WDCC_WRITE;
			break;
		default:
#ifdef DIAGNOSTIC
			panic("bad wd mode");
#endif
			return;
		}
	
		/* Initiate command! */
		if (wdcommand(wd, command, cylin, head, sector, nblks) != 0) {
			wderror(wd, NULL,
			    "wdcstart: timeout waiting for unbusy");
			wdcunwedge(wdc);
			return;
		}

#ifdef WDDEBUG
		printf("sector %ld cylin %ld head %ld addr %p sts %x\n",
		    sector, cylin, head, bp->b_data,
		    bus_space_read_1(wdc->sc_iot, wdc->sc_auxioh, wd_altsts));
#endif
	} else if (wd->sc_nblks > 1) {
		/* The number of blocks in the last stretch may be smaller. */
		nblks = wd->sc_bcount / lp->d_secsize;
		if (wd->sc_nblks > nblks) {
			wd->sc_nblks = nblks;
			wd->sc_nbytes = wd->sc_bcount;
		}
	}

	/* If this was a write and not using DMA, push the data. */
	if (wd->sc_mode != WDM_DMA &&
	    (bp->b_flags & (B_READ|B_WRITE)) == B_WRITE) {
		if (wait_for_drq(wdc) < 0) {
			wderror(wd, NULL, "wdcstart: timeout waiting for drq");
			wdcunwedge(wdc);
			return;
		}

		/* Push out data. */
		if ((wd->sc_flags & WDF_32BIT) == 0)
			bus_space_write_multi_2(wdc->sc_iot, wdc->sc_dataioh,
			    wd_data, (u_int16_t *)(bp->b_data + wd->sc_skip),
			    wd->sc_nbytes >> 1);
		else
			bus_space_write_multi_4(wdc->sc_iot, wdc->sc_data32ioh,
			    wd_data, (u_int32_t *)(bp->b_data + wd->sc_skip),
			    wd->sc_nbytes >> 2);
	}

	wdc->sc_flags |= WDCF_ACTIVE;
	if (wdc->sc_inten) wdc->sc_inten(wdc, 1);
	timeout(wdctimeout, wdc, WAITTIME);
}

/*
 * Interrupt routine for the controller.  Acknowledge the interrupt, check for
 * errors on the current operation, mark it done if necessary, and start the
 * next request.  Also check for a partially done transfer, and continue with
 * the next chunk if so.
 */
int
wdcintr(arg)
	void *arg;
{
	struct wdc_softc *wdc = arg;
	struct wd_softc *wd;
	struct buf *bp;

	if ((wdc->sc_flags & WDCF_ACTIVE) == 0) {
		/* Clear the pending interrupt and abort. */
		(void) bus_space_read_1(wdc->sc_iot, wdc->sc_ioh, wd_status);
		return 0;
	}

	wdc->sc_flags &= ~WDCF_ACTIVE;
	if (wdc->sc_inten) wdc->sc_inten(wdc, 0);
	untimeout(wdctimeout, wdc);

	wd = wdc->sc_drives.tqh_first;
	bp = wd->sc_q.b_actf;

#ifdef WDDEBUG
	printf("I%d ", wdc->sc_dev.dv_unit);
#endif

	if (wait_for_unbusy(wdc) < 0) {
		wderror(wd, NULL, "wdcintr: timeout waiting for unbusy");
		wdc->sc_status |= WDCS_ERR;	/* XXX */
	}
    
	/* Is it not a transfer, but a control operation? */
	if (wd->sc_state < READY) {
		if (wdcontrol(wd) == 0) {
			/* The drive is busy.  Wait. */
			return 1;
		}
		wdcstart(wdc);
		return 1;
	}

	/* Turn off the DMA channel and unbounce the buffer. */
	if (wd->sc_mode == WDM_DMA)
#if 0
		isa_dmadone(bp->b_flags & B_READ ? DMAMODE_READ : DMAMODE_WRITE,
		    bp->b_data + wd->sc_skip, wd->sc_nbytes, wdc->sc_drq);
#else
		panic("wd cannot do DMA yet\n");
#endif
	/* Have we an error? */
	if (wdc->sc_status & WDCS_ERR) {
#ifdef WDDEBUG
		wderror(wd, NULL, "wdcintr");
#endif
		if ((wdc->sc_flags & WDCF_SINGLE) == 0) {
			wdc->sc_flags |= WDCF_ERROR;
			goto restart;
		}

#ifdef B_FORMAT
		if (bp->b_flags & B_FORMAT)
			goto bad;
#endif

		if (++wdc->sc_errors < WDIORETRIES) {
			if (wdc->sc_errors == (WDIORETRIES + 1) / 2) {
#if 0
				wderror(wd, NULL, "wedgie");
#endif
				wdcunwedge(wdc);
				return 1;
			}
			goto restart;
		}
		wderror(wd, bp, "wdcintr hard error");
#ifdef B_FORMAT
	bad:
#endif
		bp->b_error = EIO;
		bp->b_flags |= B_ERROR;
		goto done;
	}

	/* If this was a read and not using DMA, fetch the data. */
	if (wd->sc_mode != WDM_DMA &&
	    (bp->b_flags & (B_READ|B_WRITE)) == B_READ) {
		if ((wdc->sc_status & (WDCS_DRDY | WDCS_DSC | WDCS_DRQ))
		    != (WDCS_DRDY | WDCS_DSC | WDCS_DRQ)) {
			wderror(wd, NULL, "wdcintr: read intr before drq");
			wdcunwedge(wdc);
			return 1;
		}

		/* Pull in data. */
		if ((wd->sc_flags & WDF_32BIT) == 0)
			bus_space_read_multi_2(wdc->sc_iot, wdc->sc_dataioh,
			    wd_data, (u_int16_t *)(bp->b_data + wd->sc_skip), 
			    wd->sc_nbytes >> 1);
		else
			bus_space_read_multi_4(wdc->sc_iot, wdc->sc_data32ioh,
			    wd_data, (u_int32_t *)(bp->b_data + wd->sc_skip), 
			    wd->sc_nbytes >> 2);
	}
    
	/* If we encountered any abnormalities, flag it as a soft error. */
	if (wdc->sc_errors > 0 ||
	    (wdc->sc_status & WDCS_CORR) != 0) {
		wderror(wd, bp, "soft error (corrected)");
		wdc->sc_errors = 0;
	}
    
	/* Adjust pointers for the next block, if any. */
	wd->sc_blkno += wd->sc_nblks;
	wd->sc_skip += wd->sc_nbytes;
	wd->sc_bcount -= wd->sc_nbytes;

	/* See if this transfer is complete. */
	if (wd->sc_bcount > 0)
		goto restart;

done:
	/* Done with this transfer, with or without error. */
	wdfinish(wd, bp);

restart:
	/* Start the next operation, if any. */
	wdcstart(wdc);

	return 1;
}

/*
 * Wait interruptibly for an exclusive lock.
 *
 * XXX
 * Several drivers do this; it should be abstracted and made MP-safe.
 */
int
wdlock(wd)
	struct wd_softc *wd;
{
	int error;

	while ((wd->sc_flags & WDF_LOCKED) != 0) {
		wd->sc_flags |= WDF_WANTED;
		if ((error = tsleep(wd, PRIBIO | PCATCH, "wdlck", 0)) != 0)
			return error;
	}
	wd->sc_flags |= WDF_LOCKED;
	return 0;
}

/*
 * Unlock and wake up any waiters.
 */
void
wdunlock(wd)
	struct wd_softc *wd;
{

	wd->sc_flags &= ~WDF_LOCKED;
	if ((wd->sc_flags & WDF_WANTED) != 0) {
		wd->sc_flags &= ~WDF_WANTED;
		wakeup(wd);
	}
}

int
wdopen(dev, flag, fmt, p)
	dev_t dev;
	int flag, fmt;
	struct proc *p;
{
	struct wd_softc *wd;
	int unit, part;
	int error;
    
	unit = WDUNIT(dev);
	if (unit >= wd_cd.cd_ndevs)
		return ENXIO;
	wd = wd_cd.cd_devs[unit];
	if (wd == 0)
		return ENXIO;
    
	if ((error = wdlock(wd)) != 0)
		return error;

	if (wd->sc_dk.dk_openmask != 0) {
		/*
		 * If any partition is open, but the disk has been invalidated,
		 * disallow further opens.
		 */
		if ((wd->sc_flags & WDF_LOADED) == 0) {
			error = EIO;
			goto bad3;
		}
	} else {
		if ((wd->sc_flags & WDF_LOADED) == 0) {
			wd->sc_flags |= WDF_LOADED;

			/* Load the physical device parameters. */
			if (wd_get_parms(wd) != 0) {
				error = ENXIO;
				goto bad2;
			}

			/* Load the partition info if not already loaded. */
			wdgetdisklabel(wd);
		}
	}

	part = WDPART(dev);

	/* Check that the partition exists. */
	if (part != RAW_PART &&
	    (part >= wd->sc_dk.dk_label->d_npartitions ||
	     wd->sc_dk.dk_label->d_partitions[part].p_fstype == FS_UNUSED)) {
		error = ENXIO;
		goto bad;
	}
    
	/* Insure only one open at a time. */
	switch (fmt) {
	case S_IFCHR:
		wd->sc_dk.dk_copenmask |= (1 << part);
		break;
	case S_IFBLK:
		wd->sc_dk.dk_bopenmask |= (1 << part);
		break;
	}
	wd->sc_dk.dk_openmask = wd->sc_dk.dk_copenmask | wd->sc_dk.dk_bopenmask;

	wdunlock(wd);
	return 0;

bad2:
	wd->sc_flags &= ~WDF_LOADED;

bad:
	if (wd->sc_dk.dk_openmask == 0) {
	}

bad3:
	wdunlock(wd);
	return error;
}

int
wdclose(dev, flag, fmt, p)
	dev_t dev;
	int flag, fmt;
	struct proc *p;
{
	struct wd_softc *wd = wd_cd.cd_devs[WDUNIT(dev)];
	int part = WDPART(dev);
	int error;
    
	if ((error = wdlock(wd)) != 0)
		return error;

	switch (fmt) {
	case S_IFCHR:
		wd->sc_dk.dk_copenmask &= ~(1 << part);
		break;
	case S_IFBLK:
		wd->sc_dk.dk_bopenmask &= ~(1 << part);
		break;
	}
	wd->sc_dk.dk_openmask = wd->sc_dk.dk_copenmask | wd->sc_dk.dk_bopenmask;

	if (wd->sc_dk.dk_openmask == 0) {
		/* XXXX Must wait for I/O to complete! */
	}

	wdunlock(wd);
	return 0;
}

void
wdgetdefaultlabel(wd, lp)
	struct wd_softc *wd;
	struct disklabel *lp;
{
	int	type;

	/*
	 * Preserve type in case the label is faked...
	 */
	type = lp->d_type;

	bzero(lp, sizeof(struct disklabel));

	lp->d_secsize = DEV_BSIZE;
	lp->d_ntracks = wd->sc_params.wdp_heads;
	lp->d_nsectors = wd->sc_params.wdp_sectors;
	lp->d_ncylinders = wd->sc_params.wdp_cylinders;
	lp->d_secpercyl = lp->d_ntracks * lp->d_nsectors;

#if 0
	strncpy(lp->d_typename, "ST506 disk", 16);
	lp->d_type = DTYPE_ST506;
#else
	if (type == DTYPE_ST506)
		strncpy(lp->d_typename, "ST506 disk", 16);
	else if (type == DTYPE_ESDI)
		strncpy(lp->d_typename, "ESDI/IDE", 16);
	lp->d_type = type;
#endif
	strncpy(lp->d_packname, wd->sc_params.wdp_model, 16);
	lp->d_secperunit = lp->d_secpercyl * lp->d_ncylinders;
	lp->d_rpm = 3600;
	lp->d_interleave = 1;
	lp->d_flags = 0;

	lp->d_partitions[RAW_PART].p_offset = 0;
	lp->d_partitions[RAW_PART].p_size =
	    lp->d_secperunit * (lp->d_secsize / DEV_BSIZE);
	lp->d_partitions[RAW_PART].p_fstype = FS_UNUSED;
	lp->d_npartitions = RAW_PART + 1;

	lp->d_magic = DISKMAGIC;
	lp->d_magic2 = DISKMAGIC;
	lp->d_checksum = dkcksum(lp);
}

/*
 * Fabricate a default disk label, and try to read the correct one.
 */
void
wdgetdisklabel(wd)
	struct wd_softc *wd;
{
	struct disklabel *lp = wd->sc_dk.dk_label;
	char *errstring;

	bzero(wd->sc_dk.dk_cpulabel, sizeof(struct cpu_disklabel));

	wdgetdefaultlabel(wd, lp);

	wd->sc_badsect[0] = -1;

	if (wd->sc_state > RECAL)
		wd->sc_state = RECAL;
	errstring = readdisklabel(MAKEWDDEV(0, wd->sc_dev.dv_unit, RAW_PART),
	    wdstrategy, lp, wd->sc_dk.dk_cpulabel);
	if (errstring) {
		/*
		 * This probably happened because the drive's default
		 * geometry doesn't match the DOS geometry.  We
		 * assume the DOS geometry is now in the label and try
		 * again.  XXX This is a kluge.
		 */
		if (wd->sc_state > GEOMETRY)
			wd->sc_state = GEOMETRY;
		errstring = readdisklabel(MAKEWDDEV(0, wd->sc_dev.dv_unit, RAW_PART),
		    wdstrategy, lp, wd->sc_dk.dk_cpulabel);
	}
	if (errstring) {
		printf("%s: %s\n", wd->sc_dev.dv_xname, errstring);
		return;
	}

	if (wd->sc_state > GEOMETRY)
		wd->sc_state = GEOMETRY;
#if 0 /* LWP */
	if ((lp->d_flags & D_BADSECT) != 0)
		bad144intern(wd);
#endif
}

/*
 * Implement operations needed before read/write.
 * Returns 0 if operation still in progress, 1 if completed.
 */
int
wdcontrol(wd)
	struct wd_softc *wd;
{
	struct wdc_softc *wdc = (void *)wd->sc_dev.dv_parent;
    
	switch (wd->sc_state) {
	case RECAL:			/* Set SDH, step rate, do recal. */
		if (wdcommandshort(wdc, wd->sc_drive, WDCC_RECAL) != 0) {
			wderror(wd, NULL, "wdcontrol: recal failed (1)");
			goto bad;
		}
		wd->sc_state = RECAL_WAIT;
		break;

	case RECAL_WAIT:
		if (wdc->sc_status & WDCS_ERR) {
			wderror(wd, NULL, "wdcontrol: recal failed (2)");
			goto bad;
		}
		/* fall through */
	case GEOMETRY:
		if ((wd->sc_params.wdp_capabilities & WD_CAP_LBA) != 0)
			goto multimode;
		if (wdsetctlr(wd) != 0) {
			/* Already printed a message. */
			goto bad;
		}
		wd->sc_state = GEOMETRY_WAIT;
		break;

	case GEOMETRY_WAIT:
		if (wdc->sc_status & WDCS_ERR) {
			wderror(wd, NULL, "wdcontrol: geometry failed");
			goto bad;
		}
		/* fall through */
	case MULTIMODE:
	multimode:
		if (wd->sc_mode != WDM_PIOMULTI)
			goto ready;
		bus_space_write_1(wdc->sc_iot, wdc->sc_ioh, wd_seccnt, wd->sc_multiple);
		if (wdcommandshort(wdc, wd->sc_drive, WDCC_SETMULTI) != 0) {
			wderror(wd, NULL, "wdcontrol: setmulti failed (1) "
					  "- reverting to single sector mode");
			wd->sc_mode = WDM_PIOSINGLE;
			wd->sc_multiple = 1;
			wdcunwedge(wdc);
			goto ready;
		}
		wd->sc_state = MULTIMODE_WAIT;
		break;

	case MULTIMODE_WAIT:
		if (wdc->sc_status & WDCS_ERR) {
			wderror(wd, NULL, "wdcontrol: setmulti failed (2) "
					  "- reverting to single sector mode");
			wd->sc_mode = WDM_PIOSINGLE;
			wd->sc_multiple = 1;
			goto bad;
		}
		/* fall through */
	case READY:
	ready:
		wdc->sc_errors = 0;
		wd->sc_state = READY;
		/*
		 * The rest of the initialization can be done by normal means.
		 */
		return 1;

	bad:
		wdcunwedge(wdc);
		return 0;
	}

	wdc->sc_flags |= WDCF_ACTIVE;
	if (wdc->sc_inten) wdc->sc_inten(wdc, 1);
	timeout(wdctimeout, wdc, WAITTIME);
	return 0;
}

/*
 * Wait for the drive to become ready and send a command.
 * Return -1 if busy for too long or 0 otherwise.
 * Assumes interrupts are blocked.
 */
int
wdcommand(wd, command, cylin, head, sector, count)
	struct wd_softc *wd;
	int command;
	int cylin, head, sector, count;
{
	struct wdc_softc *wdc = (void *)wd->sc_dev.dv_parent;
	bus_space_tag_t iot = wdc->sc_iot;
	bus_space_handle_t ioh = wdc->sc_ioh;
	int stat;
    
	/* Select drive, head, and addressing mode. */
	bus_space_write_1(iot, ioh, wd_sdh, WDSD_IBM | (wd->sc_drive << 4) | head);

	/* Wait for it to become ready to accept a command. */
	if (command == WDCC_IDP)
		stat = wait_for_unbusy(wdc);
	else
		stat = wdcwait(wdc, WDCS_DRDY);
	if (stat < 0)
		return -1;
    
	/* Load parameters. */
	if (wd->sc_dk.dk_label->d_type == DTYPE_ST506)
		bus_space_write_1(iot, ioh, wd_precomp, wd->sc_dk.dk_label->d_precompcyl / 4);
	else
		bus_space_write_1(iot, ioh, wd_features, 0);
	bus_space_write_1(iot, ioh, wd_cyl_lo, cylin);
	bus_space_write_1(iot, ioh, wd_cyl_hi, cylin >> 8);
	bus_space_write_1(iot, ioh, wd_sector, sector);
	bus_space_write_1(iot, ioh, wd_seccnt, count);

	/* Send command. */
	bus_space_write_1(iot, ioh, wd_command, command);

	return 0;
}

/*
 * Simplified version of wdcommand().
 */
int
wdcommandshort(wdc, drive, command)
	struct wdc_softc *wdc;
	int drive;
	int command;
{
	/* Select drive. */
	bus_space_write_1(wdc->sc_iot, wdc->sc_ioh, wd_sdh, WDSD_IBM | (drive << 4));

	if (wdcwait(wdc, WDCS_DRDY) < 0)
		return -1;

	bus_space_write_1(wdc->sc_iot, wdc->sc_ioh, wd_command, command);

	return 0;
}

/*
 * Tell the drive what geometry to use.
 */
int
wdsetctlr(wd)
	struct wd_softc *wd;
{

#ifdef WDDEBUG
	printf("wd(%d,%d) C%dH%dS%d\n", wd->sc_dev.dv_unit, wd->sc_drive,
	    wd->sc_dk.dk_label->d_ncylinders, wd->sc_dk.dk_label->d_ntracks,
	    wd->sc_dk.dk_label->d_nsectors);
#endif
    
	if (wdcommand(wd, WDCC_IDP, wd->sc_dk.dk_label->d_ncylinders,
	    wd->sc_dk.dk_label->d_ntracks - 1, 0,
	    wd->sc_dk.dk_label->d_nsectors) != 0) {
		wderror(wd, NULL, "wdsetctlr: geometry upload failed");
		return -1;
	}

	return 0;
}

/*
 * Get the drive parameters, if ESDI or ATA, or create fake ones for ST506.
 */
int
wd_get_parms(wd)
	struct wd_softc *wd;
{
	struct wdc_softc *wdc = (void *)wd->sc_dev.dv_parent;
	int i;
	char tb[DEV_BSIZE];
	int s, error;

	/*
	 * XXX
	 * The locking done here, and the length of time this may keep the rest
	 * of the system suspended, is a kluge.  This should be rewritten to
	 * set up a transfer and queue it through wdstart(), but it's called
	 * infrequently enough that this isn't a pressing matter.
	 */

	s = splbio();

	while ((wdc->sc_flags & WDCF_ACTIVE) != 0) {
		wdc->sc_flags |= WDCF_WANTED;
		if ((error = tsleep(wdc, PRIBIO | PCATCH, "wdprm", 0)) != 0) {
			splx(s);
			return error;
		}
	}

	if (!claimed_dma(wdc, NULL))
		panic("wd_get_parms: Cannot claim DMA");

	if (wdcommandshort(wdc, wd->sc_drive, WDCC_IDENTIFY) != 0 ||
	    wait_for_drq(wdc) != 0) {
		/*
		 * We `know' there's a drive here; just assume it's old.
		 * This geometry is only used to read the MBR and print a
		 * (false) attach message.
		 */
		strncpy(wd->sc_dk.dk_label->d_typename, "ST506",
		    sizeof wd->sc_dk.dk_label->d_typename);
		wd->sc_dk.dk_label->d_type = DTYPE_ST506;

		strncpy(wd->sc_params.wdp_model, "unknown",
		    sizeof wd->sc_params.wdp_model);
		wd->sc_params.wdp_config = WD_CFG_FIXED;
		wd->sc_params.wdp_cylinders = 1024;
		wd->sc_params.wdp_heads = 8;
		wd->sc_params.wdp_sectors = 17;
		wd->sc_params.wdp_maxmulti = 0;
		wd->sc_params.wdp_usedmovsd = 0;
		wd->sc_params.wdp_capabilities = 0;
	} else {
		strncpy(wd->sc_dk.dk_label->d_typename, "ESDI/IDE",
		    sizeof wd->sc_dk.dk_label->d_typename);
		wd->sc_dk.dk_label->d_type = DTYPE_ESDI;

		/* Read in parameter block. */
		bus_space_read_multi_2(wdc->sc_iot, wdc->sc_dataioh, wd_data,
		    (u_int16_t *)tb, sizeof(tb) / sizeof(short));
		bcopy(tb, &wd->sc_params, sizeof(struct wdparams));

		/* Shuffle string byte order. */
		for (i = 0; i < sizeof(wd->sc_params.wdp_model); i += 2) {
			u_short *p;
			p = (u_short *)(wd->sc_params.wdp_model + i);
			*p = ntohs(*p);
		}
	}

	/* Clear any leftover interrupt. */
	(void) bus_space_read_1(wdc->sc_iot, wdc->sc_ioh, wd_status);

	free_dma(wdc);

	/* Restart the queue. */
	wdcstart(wdc);

	splx(s);
	return 0;
}

int
wdioctl(dev, cmd, addr, flag, p)
	dev_t dev;
	u_long cmd;
	caddr_t addr;
	int flag;
	struct proc *p;
{
	struct wd_softc *wd = wd_cd.cd_devs[WDUNIT(dev)];
	int error;
    
	if ((wd->sc_flags & WDF_LOADED) == 0)
		return EIO;

	switch (cmd) {
#if 0 /* LWP */
	case DIOCSBAD:
		if ((flag & FWRITE) == 0)
			return EBADF;
		wd->sc_dk.dk_cpulabel->bad = *(struct dkbad *)addr;
		wd->sc_dk.dk_label->d_flags |= D_BADSECT;
		bad144intern(wd);
		return 0;
#endif

	case DIOCGDINFO:
		*(struct disklabel *)addr = *(wd->sc_dk.dk_label);
		return 0;
	
	case DIOCGPART:
		((struct partinfo *)addr)->disklab = wd->sc_dk.dk_label;
		((struct partinfo *)addr)->part =
		    &wd->sc_dk.dk_label->d_partitions[WDPART(dev)];
		return 0;
	
	case DIOCWDINFO:
	case DIOCSDINFO:
		if ((flag & FWRITE) == 0)
			return EBADF;

		if ((error = wdlock(wd)) != 0)
			return error;
		wd->sc_flags |= WDF_LABELLING;

		error = setdisklabel(wd->sc_dk.dk_label,
		    (struct disklabel *)addr, /*wd->sc_dk.dk_openmask : */0,
		    wd->sc_dk.dk_cpulabel);
		if (error == 0) {
			if (wd->sc_state > GEOMETRY)
				wd->sc_state = GEOMETRY;
			if (cmd == DIOCWDINFO)
				error = writedisklabel(WDLABELDEV(dev),
				    wdstrategy, wd->sc_dk.dk_label,
				    wd->sc_dk.dk_cpulabel);
		}

		wd->sc_flags &= ~WDF_LABELLING;
		wdunlock(wd);
		return error;
	
	case DIOCWLABEL:
		if ((flag & FWRITE) == 0)
			return EBADF;
		if (*(int *)addr)
			wd->sc_flags |= WDF_WLABEL;
		else
			wd->sc_flags &= ~WDF_WLABEL;
		return 0;

	case DIOCGDEFLABEL:
		wdgetdefaultlabel(wd, (struct disklabel *)addr);
		return 0;

#ifdef notyet
	case DIOCWFORMAT:
		if ((flag & FWRITE) == 0)
			return EBADF;
	{
		register struct format_op *fop;
		struct iovec aiov;
		struct uio auio;
	    
		fop = (struct format_op *)addr;
		aiov.iov_base = fop->df_buf;
		aiov.iov_len = fop->df_count;
		auio.uio_iov = &aiov;
		auio.uio_iovcnt = 1;
		auio.uio_resid = fop->df_count;
		auio.uio_segflg = 0;
		auio.uio_offset =
		    fop->df_startblk * wd->sc_dk.dk_label->d_secsize;
		auio.uio_procp = p;
		error = physio(wdformat, NULL, dev, B_WRITE, minphys,
		    &auio);
		fop->df_count -= auio.uio_resid;
		fop->df_reg[0] = wdc->sc_status;
		fop->df_reg[1] = wdc->sc_error;
		return error;
	}
#endif

	default:
		return ENOTTY;
	}

#ifdef DIAGNOSTIC
	panic("wdioctl: impossible");
#endif
}

#ifdef B_FORMAT
int
wdformat(struct buf *bp)
{

	bp->b_flags |= B_FORMAT;
	return wdstrategy(bp);
}
#endif

int
wdsize(dev)
	dev_t dev;
{
	struct wd_softc *wd;
	int part, unit, omask;
	int size;

	unit = WDUNIT(dev);
	if (unit >= wd_cd.cd_ndevs)
		return (-1);
	wd = wd_cd.cd_devs[unit];
	if (wd == NULL)
		return (-1);    

	part = WDPART(dev);     
	omask = wd->sc_dk.dk_openmask & (1 << part);

    
	if (omask == 0 && wdopen(dev, 0, S_IFBLK, NULL) != 0)
		return -1;
	if (wd->sc_dk.dk_label->d_partitions[part].p_fstype != FS_SWAP)
		size = -1;
	else
		size = wd->sc_dk.dk_label->d_partitions[part].p_size *
		    (wd->sc_dk.dk_label->d_secsize / DEV_BSIZE);
	if (omask == 0 && wdclose(dev, 0, S_IFBLK, NULL) != 0)
		return -1;
	return size;
}


#ifndef __BDEVSW_DUMP_OLD_TYPE
/* #define WD_DUMP_NOT_TRUSTED if you just want to watch */
static int wddoingadump;
static int wddumprecalibrated;

/*
 * Dump core after a system crash.
 */
int
wddump(dev, blkno, va, size)
        dev_t dev;
        daddr_t blkno;
        caddr_t va;
        size_t size;
{
	struct wd_softc *wd;	/* disk unit to do the I/O */
	struct wdc_softc *wdc;	/* disk controller to do the I/O */
	struct disklabel *lp;	/* disk's disklabel */
	int	unit, part;
	int	nblks;		/* total number of sectors left to write */

	/* Check if recursive dump; if so, punt. */
	if (wddoingadump)
		return EFAULT;
	wddoingadump = 1;

	unit = WDUNIT(dev);
	if (unit >= wd_cd.cd_ndevs)
		return ENXIO;
	wd = wd_cd.cd_devs[unit];
	if (wd == 0)
		return ENXIO;

	part = WDPART(dev);

	/* Make sure it was initialized. */
	if (wd->sc_state < READY)
		return ENXIO;

	wdc = (void *)wd->sc_dev.dv_parent;

        /* Convert to disk sectors.  Request must be a multiple of size. */
	lp = wd->sc_dk.dk_label;
	if ((size % lp->d_secsize) != 0)
		return EFAULT;
	nblks = size / lp->d_secsize;
	blkno = blkno / (lp->d_secsize / DEV_BSIZE);

	/* Check transfer bounds against partition size. */
	if ((blkno < 0) || ((blkno + nblks) > lp->d_partitions[part].p_size))
		return EINVAL;  

	/* Offset block number to start of partition. */
	blkno += lp->d_partitions[part].p_offset;

	/* Recalibrate, if first dump transfer. */
	if (wddumprecalibrated == 0) {
		wddumprecalibrated = 1;
		if (wdcommandshort(wdc, wd->sc_drive, WDCC_RECAL) != 0 ||
		    wait_for_ready(wdc) != 0 || wdsetctlr(wd) != 0 ||
		    wait_for_ready(wdc) != 0) {
			wderror(wd, NULL, "wddump: recal failed");
			return EIO;
		}
	}
   
	while (nblks > 0) {
		daddr_t xlt_blkno = blkno;
		long cylin, head, sector;

		if ((lp->d_flags & D_BADSECT) != 0) {
			long blkdiff;
			int i;

			for (i = 0; (blkdiff = wd->sc_badsect[i]) != -1; i++) {
				blkdiff -= xlt_blkno;
				if (blkdiff < 0)
					continue;
				if (blkdiff == 0) {
					/* Replace current block of transfer. */
					xlt_blkno = lp->d_secperunit -
					    lp->d_nsectors - i - 1;
				}
				break;
			}
			/* Tranfer is okay now. */
		}

		if ((wd->sc_params.wdp_capabilities & WD_CAP_LBA) != 0) {
			sector = (xlt_blkno >> 0) & 0xff;
			cylin = (xlt_blkno >> 8) & 0xffff;
			head = (xlt_blkno >> 24) & 0xf;
			head |= WDSD_LBA;
		} else {
			sector = xlt_blkno % lp->d_nsectors;
			sector++;	/* Sectors begin with 1, not 0. */
			xlt_blkno /= lp->d_nsectors;
			head = xlt_blkno % lp->d_ntracks;
			xlt_blkno /= lp->d_ntracks;
			cylin = xlt_blkno;
			head |= WDSD_CHS;
		}

#ifndef WD_DUMP_NOT_TRUSTED
		if (wdcommand(wd, WDCC_WRITE, cylin, head, sector, 1) != 0 ||
		    wait_for_drq(wdc) != 0) {
			wderror(wd, NULL, "wddump: write failed");
			return EIO;
		}
	
		bus_space_write_multi_2(wdc->sc_iot, wdc->sc_dataioh, wd_data,
		    (u_int16_t *)va, lp->d_secsize >> 1);
	
		/* Check data request (should be done). */
		if (wait_for_ready(wdc) != 0) {
			wderror(wd, NULL, "wddump: timeout waiting for ready");
			return EIO;
		}
#else	/* WD_DUMP_NOT_TRUSTED */
		/* Let's just talk about this first... */
		printf("wd%d: dump addr 0x%x, cylin %d, head %d, sector %d\n",
		    unit, va, cylin, head, sector);
		delay(500 * 1000);	/* half a second */
#endif

		/* update block count */
		nblks -= 1;
		blkno += 1;
		va += lp->d_secsize;
	}

	wddoingadump = 0;
	return 0;
}
#else /* __BDEVSW_DUMP_NEW_TYPE */
int
wddump(dev, blkno, va, size)
        dev_t dev;
        daddr_t blkno;
        caddr_t va;
        size_t size;
{

	/* Not implemented. */
	return ENXIO;
}
#endif /* __BDEVSW_DUMP_NEW_TYPE */

#if 0 /* LWP */
/*
 * Internalize the bad sector table.
 */
void
bad144intern(wd)
	struct wd_softc *wd;
{
	struct dkbad *bt = &wd->sc_dk.dk_cpulabel->bad;
	struct disklabel *lp = wd->sc_dk.dk_label;
	int i = 0;

	for (; i < 126; i++) {
		if (bt->bt_bad[i].bt_cyl == 0xffff)
			break;
		wd->sc_badsect[i] =
		    bt->bt_bad[i].bt_cyl * lp->d_secpercyl +
		    (bt->bt_bad[i].bt_trksec >> 8) * lp->d_nsectors +
		    (bt->bt_bad[i].bt_trksec & 0xff);
	}
	for (; i < 127; i++)
		wd->sc_badsect[i] = -1;
}
#endif /* 0 */

int
wdcreset(wdc)
	struct wdc_softc *wdc;
{
	bus_space_tag_t iot = wdc->sc_iot;
	bus_space_handle_t ioh = wdc->sc_ioh;
	bus_space_handle_t aux_ioh = wdc->sc_auxioh;

	/* Reset the device. */
	if (wdresethack) {
		bus_space_write_1(iot, aux_ioh, wd_ctlr, WDCTL_RST | WDCTL_IDS);
		delay(1000);
		bus_space_write_1(iot, aux_ioh, wd_ctlr, WDCTL_IDS);
		delay(1000);
		(void) bus_space_read_1(iot, ioh, wd_error);
		bus_space_write_1(iot, aux_ioh, wd_ctlr, WDCTL_4BIT);
	}

	if (wait_for_unbusy(wdc) < 0) {
		if (!(wdc->sc_flags & WDCF_QUIET))
			printf("%s: reset failed\n", wdc->sc_dev.dv_xname);
		return 1;
	}

/*printf("wdcreset: %d\n", __LINE__);*/
	return 0;
}

void
wdcrestart(arg)
	void *arg;
{
	struct wdc_softc *wdc = arg;
	int s;

	s = splbio();
	wdcstart(wdc);
	splx(s);
}

/*
 * Unwedge the controller after an unexpected error.  We do this by resetting
 * it, marking all drives for recalibration, and stalling the queue for a short
 * period to give the reset time to finish.
 * NOTE: We use a timeout here, so this routine must not be called during
 * autoconfig or dump.
 */
void
wdcunwedge(wdc)
	struct wdc_softc *wdc;
{
	int unit;

	untimeout(wdctimeout, wdc);
	(void) wdcreset(wdc);

	/* Schedule recalibrate for all drives on this controller. */
	for (unit = 0; unit < wd_cd.cd_ndevs; unit++) {
		struct wd_softc *wd = wd_cd.cd_devs[unit];
		if (!wd || (void *)wd->sc_dev.dv_parent != wdc)
			continue;
		if (wd->sc_state > RECAL)
			wd->sc_state = RECAL;
	}

	wdc->sc_flags |= WDCF_ERROR;
	++wdc->sc_errors;

	/* Wake up in a little bit and restart the operation. */
	timeout(wdcrestart, wdc, RECOVERYTIME);
}

int
wdcwait(wdc, mask)
	struct wdc_softc *wdc;
	int mask;
{
	int timeout = 0;
	u_char status;
#ifdef WDCNDELAY_DEBUG
	extern int cold;
#endif

	for (;;) {
		wdc->sc_status = status = bus_space_read_1(wdc->sc_iot,
		    wdc->sc_ioh, wd_status);
		if ((status & WDCS_BSY) == 0 && (status & mask) == mask)
			break;
		if (++timeout > WDCNDELAY)
			return -1;
		delay(WDCDELAY);
	}
	if (status & WDCS_ERR) {
		wdc->sc_error = bus_space_read_1(wdc->sc_iot, wdc->sc_ioh,
		    wd_error);
		return WDCS_ERR;
	}
#ifdef WDCNDELAY_DEBUG
	/* After autoconfig, there should be no long delays. */
	if (!cold && timeout > WDCNDELAY_DEBUG)
		printf("%s: warning: busy-wait took %dus\n",
		    wdc->sc_dev.dv_xname, WDCDELAY * timeout);
#endif
	return 0;
}

void
wdctimeout(arg)
	void *arg;
{
	struct wdc_softc *wdc = (struct wdc_softc *)arg;
	int s;

	s = splbio();
	if ((wdc->sc_flags & WDCF_ACTIVE) != 0) {
		struct wd_softc *wd = wdc->sc_drives.tqh_first;
		struct buf *bp = wd->sc_q.b_actf;

		wdc->sc_flags &= ~WDCF_ACTIVE;
		if (wdc->sc_inten) wdc->sc_inten(wdc, 0);
		wderror(wdc, NULL, "lost interrupt");
		printf("%s: lost interrupt: %sing %d@%s:%d\n",
		    wdc->sc_dev.dv_xname,
		    (bp->b_flags & B_READ) ? "read" : "writ",
		    wd->sc_nblks, wd->sc_dev.dv_xname, wd->sc_blkno);
		wdcunwedge(wdc);
	} else
		wderror(wdc, NULL, "missing untimeout");
	splx(s);
}

void
wderror(dev, bp, msg)
	void *dev;
	struct buf *bp;
	char *msg;
{
	struct wd_softc *wd = dev;
	struct wdc_softc *wdc = dev;

	if (bp) {
		diskerr(bp, "wd", msg, LOG_PRINTF, wd->sc_skip / DEV_BSIZE,
		    wd->sc_dk.dk_label);
		printf("\n");
	} else
		printf("%s: %s: status %b error %b\n", wdc->sc_dev.dv_xname,
		    msg, wdc->sc_status, WDCS_BITS, wdc->sc_error, WDERR_BITS);
}
