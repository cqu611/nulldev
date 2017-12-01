#include "ramufs.h"

static const struct ufs_geo_config_attr_tbl prs_cfg_ufs_geo[] = {
	{ "version", 0, 0 },
	{ "vnvmt", 0, 0 },
	{ "cgrps", 0, 0 },
	{ "cap", 0, 0 },
	{ "dom", 0, 0 },
	{ NULL, 0, 0 }
};

/**
 * matching configure keys
 * buf: buffer input
 * val: key (string type) to matched
 * pos: current position in buf
 * len: length of buf
 */
static int parse_config_parse_key(const char *buf, char *val, int pos, int len)
{
	char tmpbuf[16];
	int valen = strlen(val);
	
	/* out of range */
	if (valen + pos > len || valen >= 16) 
		return RU_PARSE_STATUS_RANGED;

	memcpy(tmpbuf, &buf[pos], valen);
	tmpbuf[valen] = 0;

	return strcmp(tmpbuf,val)==0 ? RU_PARSE_STATUS_MATCHED : RU_PARSE_STATUS_UNMATCH;
}

/**
 * parse value for a key
 * buf: buffer input
 * pos: current position in buf
 * val: to save value
 * off: offset in traversal, this used for calculate offset
 * len: length of buf
 * cnt: count of 'val' by bytes. e.g. u8(1), u16(2), u32(4), u64(8)
 */
static int parse_config_parse_value(const char *buf, int pos, void *val, 
				int *off, int len, int cnt)
{
	int i, j=0, flag=0, qflag=0;
	int ret, tmpval;
	char tmpbuf[17];

	for (i=0; pos + i < len; i++) {
		/* out of range */
		if (j > 17)
			return RU_PARSE_STATUS_ERROR;

		if (buf[pos+i] == 0x3d) {
			if (flag == 1 || qflag == 1)
				ret = -EINVAL;
			qflag = 1;
			continue;
		}
		
		if (buf[pos+i] != 0x20 && buf[pos+i] != 0 && buf[pos+i] != 0x3d) {
			if (qflag == 0)
				ret = -EINVAL;
			flag = 1;
			tmpbuf[j++] = buf[pos+i];
			continue;
		}
		
		/* to filter space behind hex-string */
		if (buf[pos+i] == 0x20 && flag == 0) continue;
		/* terminated */
		tmpbuf[j] = 0;
		ret = kstrtoint(tmpbuf, 16, &tmpval);
		
		if (ret || j == 0)	return RU_PARSE_STATUS_ERROR;		
		if (cnt == 1)		*(u8*)val = (u8)tmpval;
		else if (cnt == 2)	*(u16*)val = (u16)tmpval;
		else if (cnt == 4)	*(u32*)val = (u32)tmpval;
		else if (cnt == 8)	*(u64*)val = (u64)tmpval;
		else				return RU_PARSE_STATUS_ERROR;
		
		*off = i;
		return RU_PARSE_STATUS_SPACE;
	} 
	
	return RU_PARSE_STATUS_ERROR;
}

static inline void parse_config_init(char *tmpbuf, const char *buf, int count)
{
	int i;
	
	memcpy(tmpbuf, buf, count);
	for (i=0; i < count; i++) {
		tmpbuf[i] = tolower(tmpbuf[i]);
		if (tmpbuf[i] == '\t' || tmpbuf[i] == '\r' || tmpbuf[i] == '\n')
			tmpbuf[i] = 0x20;
	}
}

/**
 * buf: buffer input
 * pos: current position in buffer
 * count: length of buffer
 * geo: to save value
 */
static int parse_config_ufs_geo_v(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=7;
	u8 val;

	ret = parse_config_parse_key(buf, "version", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		keylen = 5;
		ret = parse_config_parse_key(buf, "vnvmt", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_RANGED || ret == RU_PARSE_STATUS_UNMATCH)
		return ret;

	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, 1);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (keylen == 7)
			geo->version = val;
		else
			geo->vnvmt = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ufs_geo_v, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_ufs_geo_c(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{	
	int ret, offset, keylen=5, valtyp=1;
	u32 val;

	ret = parse_config_parse_key(buf, "cgrps", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		keylen = 3;
		valtyp = 4;
		ret = parse_config_parse_key(buf, "cap", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_RANGED || ret == RU_PARSE_STATUS_UNMATCH)
		return ret;

	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (keylen == 5)
			geo->cgrps = (u8)val;
		else
			geo->cap = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ufs_geo_c, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_ufs_geo_d(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=3, valtyp=4;
	u32 val;

	ret = parse_config_parse_key(buf, "dom", *pos, count);
	if (ret == RU_PARSE_STATUS_RANGED || ret == RU_PARSE_STATUS_UNMATCH)
		return ret;

	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		geo->dom = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ufs_geo_d, bad status: %d\n", ret);
		return -EINVAL;
	}
}

int __parse_config_ufs_geo(const char *buf, size_t count, struct ufs_geo *geo)
{
	int i, ret = 0, status;
	char *tmpbuf;

	pr_err("geo addr=%u\n", geo);
	pr_err("version addr=%u\n", &geo->version);
	pr_err("vnvmt addr=%u\n", &geo->vnvmt);
	pr_err("cgrps addr=%u\n", &geo->cgrps);
	pr_err("cap addr=%u\n", &geo->cap);
	pr_err("dom addr=%u\n", &geo->dom);

	pr_err("ch_off addr=%u\n", &geo->ppaf.ch_off);
	pr_err("ch_len addr=%u\n", &geo->ppaf.ch_len);
	pr_err("lun_off addr=%u\n", &geo->ppaf.lun_off);
	pr_err("lun_len addr=%u\n", &geo->ppaf.lun_len);
	pr_err("pln_off addr=%u\n", &geo->ppaf.pln_off);
	pr_err("pln_len addr=%u\n", &geo->ppaf.pln_len);
	pr_err("blk_off addr=%u\n", &geo->ppaf.blk_off);
	pr_err("blk_len addr=%u\n", &geo->ppaf.blk_len);
	pr_err("pg_off addr=%u\n", &geo->ppaf.pg_off);
	pr_err("pg_len addr=%u\n", &geo->ppaf.pg_len);
	pr_err("sect_off addr=%u\n", &geo->ppaf.sect_off);
	pr_err("sect_len addr=%u\n", &geo->ppaf.sect_len);

	pr_err("mtype addr=%u\n", &geo->ggrp.mtype);
	pr_err("fmtype addr=%u\n", &geo->ggrp.fmtype);
	pr_err("num_ch addr=%u\n", &geo->ggrp.num_ch);
	pr_err("num_lun addr=%u\n", &geo->ggrp.num_lun);
	pr_err("num_pln addr=%u\n", &geo->ggrp.num_pln);
	pr_err("num_blk addr=%u\n", &geo->ggrp.num_blk);
	pr_err("num_pg addr=%u\n", &geo->ggrp.num_pg);
	pr_err("fpg_sz addr=%u\n", &geo->ggrp.fpg_sz);
	pr_err("csecs addr=%u\n", &geo->ggrp.csecs);
	pr_err("sos addr=%u\n", &geo->ggrp.sos);
	pr_err("trdt addr=%u\n", &geo->ggrp.trdt);
	pr_err("trdm addr=%u\n", &geo->ggrp.trdm);
	pr_err("tprt addr=%u\n", &geo->ggrp.tprt);
	pr_err("tprm addr=%u\n", &geo->ggrp.tprm);
	pr_err("tbet addr=%u\n", &geo->ggrp.tbet);
	pr_err("tbem addr=%u\n", &geo->ggrp.tbem);
	pr_err("mpos addr=%u\n", &geo->ggrp.mpos);
	pr_err("mccap addr=%u\n", &geo->ggrp.mccap);
	pr_err("cpar addr=%u\n", &geo->ggrp.cpar);

	pr_err("id addr=%u\n", &geo->ggrp.l2ptbl.id);
	pr_err("num_pairs addr=%u\n", &geo->ggrp.l2ptbl.mlc.num_pairs);
	pr_err("pairs addr=%u\n", &geo->ggrp.l2ptbl.mlc.pairs);
	
	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}	
	parse_config_init(tmpbuf, buf, count);

	/* begin to parse input string */
	status = RU_PARSE_STATUS_SPACE;
	for (i=0; i < count; i++) {
		if (status != RU_PARSE_STATUS_SPACE) {
			ret = -EINVAL;
			goto destroy_buf;
		}
		if (tmpbuf[i] == 'v')
			status = parse_config_ufs_geo_v(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'c') 
			status = parse_config_ufs_geo_c(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'd')
			status = parse_config_ufs_geo_d(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 0x20)
			continue;
		else {
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

static int parse_config_ppa_fmt_c(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=6, valtyp=1, nr=0;
	u8 val;

	ret = parse_config_parse_key(buf, "ch_off", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		ret = parse_config_parse_key(buf, "ch_len", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ppaf.ch_off = val;
		else
			geo->ppaf.ch_len = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ppa_fmt_c, bad status: %d\n", ret);
		return -EINVAL;
	}
}


static int parse_config_ppa_fmt_l(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=7, valtyp=1, nr=0;
	u8 val;

	ret = parse_config_parse_key(buf, "lun_off", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		ret = parse_config_parse_key(buf, "lun_len", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ppaf.lun_off = val;
		else
			geo->ppaf.lun_len = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ppa_fmt_l, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_ppa_fmt_p(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=7, valtyp=1, nr=0;
	u8 val;

	ret = parse_config_parse_key(buf, "pln_off", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		ret = parse_config_parse_key(buf, "pln_len", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 2;
		keylen = 6;
		ret = parse_config_parse_key(buf, "pg_off", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 3;
		keylen = 6;
		ret = parse_config_parse_key(buf, "pg_len", *pos, count);
	}
	
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ppaf.pln_off = val;
		else if (nr == 1)
			geo->ppaf.pln_len = val;
		else if (nr == 2)
			geo->ppaf.pg_off = val;
		else
			geo->ppaf.pg_len = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ppa_fmt_p, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_ppa_fmt_b(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=7, valtyp=1, nr=0;
	u8 val;

	ret = parse_config_parse_key(buf, "blk_off", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		ret = parse_config_parse_key(buf, "blk_len", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ppaf.blk_off = val;
		else
			geo->ppaf.blk_len = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ppa_fmt_b, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_ppa_fmt_s(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=8, valtyp=1, nr=0;
	u8 val;

	ret = parse_config_parse_key(buf, "sect_off", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		ret = parse_config_parse_key(buf, "sect_len", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ppaf.sect_off = val;
		else
			geo->ppaf.sect_len = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_ppa_fmt_s, bad status: %d\n", ret);
		return -EINVAL;
	}
}

int __parse_config_ppa_fmt(const char *buf, size_t count, struct ufs_geo *geo)
{
	int i, ret = 0, status;
	char *tmpbuf;
	
	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}	
	parse_config_init(tmpbuf, buf, count);

	/* begin to parse input string */
	status = RU_PARSE_STATUS_SPACE;
	for (i=0; i < count; i++) {
		if (status != RU_PARSE_STATUS_SPACE) {
			ret = -EINVAL;
			goto destroy_buf;
		}
		if (tmpbuf[i] == 'c')
			status = parse_config_ppa_fmt_c(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'l') 
			status = parse_config_ppa_fmt_l(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'p')
			status = parse_config_ppa_fmt_p(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'b')
			status = parse_config_ppa_fmt_b(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 's')
			status = parse_config_ppa_fmt_s(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 0x20)
			continue;
		else {
			pr_err("RAMUFS: __parse_config_ppa_fmt, bad status: %d\n", status);
			ret = -EINVAL;
			goto destroy_buf;
		}
	}

destroy_buf:
	kfree(tmpbuf);
out:
	return ret;
}

static int parse_config_cfg_grp_m(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=5, valtyp=1, nr=0;
	u32 val;

	ret = parse_config_parse_key(buf, "mtype", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		valtyp = 4;
		ret = parse_config_parse_key(buf, "mccap", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 2;
		valtyp = 4;
		keylen = 4;
		ret = parse_config_parse_key(buf, "mpos", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ggrp.mtype = (u8)val;
		else if (nr == 1)
			geo->ggrp.mccap = val;
		else 
			geo->ggrp.mpos = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_cfg_grp_m, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_cfg_grp_f(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=6, valtyp=1, nr=0;
	u32 val;

	ret = parse_config_parse_key(buf, "fmtype", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		valtyp = 2;
		ret = parse_config_parse_key(buf, "fpg_sz", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ggrp.fmtype = (u8)val;
		else 
			geo->ggrp.fpg_sz = (u16)val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_cfg_grp_f, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_cfg_grp_n(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=6, valtyp=1, nr=0;
	u32 val;

	ret = parse_config_parse_key(buf, "num_ch", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		keylen = 7;
		ret = parse_config_parse_key(buf, "num_lun", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 2;
		keylen = 7;
		ret = parse_config_parse_key(buf, "num_pln", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 3;
		valtyp = 2;
		keylen = 7;
		ret = parse_config_parse_key(buf, "num_blk", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 4;
		valtyp = 2;
		keylen = 6;
		ret = parse_config_parse_key(buf, "num_pg", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ggrp.num_ch = (u8)val;
		else if (nr == 1)
			geo->ggrp.num_lun = (u8)val;
		else if (nr == 2)
			geo->ggrp.num_pln = (u8)val;
		else if (nr == 3)
			geo->ggrp.num_blk = (u16)val;
		else
			geo->ggrp.num_pg = (u16)val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_cfg_grp_n, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_cfg_grp_c(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=5, valtyp=2, nr=0;
	u32 val;

	ret = parse_config_parse_key(buf, "csecs", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		keylen = 4;
		ret = parse_config_parse_key(buf, "cpar", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ggrp.csecs = (u16)val;
		else 
			geo->ggrp.cpar = (u16)val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_cfg_grp_c, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_cfg_grp_s(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=3, valtyp=2;
	u32 val;

	ret = parse_config_parse_key(buf, "sos", *pos, count);
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		geo->ggrp.sos = (u16)val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_cfg_grp_s, bad status: %d\n", ret);
		return -EINVAL;
	}
}

static int parse_config_cfg_grp_t(const char *buf, int *pos,
				size_t count, struct ufs_geo *geo)
{
	int ret, offset, keylen=4, valtyp=4, nr=0;
	u32 val;

	ret = parse_config_parse_key(buf, "trdt", *pos, count);
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 1;
		ret = parse_config_parse_key(buf, "trdm", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 2;
		ret = parse_config_parse_key(buf, "tprt", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 3;
		ret = parse_config_parse_key(buf, "tprm", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 4;
		ret = parse_config_parse_key(buf, "tbet", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_UNMATCH) {
		nr = 5;
		ret = parse_config_parse_key(buf, "tbem", *pos, count);
	}
	if (ret == RU_PARSE_STATUS_MATCHED) {
		*pos += keylen;
		ret = parse_config_parse_value(buf, *pos, &val, &offset, count, valtyp);
		if (ret == RU_PARSE_STATUS_ERROR)
			return ret;
		*pos += offset;
		if (nr == 0) 
			geo->ggrp.trdt = val;
		else if (nr == 1)
			geo->ggrp.trdm = val;
		else if (nr == 2)
			geo->ggrp.tprt = val;
		else if (nr == 3)
			geo->ggrp.tprm = val;
		else if (nr == 4)
			geo->ggrp.tbet = val;
		else 
			geo->ggrp.tbem = val;
		return RU_PARSE_STATUS_SPACE;
	} else {
		pr_err("RAMUFS: parse_config_cfg_grp_t, bad status: %d\n", ret);
		return -EINVAL;
	}
}

int __parse_config_cfg_grp(const char *buf, size_t count, struct ufs_geo *geo)
{
	int i, ret = 0, status;
	char *tmpbuf;
	
	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}	
	parse_config_init(tmpbuf, buf, count);

	/* begin to parse input string */
	status = RU_PARSE_STATUS_SPACE;
	for (i=0; i < count; i++) {
		if (status != RU_PARSE_STATUS_SPACE) {
			ret = -EINVAL;
			goto destroy_buf;
		}
		if (tmpbuf[i] == 'm')
			status = parse_config_cfg_grp_m(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'f') 
			status = parse_config_cfg_grp_f(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'n')
			status = parse_config_cfg_grp_n(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 'c')
			status = parse_config_cfg_grp_c(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 's')
			status = parse_config_cfg_grp_s(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 't')
			status = parse_config_cfg_grp_t(tmpbuf, &i, count, geo);
		else if (tmpbuf[i] == 0x20)
			continue;
		else {
			pr_err("RAMUFS: __parse_config_cfg_grp, bad status: %d\n", status);
			ret = -EINVAL;
			goto destroy_buf;
		}
	}

destroy_buf:
	kfree(tmpbuf);
out:
	return ret;
}

int __parse_config_l2p_tbl(const char *buf, size_t count, struct ufs_geo *geo)
{
	int i, ret = 0;
	char *tmpbuf;

	tmpbuf = kmalloc(count, GFP_KERNEL);
	if (!tmpbuf) {
		pr_err("RAMUFS: kmalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

	memcpy(tmpbuf, buf, count);
	for (i=0; i < count; i++)
		if (tmpbuf[i] == '\t' || tmpbuf[i] == '\r' || tmpbuf[i] == '\n')
			tmpbuf[i] = 0x20;
	
	
	kfree(tmpbuf);

out:
	return ret;
}


