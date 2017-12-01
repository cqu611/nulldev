#include "ramufs.h"

/**
 * matching configure keys
 * buf: buffer input
 * val: key (string type) to matched
 * pos: current position in buf
 * len: length of buf
 */
static int __parse_config_parse_key(char *buf, char *val, int pos, int len)
{
	char tmpbuf[16];
	int valen = strlen(val);
	
	/* out of range */
	if (valen + pos > len || valen >= 16) 
		return RU_PARSE_STATUS_RANGED;

	memcpy(tmpbuf, &buf[pos], valen);
	tmpbuf[valen] = 0;

	return strcmp(tmpbuf, val) == 0 ? RU_PARSE_STATUS_MATCHED : RU_PARSE_STATUS_UNMATCH;
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
static int __parse_config_parse_value(char *buf, int pos, void *val, 
				int *off, int len, int cnt)
{
	int i, j=0, flag=0, ret;
	u8 *val8;
	u16 *val16;
	u32 *val32;
	u64 *val64;
	char tmpbuf[32], dst[8];

	memset(dst, 0, 8);
	for (i=0; pos + i < len; i++) {
		if (buf[pos+i] == 0x20 || buf[pos+i] == 0) {
			if (flag == 0) {
				continue;
			} else {	/* terminated */
				if (j == 0 || j > 17)
					return RU_PARSE_STATUS_ERROR;
				tmpbuf[j] = 0;

				pr_err("j=%d, i=%d, buf=%s, tmpbuf=%s\n", j, i, buf, tmpbuf);
				
				ret = hex2bin(dst, tmpbuf, j/2);
				if (ret)
					return RU_PARSE_STATUS_ERROR;

				pr_err("dst=%d,%d,%d,%d,%d,%d,%d,%d\n", dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],dst[6],dst[7]);
				
				if (cnt == 1) {
					val8 = (u8*)val;
					pr_err("&val=%d\n", val8);
					*val8 = (u8)dst[0];
				}
				else if (cnt == 2) {
					val16 = (u16*)val;
					pr_err("&val=%d\n", val16);
					*val16 = ((u16)dst[0] << 8) | (u16)dst[1];
				}
				else if (cnt == 4) {
					val32 = (u32*)val;
					pr_err("&val=%d\n", val32);
					*val32 = ((u32)dst[0] << 24) | ((u32)dst[1] << 16)
							| ((u32)dst[2] << 8) | (u32)dst[3];
				}
				else if (cnt == 8) {
					val64 = (u64*)val;
					pr_err("&val=%d\n", val64);
					*val64 = ((u64)dst[0] << 56) | ((u64)dst[1] << 48)
							| ((u64)dst[2] << 40) | ((u64)dst[3] << 32)
							| ((u64)dst[4] << 24) | ((u64)dst[5] << 16)
							| ((u64)dst[6] << 8) | (u64)dst[7];
				}
				else
					return RU_PARSE_STATUS_ERROR;
				*off = i;
				return RU_PARSE_STATUS_SPACE;
			}
		} else {
			flag = 1;
			tmpbuf[j++] = buf[pos+i];
		}
	}
	
	return RU_PARSE_STATUS_ERROR;
}


static void __test__(void)
{
	char *a1 = "abcd    ";
	char *a2 = "abcdefgh";
	char *a3 = "abcdefgh";
	char *a4 = "12asbt 4";
	char *b1 = "12      ";
	char *b2 = " 45a4 ad";
	char *b3 = "  456b  ";
	char *b4 = " 34efe9 ";
	int status;
	u8 val8;
	u16 val16;
	u32 val32;
	u64 val64,
	int valen;

	status = __parse_config_parse_key(a1, "abcd", 0, 9);
	pr_err("a1=%d\n", status);
	status = __parse_config_parse_key(a2, "cde", 2, 9);
	pr_err("a2=%d\n", status);
	status = __parse_config_parse_key(a3, "cde", 0, 9);
	pr_err("a3=%d\n", status);
	status = __parse_config_parse_key(a4, "as", 2, 9);
	pr_err("a4=%d\n", status);

	status = __parse_config_parse_value(b1, 0, &val8, &valen, 9, 1);
	pr_err("b1=%d, value=%u, valen=%d, &value=%d, &valen=%d\n", status, val8, valen, &val8, &valen);
	status = __parse_config_parse_value(b2, 1, &val16, &valen, 9, 2);
	pr_err("b2=%d, value=%u, valen=%d, &value=%d, &valen=%d\n", status, val16, valen, &val16, &valen);
	status = __parse_config_parse_value(b3, 2, &val32, &valen, 9, 4);
	pr_err("b3=%d, value=%u, valen=%d, &value=%d, &valen=%d\n", status, val32, valen, &val32, &valen);
	status = __parse_config_parse_value(b4, 1, &val64, &valen, 9, 8);
	pr_err("b4=%d, value=%lu, valen=%d, &value=%d, &valen=%d\n", status, val64, valen, &val64, &valen);
}

int __parse_config_ufs_geo(const char *buf, size_t count, struct ufs_geo *geo)
{
	int i, ret = 0, status;
	char *tmpbuf;
	u8 tmp_ver, tmp_vnvmt, tmp_cgrps;
	u32 tmp_cap, tmp_dom;
	void *val;							/* point to tmp_ver, tmp_vnvmt, .... */
	int keylen=0, vallen=0, typelen=0;	/* key ,value and type length */
	val = &tmp_ver;						/* set default value */

	__test__();
	return 0;

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
			} else if (status == RU_PARSE_STATUS_MATCHED) {
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
			} else if (status == RU_PARSE_STATUS_MATCHED) {
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
			} else if (status == RU_PARSE_STATUS_MATCHED) {
				i += keylen - 1;
				continue;
			} else {
				pr_err("RAMUFS: __parse_config_ufs_geo, bad status: %d\n", status);
				ret = -EINVAL;
				goto destroy_buf;
			}
		} else if (tmpbuf[i] == '=') {

			/* try to parse values */
			if (status != RU_PARSE_STATUS_MATCHED) {
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

int __parse_config_ppa_fmt(const char *buf, size_t count, struct ufs_geo *geo)
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

int __parse_config_cfg_grp(const char *buf, size_t count, struct ufs_geo *geo)
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


