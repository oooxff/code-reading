# $NetBSD: Makefile,v 1.14 1997/09/06 14:07:58 drochner Exp $

S=	${.CURDIR}/../../../..

.PATH: ${.CURDIR}/../common

PROG = bootxx

SRCS = start.S bootxx.c prom.c prom_disp.S puts.c
NOMAN=
STRIPFLAG=
BINMODE= 444

BOOT_RELOC = ${PRIMARY_LOAD_ADDRESS}
HEADERSIZE_PROG = headersize

AFLAGS += -DASSEMBLER
CPPFLAGS += -I${.CURDIR}/../.. -I${.CURDIR}/../../../.. -DPRIMARY_BOOTBLOCK
CFLAGS = -Wall -Werror -mno-fp-regs -g

CLEANFILES+= ${PROG}.sym ${PROG}.nosym ${HEADERSIZE_PROG}

${PROG}.nosym: ${PROG}.sym
	cp ${PROG}.sym ${PROG}.nosym
	strip ${PROG}.nosym

${PROG}: ${PROG}.nosym ${HEADERSIZE_PROG}
	dd if=${PROG}.nosym of=${PROG} \
	    ibs=`./${HEADERSIZE_PROG} ${BOOT_RELOC} ${PROG}.nosym` skip=1 \
	    obs=`expr 15 \* 512` conv=osync

.include <bsd.prog.mk>

### find out what to use for libkern
KERN_AS=	library
.include "${S}/lib/libkern/Makefile.inc"
LIBKERN=	${KERNLIB}

${PROG}.sym: ${OBJS} ${LIBKERN}
	${LD} -Ttext ${BOOT_RELOC} -N -e start -o ${PROG}.sym ${OBJS} \
	    ${LIBKERN}
	size ${PROG}.sym
