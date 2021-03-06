#!/bin/sh
# $NetBSD: keywords.sh,v 1.4 1997/04/03 02:35:49 christos Exp $
# @(#)keywords	8.2 (Berkeley) 3/19/94
#
# WARNING!  If you change this file, re-run it!

# This program requires "new" awk (or GNU awk).
awk=${AWK:-awk}

cat << _EOF_ > _keywords.t1
add
atalk
blackhole
change
cloning
delete
dst
expire
flush
gateway
genmask
get
host
hopcount
iface
interface
ifa
ifp
inet
iso
link
llinfo
lock
lockrest
mask
monitor
mtu
net
netmask
nostatic
osi
proto1
proto2
recvpipe
reject
rtt
rttvar
sa
sendpipe
show
ssthresh
static
x25
xns
xresolve
_EOF_


################################################################
# Setup
################################################################

# This creates a stream of:
#	keyword KEYWORD
# (lower case, upper case).
tr a-z A-Z < _keywords.t1 |
paste _keywords.t1 - > _keywords.t2


################################################################
# Generate the h file
################################################################
exec > keywords.h

echo '/* $'NetBSD'$ */

/* WARNING!  This file was generated by keywords.sh  */

extern struct keytab {
	char	*kt_cp;
	int	kt_i;
} keywords[];

' # defines follow

$awk '{
	printf("#define\tK_%s\t%d\n", $2, NR);
}' < _keywords.t2


################################################################
# Generate the c file
################################################################
exec > keywords.c

echo '/* $'NetBSD'$ */

/* WARNING!  This file was generated by keywords.sh  */

#include "keywords.h"

struct keytab keywords[] = {
' # initializers follow

$awk '{
	printf("\t{\"%s\", K_%s},\n", $1, $2);
}' < _keywords.t2

echo '	{0, 0}
};
' # tail


################################################################
# Cleanup
################################################################

rm -f _keywords.t1 _keywords.t2
exit 0
