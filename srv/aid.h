#ifndef __AID_H
#define __AID_H

struct saveparam {
    time_t seconds;	/* N秒, 数据被修改了N次 */
    int changes;
};

int expireIfNeeded(redisDb *db, robj *key);
int removeExpire(redisDb *db, robj *key);
robj *createStringObject(char *ptr, size_t len);
robj *createObject(int type, void *ptr);
void freeStringObject(robj *o);
void freeListObject(robj *o);
void freeSetObject(robj *o);
void decrRefCount(void *o);
void incrRefCount(robj *o);
void redisLog(int level, const char *fmt, ...);
void oom(const char *msg);
void closeTimedoutClients(void);
void tryResizeHashTables(void);
void appendServerSaveParams(time_t seconds, int changes);
void ResetServerSaveParams();
void initServerConfig();
long long emptyDb();
int yesnotoi(char *s);
void loadServerConfig(char *filename);
void glueReplyBuffersIfNeeded(redisClient *c);
void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask);
void freeClientArgv(redisClient *c);
void resetClient(redisClient *c);
int selectDb(redisClient *c, int id)
robj *createObject(int type, void *ptr)
int sdsDictKeyCompare(void *privdata, const void *key1,
			const void *key2);
void dictRedisObjectDestructor(void *privdata, void *val);
int dictSdsKeyCompare(void *privdata, const void *key1,
				const void *key2);
unsigned int dictSdsHash(const void *key);

#endif
