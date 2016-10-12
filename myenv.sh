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
# Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
# Copyright 2012 Joshua M. Clulow <josh@sysmgr.org>
# Copyright 2013 Nexenta Systems, Inc. All rights reserved.
#

#	Configuration variables for the runtime environment of the nightly
# build script and other tools for construction and packaging of releases.
# This script is sourced by 'nightly' and 'bldenv' to set up the environment
# for the build. This example is suitable for building an OpenSolaris
# workspace, which will contain the resulting archives. It is based
# off the onnv release. It sets NIGHTLY_OPTIONS to make nightly do:
#	do DEBUG (only!) build (-D -F)
#	do not bringover from the parent (-n)
#	runs 'make check' (-C)
#	runs lint in usr/src (-l plus the LINTDIRS variable)
#	sends mail on completion (-m and the MAILTO variable)
#	creates packages for PIT/RE (-p)
#	checks for changes in ELF runpaths (-r)
#	build and use this workspace's tools in $SRC/tools (-t)
#
export NIGHTLY_OPTIONS='-DCFlnpr'

# CODEMGR_WS - where is your workspace (or what should nightly name it)
export CODEMGR_WS=`git rev-parse --show-toplevel`

# This is a variable for the rest of the script - GATE doesn't matter to
# nightly itself.
GATE=`basename ${CODEMGR_WS}`

# Maximum number of dmake jobs.  The recommended number is 2 + NCPUS,
# where NCPUS is the number of logical CPUs on your build system.
export DMAKE_MAX_JOBS=4

# PARENT_WS is used to determine the parent of this workspace. This is
# for the options that deal with the parent workspace (such as where the
# proto area will go).
# export PARENT_WS=

# CLONE_WS is the workspace nightly should do a bringover from.
# export CLONE_WS=

# The bringover, if any, is done as STAFFER.
# Set STAFFER to your own login as gatekeeper or developer
# The point is to use group "staff" and avoid referencing the parent
# workspace as root.
# Some scripts optionally send mail messages to MAILTO.
#
export STAFFER="$LOGNAME"
export MAILTO="$STAFFER"

# If you wish the mail messages to be From: an arbitrary address, export
# MAILFROM.
#export MAILFROM="user@example.com"

# The project (see project(4)) under which to run this build.  If not
# specified, the build is simply run in a new task in the current project.
export BUILD_PROJECT=

# You should not need to change the next three lines
export ATLOG="$CODEMGR_WS/log"
export LOGFILE="$ATLOG/nightly.log"
export MACH="$(uname -p)"

#
#  The following two macros are the closed/crypto binaries.  Once
#  Illumos has totally freed itself, we can remove these references.
#
# Location of encumbered binaries.
# This assumes you keep a local copy around
export ON_CLOSED_BINS=/ws/on-closed-bins/onnv147/closed
# Location of signed cryptographic binaries.
# export ON_CRYPTO_BINS="$CLONE_WS/on-crypto.$MACH.tar.bz2"

#
# REF_PROTO_LIST - for comparing the list of stuff in your proto area
# with. Generally this should be left alone, since you want to see differences
# from your parent (the gate).
#
export REF_PROTO_LIST="$PARENT_WS/usr/src/proto_list_${MACH}"

export ROOT="$CODEMGR_WS/proto/root_${MACH}"
export SRC="$CODEMGR_WS/usr/src"
export MULTI_PROTO="no"

#
# Build environment variables, including version info for mcs, motd,
# motd, uname and boot messages. Mostly you shouldn't change this except
# when a release name changes, etc.
#
# With modern SCM systems like git, one typically wants the
# change set ID (hash) in the version sring.
GIT_REV=`git rev-parse --short=10 HEAD`
export VERSION="${GATE}:${GIT_REV}"
export ONNV_BUILDNUM=152

#
# the RELEASE and RELEASE_DATE variables are set in Makefile.master;
# there might be special reasons to override them here.
#
# export RELEASE='5.11'
# export RELEASE_DATE='October 2007'

# proto area in parent for optionally depositing a copy of headers and
# libraries corresponding to the protolibs target
# not applicable given the NIGHTLY_OPTIONS
#
export PARENT_ROOT="$PARENT_WS/proto/root_$MACH"
export PARENT_TOOLS_ROOT="$PARENT_WS/public/tools/root_$MACH-nd"

# Package creation variables.  You probably shouldn't change these,
# either.
#
# PKGARCHIVE determines where the repository will be created.
#
# PKGPUBLISHER_REDIST controls the publisher setting for the repository.
#
export PKGARCHIVE="${CODEMGR_WS}/packages/${MACH}/nightly"
# export PKGPUBLISHER_REDIST='on-redist'

# Package manifest format version.
export PKGFMT_OUTPUT='v1'

# we want make to do as much as it can, just in case there's more than
# one problem.
# export MAKEFLAGS='k'

# Magic variable to prevent the devpro compilers/teamware from sending
# mail back to devpro on every use.
export UT_NO_USAGE_TRACKING='1'

# Build tools - don't change these unless you know what you're doing.  These
# variables allows you to get the compilers and onbld files locally or
# through cachefs.  Set BUILD_TOOLS to pull everything from one location.
# Alternately, you can set ONBLD_TOOLS to where you keep the contents of
# SUNWonbld and SPRO_ROOT to where you keep the compilers.  SPRO_VROOT
# exists to make it easier to test new versions of the compiler.
#export BUILD_TOOLS=/ws/onnv-tools
export ONBLD_TOOLS=/opt/onbld
#export SPRO_ROOT=/opt/onspro
#export SPRO_VROOT="$SPRO_ROOT"

# path to onbld tool binaries
ONBLD_BIN="${ONBLD_TOOLS}/bin"

# This goes along with lint - it is a series of the form "A [y|n]" which
# means "go to directory A and run 'make lint'" Then mail me (y) the
# difference in the lint output. 'y' should only be used if the area you're
# linting is actually lint clean or you'll get lots of mail.
# You shouldn't need to change this though.
#export LINTDIRS="$SRC y"

# Set this flag to 'n' to disable the automatic validation of the dmake
# version in use.  The default is to check it.
#CHECK_DMAKE=y

# Set this flag to 'n' to disable the use of 'checkpaths'.  The default,
# if the 'N' option is not specified, is to run this test.
#CHECK_PATHS=y

# POST_NIGHTLY can be any command to be run at the end of nightly.  See
# nightly(1) for interactions between environment variables and this command.
#POST_NIGHTLY=

# XXX "parent root" (where I build gfx_private + AGP)
export ILLUMOS_GFX_GATE=/tank/ws/illumos-gfx

