#ifndef __REDIS_DB_H
#define __REDIS_DB_H

int rdbSaveType(FILE *fp, unsigned char type);
int rdbSaveTime(FILE *fp, time_t t);
int rdbSaveLen(FILE *fp, uint32_t len);
int rdbTryIntegerEncoding(sds s, unsigned char *enc);
int rdbSaveLzfStringObject(FILE *fp, robj *obj);
int rdbSaveStringObject(FILE *fp, robj *obj);
int rdbSave(char *filename);
int rdbSaveBackground(char *filename);
int rdbLoadType(FILE *fp);
time_t rdbLoadTime(FILE *fp);
uint32_t rdbLoadLen(FILE *fp, int rdbver, int *isencoded);
robj *rdbLoadIntegerObject(FILE *fp, int enctype);
robj *rdbLoadLzfStringObject(FILE*fp, int rdbver);
robj *rdbLoadStringObject(FILE*fp, int rdbver);
rdbLoad(char *filename);

#endif
