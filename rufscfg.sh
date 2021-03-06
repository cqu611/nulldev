#!/bin/bash

help="Usage: rufscfg [show]|[store]"
ufsgeo="version = 0x23 vnvmt = 0x31 cgrps = 98 cap = 12345610 dom = 0x04441"
ppafmt="ch_off=12 ch_len=13 lun_off=14 lun_len=15 pln_off=16 pln_len=17 blk_off=18 blk_len=19 pg_off=20 pg_len=21 sect_off=22 sect_len = 23"
cfggrp="mtype = 31 fmtype =32 num_ch=33 num_lun=34 num_pln =35 num_blk =36 num_pg =37 fpg_sz=38 csecs=39
sos =40 trdt=41 trdm =42 tprt =43 tprm 	=44 tbet=45 tbem =46 mpos =47 mccap =48 cpar =49"
l2ptbl=""

if [ $# -eq 0 ] || [ $# -gt 1 ] || [ "$1" = "help" ] ; then
    echo ${help}
    exit 1
fi

if [ $1 = "show" ]; then
    cat /sys/kernel/ramufs/ufs_geo
    echo
    cat /sys/kernel/ramufs/ppa_fmt
    echo
    cat /sys/kernel/ramufs/cfg_grp
    echo
    cat /sys/kernel/ramufs/l2p_tbl
    exit 0  
fi

if [ $1 = "store" ]; then
    echo ${ufsgeo} > /sys/kernel/ramufs/ufs_geo
    echo ${ppafmt} > /sys/kernel/ramufs/ppa_fmt
    echo ${cfggrp} > /sys/kernel/ramufs/cfg_grp
    #echo ${l2ptbl} > /sys/kernel/ramufs/l2p_tbl
    exit 0
fi  

echo ${help}
