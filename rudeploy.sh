#!/bin/bash

# This script using for deploying source files into linufs DIR.

LINUFS="/root/linufs"

VERSION=4
PATCHLEVEL=14
SUBLEVEL=2

RAMUFS_C="ramufs.c"
RAMUFS_H="ramufs.h"
RUFSLIB_C="rufslib.c"
UFS_KCONFIG="ufs_Kconfig"
UFS_MAKEFILE="ufs_Makefile"

PATH_UFS=${LINUFS}"/drivers/scsi/ufs/"

HELP="Usage: rudeploy [run]|[check]"

if [ $# -eq 0 ] || [ $# -gt 1 ] || [ "$1" = "help" ] ; then
    echo ${HELP}
    exit 1
fi

function check() {
    if [ ! -d $LINUFS ] ; then
        echo "Linux source DIR ["${LINUFS}"] NOT found!"
        exit 0  
    fi

    L_VERSION=`sed -n 2p ${LINUFS}"/Makefile" | sed "s/[^0-9]//g"`
    L_PATCHLEVEL=`sed -n 3p ${LINUFS}"/Makefile" | sed "s/[^0-9]//g"`
    L_SUBLEVEL=`sed -n 4p ${LINUFS}"/Makefile" | sed "s/[^0-9]//g"`

    echo "Current version: ${VERSION}.${PATCHLEVEL}.${SUBLEVEL}"
    echo "Target Linux version: ${L_VERSION}.${L_PATCHLEVEL}.${L_SUBLEVEL}"

    if [ $L_VERSION -ne $VERSION ] || [ $L_PATCHLEVEL -ne $PATCHLEVEL ] || [ $L_SUBLEVEL -ne $SUBLEVEL ] ; then
        echo "EXIT ERROR: Linux version unmatched"
        exit 0
    else
        echo "Linux version matched"
    fi
}

if [ $1 = "check" ] ; then
    check
    exit 0
fi

function deploy() {
    echo "copy $1 to $2"
    cp $1 $2
}

if [ $1 = "run" ] ; then
    check
    deploy ${RAMUFS_C} ${PATH_UFS}${RAMUFS_C}
    deploy ${RAMUFS_H} ${PATH_UFS}${RAMUFS_H}
    deploy ${RUFSLIB_C} ${PATH_UFS}${RUFSLIB_C}
    deploy ${UFS_KCONFIG} ${PATH_UFS}"Kconfig"
    deploy ${UFS_MAKEFILE} ${PATH_UFS}"Makefile"
    echo "Completed!"
    exit 0
fi

echo ${HELP}
