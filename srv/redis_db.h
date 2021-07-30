#ifndef __REDIS_DB_H
#define __REDIS_DB_H

int rdbSaveType(FILE *fp, unsigned char type);
int rdbSaveTime(FILE *fp, time_t t);
int rdbSaveLen(FILE *fp, uint32_t len);
int rdbTryIntegerEncoding(sds s, unsigned char *enc);
int rdbSaveLzfStringObject(FILE *fp, robj *obj);

#endif
