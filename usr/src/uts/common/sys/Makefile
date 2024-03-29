#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
#
# Copyright (c) 1989, 2010, Oracle and/or its affiliates. All rights reserved.
# Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
#

include $(SRC)/uts/Makefile.uts

FILEMODE=644

#
#	Note that the following headers are present in the kernel but
#	neither installed or shipped as part of the product:
#		cpuid_drv.h:		Private interface for cpuid consumers
#		unix_bb_info.h:		Private interface to kcov
#

i386_HDRS=
sparc_HDRS=

# Generated headers
# XXX See "temporary hack" below
GENHDRS= \
	agpgart.h \
	agp/agpamd64gart_io.h \
	agp/agpdefs.h \
	agp/agpgart_impl.h \
	agp/agpmaster_io.h \
	agp/agptarget_io.h \
	gfx_private.h

CHKHDRS=

HDRS=	$(GENHDRS) $(CHKHDRS)

all_h:  $(GENHDRS)

install_h: all_h .WAIT $(ROOTDIRS) .WAIT $(ROOTHDRS)

clean:
	$(RM) $(GENHDRS)

clobber:	clean

check:	$(CHECKHDRS)

FRC:

#
# XXX Temporary hack until illumos gate AGP updates arrive.
# I'll "generate" these headers by copying from my clone of
# illumos gate where I built the updated AGP code.
#
$(GENHDRS) : agp

agp :
	-mkdir $@

%.h : $(ILLUMOS_GFX_GATE)/usr/src/uts/common/sys/%.h
	$(RM) $@
	$(CP) $< $@
