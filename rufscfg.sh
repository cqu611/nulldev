#!/bin/bash

help="Usage: rufscfg [show]|[store]"
ufsgeo="version = 0x23 vnvmt = 0x31 cgrps = 98 cap = 12345610 dom = 0x04441"
ppafmt="ch_off=12 ch_len=13 lun_off=14 lun_len=15 pln_off=16 pln_len=17 blk_off=18 blk_len=19 pg_off=20 pg_len=21 sect_off=22 sect_len = 23"
cfggrp=""
l2ptbl=""

if [ $# -eq 0 ] || [ $# -gt 1 ] || [ "$1" = "help" ] ; then
    echo ${help}
    exit 1
fi

if [ $1 = "show" ]; then
    cat /sys/kernel/ramufs/ufs_geo
    cat /sys/kernel/ramufs/ppa_fmt
    cat /sys/kernel/ramufs/cfg_grp
    cat /sys/kernel/ramufs/l2p_tbl
    exit 0  
fi

if [ $1 = "store" ]; then
    echo ${ufsgeo} > /sys/kernel/ramufs/ufs_geo
    echo ${ppafmt} > /sys/kernel/ramufs/ppa_fmt
    #echo ${cfggrp} > /sys/kernel/ramufs/cfg_grp
    #echo ${l2ptbl} > /sys/kernel/ramufs/l2p_tbl
    exit 0
fi  

echo ${help}
