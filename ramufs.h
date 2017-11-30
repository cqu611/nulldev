#ifndef RAM_UFS_H
#define RAM_UFS_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/configfs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/ctype.h>

/* space or '=' */
#define RU_PARSE_STATUS_SPACE	0
#define RU_PARSE_STATUS_ERROR	1
/* now is parsing key */
#define RU_PARSE_STATUS_KEYING	2
/* the key parsed just now */
#define RU_PARSE_STATUS_KEYED	3
#define RU_PARSE_STATUS_VALING	4
#define RU_PARSE_STATUS_VALED	5
/* out of range */
#define RU_PARSE_STATUS_RANGED	6



struct ufs_ppa_format {
	u8	ch_off;
	u8	ch_len;
	u8	lun_off;
	u8	lun_len;
	u8	pln_off;
	u8	pln_len;
	u8	blk_off;
	u8	blk_len;
	u8	pg_off;
	u8	pg_len;
	u8	sect_off;
	u8	sect_len;
};

struct ufs_geo_l2p_mlc {
	u16	num_pairs;
	u8	pairs[886];
};

struct ufs_geo_l2p_tbl {
	__u8	id[8];
	struct ufs_geo_l2p_mlc mlc;
};

struct ufs_geo_group {
	u8	mtype;
	u8	fmtype;
	u8	num_ch;
	u8	num_lun;
	u8	num_pln;
	u16	num_blk;
	u16	num_pg;
	u16	fpg_sz;
	u16	csecs;
	u16	sos;
	u32	trdt;
	u32	trdm;
	u32	tprt;
	u32	tprm;
	u32	tbet;
	u32	tbem;
	u32	mpos;
	u32	mccap;
	u16	cpar;
	struct ufs_geo_l2p_tbl l2ptbl;
};

struct ufs_geo {
	u8	version;
	u8	vnvmt;
	u8	cgrps;
	u32	cap;
	u32	dom;
	struct ufs_ppa_format ppaf;
	struct ufs_geo_group  ggrp;
}__packed;

struct ramufs {
	struct ufs_geo *geo;

	struct gendisk *gd;
	struct kobject *kobj;
	struct device *dev;
};
//#define kobj_to_ramufs(x) container_of(x, struct ramufs, kobj)

#endif /* RAM_UFS_H */
