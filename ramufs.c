#include "ramufs.h"

static struct ufs_geo geo = {
	.version 		= 0,
	.vnvmt 			= 0,
	.cgrps 			= 1,
	.cap 			= 0,
	.dom 			= 0,
	.ppaf = {
		.ch_off 	= 0,
		.ch_len 	= 0,
		.lun_off 	= 0,
		.lun_len 	= 0,
		.pln_off 	= 0,
		.pln_len 	= 0,
		.blk_off 	= 0,
		.blk_len 	= 0,
		.pg_off 	= 0,
		.pg_len 	= 0,
		.sect_off 	= 0,
		.sect_len 	= 0,
	},
	.ggrp = {
		.mtype 		= 0,
		.fmtype 	= 0,
		.num_ch 	= 0,
		.num_lun 	= 0,
		.num_pln 	= 0,
		.num_blk 	= 0,
		.num_pg 	= 0,
		.fpg_sz 	= 0,
		.csecs 		= 0,
		.sos 		= 0,
		.trdt 		= 0,
		.trdm 		= 0,
		.tprt 		= 0,
		.tprm 		= 0,
		.tbet 		= 0,
		.tbem 		= 0,
		.mpos 		= 0,
		.mccap 		= 0,
		.cpar 		= 0,
		.l2ptbl = {
			/* .id[8], */
			.mlc = {
				.num_pairs = 0,
				/* .pairs[886], */
			},
		},
	},
};

struct ramufs *ufs;

static int __parse_config_parse_key(char *buf, char *val, int pos, int len)
{
	char tmpbuf[16];
	int valen = strlen(val);
	
	/* out of range */
	if (valen + pos > len || valen >= 16) 
		return RU_PARSE_STATUS_RANGED;

	memcpy(tmpbuf, &buf[pos], valen);
	tmpbuf[valen] = 0;

	return strcmp(tmpbuf, val) == 0 ? RU_PARSE_STATUS_KEYED : RU_PARSE_STATUS_UNMATCH;
}

static int __parse_config_parse_value(char *buf, int pos, void *value, 
				int *vallen, int count, int typelen)
{
	int i, j=0, flag=0, ret;
	char tmpbuf[32], dst[8];

	for (i=0; pos + i < count; i++) {
		if (buf[pos+i] == 0x20 || buf[pos+i] == 0) {
			if (flag == 0) {
				continue;
			} else {	/* terminated */
				if (j == 0 || j > 17)
					return RU_PARSE_STATUS_ERROR;
				tmpbuf[j] = 0;
				ret = hex2bin(dst, tmpbuf, (j-1)/2);
				if (ret)
					return RU_PARSE_STATUS_ERROR;
				if (typelen == 1)
					*(u8*)value = (u8)dst[0];
				else if (typelen == 2) 
					*(u16*)value = ((u16)dst[0] << 8) + (u16)dst[1];
				else if (typelen == 4)
					*(u32*)value = ((u32)dst[0] << 24) + ((u32)dst[1] << 16)
							+ ((u32)dst[2] << 8) + (u32)dst[3];
				else if (typelen == 8)
					*(u64*)value = ((u64)dst[0] << 56) + ((u64)dst[1] << 48)
							+ ((u64)dst[2] << 40) + ((u64)dst[3] << 32)
							+ ((u64)dst[4] << 24) + ((u64)dst[5] << 16)
							+ ((u64)dst[6] << 8) + (u64)dst[7];
				else
					return RU_PARSE_STATUS_ERROR;
				*vallen = i;
				return RU_PARSE_STATUS_SPACE;
			}
		} else {
			flag = 1;
			tmpbuf[j++] = buf[pos+i];
		}
	}
	
	return RU_PARSE_STATUS_ERROR;
}

static int __parse_config_ufs_geo(const char *buf, size_t count)
{
	int i, ret = 0, status;
	char *tmpbuf;
	u8 tmp_ver, tmp_vnvmt, tmp_cgrps;
	u32 tmp_cap, tmp_dom;
	void *val;
	int keylen, vallen, typelen;	/* key ,value and type length */

	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

	memcpy(tmpbuf, buf, count);
	for (i=0; i < count; i++) {
		tmpbuf[i] = tolower(tmpbuf[i]);
		if (tmpbuf[i] == '\t' || tmpbuf[i] == '\r' || tmpbuf[i] == '\n')
			tmpbuf[i] = 0x20;
	}

	/* begin to parse input string */
	status = RU_PARSE_STATUS_SPACE;
	for (i=0; i < count; i++) {
		
		/* try to match version or vnvmt */
		if (tmpbuf[i] == 'v') {
			if (status != RU_PARSE_STATUS_SPACE) {
				ret = -EINVAL;
				goto destroy_buf;
			}
			keylen = 7;
			val = &tmp_ver;
			typelen = 1;
			status = __parse_config_parse_key(tmpbuf, "version", i, count);
			if (status == RU_PARSE_STATUS_UNMATCH) {
				keylen = 5;
				val = &tmp_vnvmt;
				status = __parse_config_parse_key(tmpbuf, "vnvmt", i, count);
			}
			/* check matching status */
			if (status == RU_PARSE_STATUS_RANGED || status == RU_PARSE_STATUS_UNMATCH) {
				ret = -EINVAL;
				goto destroy_buf;
			} else if (status == RU_PARSE_STATUS_KEYED) {
				i += keylen-1;
				continue;
			} else {
				pr_err("RAMUFS: __parse_config_ufs_geo, bad status: %d\n", status);
				ret = -EINVAL;
				goto destroy_buf;
			}
		}else if (tmpbuf[i] == 'c') {

			/* try to match cgrps or cap */
			if (status != RU_PARSE_STATUS_SPACE) {
				ret = -EINVAL;
				goto destroy_buf;
			}
			keylen = 5;
			val = &tmp_cgrps;
			typelen = 1;
			status = __parse_config_parse_key(tmpbuf, "cgrps", i, count);
			if (status == RU_PARSE_STATUS_UNMATCH) {
				keylen = 3;
				val = &tmp_cap;
				typelen = 4;
				status = __parse_config_parse_key(tmpbuf, "cap", i, count);
			}
			if (status == RU_PARSE_STATUS_RANGED || status == RU_PARSE_STATUS_UNMATCH) {
				ret = -EINVAL;
				goto destroy_buf;
			} else if (status == RU_PARSE_STATUS_KEYED) {
				i += keylen-1;
				continue;
			} else {
				pr_err("RAMUFS: __parse_config_ufs_geo, bad status: %d\n", status);
				ret = -EINVAL;
				goto destroy_buf;
			}
		}else if (tmpbuf[i] == 'd') {

			/* try to match dom */
			if (status != RU_PARSE_STATUS_SPACE) {
				ret = -EINVAL;
				goto destroy_buf;
			}
			keylen = 3;
			val = &tmp_dom;
			typelen = 4;
			status = __parse_config_parse_key(tmpbuf, "dom", i, count);
			if (status == RU_PARSE_STATUS_RANGED || status == RU_PARSE_STATUS_UNMATCH) {
				ret = -EINVAL;
				goto destroy_buf;
			} else if (status == RU_PARSE_STATUS_KEYED) {
				i += keylen - 1;
				continue;
			} else {
				pr_err("RAMUFS: __parse_config_ufs_geo, bad status: %d\n", status);
				ret = -EINVAL;
				goto destroy_buf;
			}
		} else if (tmpbuf[i] == '=') {

			/* try to parse values */
			if (status != RU_PARSE_STATUS_KEYED) {
				ret = -EINVAL;
				goto destroy_buf;
			}
			/* i+1 is offset of value head in buffer */
			/* val is point to tmp_ver/tmp_vnvmt/tmp_cgrps/tmp_cap/tmp_dom */
			/* &vallen is the length of val, this used for calculate offset */
			/* typelen suggests val type, u8/u16/u32/u64 */
			status = __parse_config_parse_value(tmpbuf, i+1, val, &vallen, count, typelen);
			if (status == RU_PARSE_STATUS_ERROR) {
				ret = -EINVAL;
				goto destroy_buf;
			} else if (status == RU_PARSE_STATUS_SPACE) {
				i += vallen - 1;
				continue;
			} else {
				ret = -EINVAL;
				goto destroy_buf;
			}
		} else if (tmpbuf[i] == 0x20) {
			continue;
		} else {
			pr_err("RAMUFS: __parse_config_ufs_geo, bad status: %d\n", status);
			ret = -EINVAL;
			goto destroy_buf;
		}
		
	}

destroy_buf:
	kfree(tmpbuf);
out:
	return ret;
}

static int __parse_config_ppa_fmt(const char *buf, size_t count)
{
	int i, ret = 0;
	char *tmpbuf;
	struct ufs_geo tmpgeo;

	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

	memcpy(tmpbuf, buf, count);
	memcpy(&tmpgeo, &geo, sizeof(struct ufs_geo));
	for (i=0; i < count; i++)
		if (tmpbuf[i] == '\t' || tmpbuf[i] == '\r' || tmpbuf[i] == '\n')
			tmpbuf[i] = 0x20;
	
	
	kfree(tmpbuf);

out:
	return ret;
}

static int __parse_config_cfg_grp(const char *buf, size_t count)
{
	int i, ret = 0;
	char *tmpbuf;
	struct ufs_geo tmpgeo;

	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

	memcpy(tmpbuf, buf, count);
	memcpy(&tmpgeo, &geo, sizeof(struct ufs_geo));
	for (i=0; i < count; i++)
		if (tmpbuf[i] == '\t' || tmpbuf[i] == '\r' || tmpbuf[i] == '\n')
			tmpbuf[i] = 0x20;
	
	
	kfree(tmpbuf);

out:
	return ret;
}

static int __parse_config_l2p_tbl(const char *buf, size_t count)
{
	int i, ret = 0;
	char *tmpbuf;
	struct ufs_geo tmpgeo;

	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

	memcpy(tmpbuf, buf, count);
	memcpy(&tmpgeo, &geo, sizeof(struct ufs_geo));
	for (i=0; i < count; i++)
		if (tmpbuf[i] == '\t' || tmpbuf[i] == '\r' || tmpbuf[i] == '\n')
			tmpbuf[i] = 0x20;
	
	
	kfree(tmpbuf);

out:
	return ret;
}

static ssize_t __show_ufs_geo(char *buf)
{
	int ret = 0;
	
	ret = sprintf(buf, "\
	Version                         =%#04x\n\
	Vendor NVM Opcode Command Set   =%#04x\n\
	Configuration Groups            =%#04x\n\
	Capabilities                    =%#010x\n\
	Device Op Mode                  =%#010x\n", 
	geo.version, geo.vnvmt, geo.cgrps, geo.cap, geo.dom);

	return ret;
}

static ssize_t __show_ppa_fmt(char *buf)
{
	int ret = 0;
	
	ret = sprintf(buf, "\
	Channel bit start       =%#04x\n\
	Channel bit length      =%#04x\n\
	LUN bit start           =%#04x\n\
	LUN bit length          =%#04x\n\
	Plane bit start         =%#04x\n\
	Plane bit length        =%#04x\n\
	Block bit start         =%#04x\n\
	Block bit length        =%#04x\n\
	Page bit start          =%#04x\n\
	Page bit length         =%#04x\n\
	Sector bit start        =%#04x\n\
	Sector bit length       =%#04x\n",
	geo.ppaf.ch_off, geo.ppaf.ch_len, geo.ppaf.lun_off,
	geo.ppaf.lun_len, geo.ppaf.pln_off, geo.ppaf.pln_len,
	geo.ppaf.blk_off, geo.ppaf.blk_len, geo.ppaf.pg_off, 
	geo.ppaf.pg_len, geo.ppaf.sect_off, geo.ppaf.sect_len);

	return ret;
}

static ssize_t __show_cfg_grp(char *buf)
{
	int ret = 0;
	
	ret = sprintf(buf, "\
	Media Type                          =%#04x\n\
	Flash Media Type                    =%#04x\n\
	Number of Channels                  =%#04x\n\
	Number of LUNs                      =%#04x\n\
	Number of Planes                    =%#04x\n\
	Number of Blocks                    =%#06x\n\
	Number of Pages                     =%#06x\n\
	Page Size                           =%#06x\n\
	Controller Sector Size              =%#06x\n\
	Sector OOB Size                     =%#06x\n\
	tRD Typical                         =%#010x\n\
	tRD Max                             =%#010x\n\
	tPROG Typical                       =%#010x\n\
	tPROG Max                           =%#010x\n\
	tBERS Typical                       =%#010x\n\
	tBERS Max                           =%#010x\n\
	Multi-plane Operation Support       =%#010x\n\
	Media and Controller Capabilities   =%#010x\n\
	Channel Parallelism                 =%#06x\n",
	geo.ggrp.mtype, geo.ggrp.fmtype, geo.ggrp.num_ch,
	geo.ggrp.num_lun, geo.ggrp.num_pln, geo.ggrp.num_blk,
	geo.ggrp.num_pg, geo.ggrp.fpg_sz, geo.ggrp.csecs,
	geo.ggrp.sos, geo.ggrp.trdt, geo.ggrp.trdm, geo.ggrp.tprt, 
	geo.ggrp.tprm, geo.ggrp.tbet, geo.ggrp.tbem, geo.ggrp.mpos, 
	geo.ggrp.mccap, geo.ggrp.cpar);

	return ret;
}

static ssize_t __show_l2p_tbl(char *buf)
{
	char hex[2022];
	char *src, *dst;
	int i, j, ret=0;
	src = geo.ggrp.l2ptbl.mlc.pairs;
	dst = hex;
	
	for (i=0; i < 27; i++) {
		for (j=0; j < 8; j++) {
			bin2hex(dst, src, 4);
			src = src + 4;
			dst = dst + 8;
			*(dst++) = 0x20;
		}
		*(dst++) = 0x0a;
	}
	for (i=0; i < 5; i++) {
		bin2hex(dst, src, 4);
		src = src + 4;
		dst = dst + 8;
		*(dst++) = 0x20;
	}
	bin2hex(dst, src, 2);
	dst = dst + 4;
	*(dst++) = 0x0a;
	*dst = 0;

	ret = sprintf(buf, "\
	ID codes for READ ID command    =%#018llx\n\
	Number of Pairs                 =%06x\n\
	Pairs Values in Hexadecimal:\n%s\n", 
	(u64)geo.ggrp.l2ptbl.id, geo.ggrp.l2ptbl.mlc.num_pairs, hex);

	return ret;
}

static inline int __store_ufs_geo(const char *buf, size_t count) 
{
	pr_info("RAMUFS: __parse_config_ufs_geo, buffer size= %lld\n", count);
	return __parse_config_ufs_geo(buf, count);
}

static inline int __store_ppa_fmt(const char *buf, size_t count) 
{
	pr_info("RAMUFS: __parse_config_ppa_fmt, buffer size= %lld\n", count);
	return __parse_config_ppa_fmt(buf, count);
}

static inline int __store_cfg_grp(const char *buf, size_t count) 
{
	pr_info("RAMUFS: __parse_config_cfg_grp, buffer size= %lld\n", count);
	return __parse_config_cfg_grp(buf, count);
}

static inline int __store_l2p_tbl(const char *buf, size_t count) 
{
	pr_info("RAMUFS: __parse_config_l2p_tbl, buffer size= %lld\n", count);
	return __parse_config_l2p_tbl(buf, count);
}

static ssize_t ramufs_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	const char *name = attr->attr.name;
	
	if (strcmp(name, "ufs_geo") == 0) {
		pr_info("RAMUFS: show geometry\n");
		return __show_ufs_geo(buf);
	} else if (strcmp(name, "ppa_fmt") == 0) {
		pr_info("RAMUFS: show ppa format\n");
		return __show_ppa_fmt(buf);
	} else if (strcmp(name, "cfg_grp") == 0) {
		pr_info("RAMUFS: show configuration group\n");
		return __show_cfg_grp(buf);
	} else if (strcmp(name, "l2p_tbl") == 0) {
		pr_info("RAMUFS: show l2p table\n");
		return __show_l2p_tbl(buf);
	}
	return sprintf(buf, "Unhandled attr(%s) in `ramufs_show`\n", name);
}

static ssize_t ramufs_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;
	const char *name = attr->attr.name;
	
	if (strcmp(name, "ufs_geo") == 0) {
		pr_info("RAMUFS: set geometry\n");
		ret = __store_ufs_geo(buf, count);
	} else if (strcmp(name, "ppa_fmt") == 0) {
		pr_info("RAMUFS: set ppa format\n");
		ret = __store_ppa_fmt(buf, count);
	} else if (strcmp(name, "cfg_grp") == 0) {
		pr_info("RAMUFS: set configuration group\n");
		ret = __store_cfg_grp(buf, count);
	} else if (strcmp(name, "l2p_tbl") == 0) {
		pr_info("RAMUFS: set l2p table\n");
		ret = __store_l2p_tbl(buf, count);
	} else {
		ret = -EINVAL;
	}
	
	return ret < 0 ? ret : count;
}

#define RAMUFS_ATTR(_name)			\
static struct kobj_attribute ramufs_attr_##_name =	\
	__ATTR(_name, 0664, ramufs_show, ramufs_store)

RAMUFS_ATTR(ufs_geo);
RAMUFS_ATTR(ppa_fmt);
RAMUFS_ATTR(cfg_grp);
RAMUFS_ATTR(l2p_tbl);

static struct attribute *ramufs_attrs[] = {
	&ramufs_attr_ufs_geo.attr,
	&ramufs_attr_ppa_fmt.attr,
	&ramufs_attr_cfg_grp.attr,
	&ramufs_attr_l2p_tbl.attr,
	NULL,	
};

static struct attribute_group ramufs_attr_group = {
	.attrs = ramufs_attrs,
};

static int __init ramufs_init(void)
{
	int ret = 0;

	pr_info("RAMUFS: ramufs init\n");
	ufs = kzalloc(sizeof(struct ramufs), GFP_KERNEL);
	if (!ufs) {
		pr_err("RAMUFS: kzalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

	ufs->kobj = kobject_create_and_add("ramufs", kernel_kobj);
	if (!(ufs->kobj)) {
		pr_err("RAMUFS: kobject_create_and_add failed, out of memory\n");
		ret = -ENOMEM;
		goto free_ufs;
	}
	ret = sysfs_create_group(ufs->kobj, &ramufs_attr_group);
	if (ret) {
		pr_err("RAMUFS: sysfs_create_group failed\n");
		kobject_put(ufs->kobj);
		goto free_ufs;
	}

	pr_info("RAMUFS: ramufs loaded\n");
	return ret;

free_ufs:
	kfree(ufs);
out:
	return ret;
}

static void __exit ramufs_exit(void)
{
	pr_info("RAMUFS: ramufs exit\n");
	sysfs_remove_group(ufs->kobj, &ramufs_attr_group);
	kobject_put(ufs->kobj);
	kfree(ufs);
}

module_init(ramufs_init);
module_exit(ramufs_exit);

MODULE_AUTHOR("Gnekiah Hsiung <gnekiah@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UFS host controller ram-disk driver");
