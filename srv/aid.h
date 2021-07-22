#ifndef __AID_H
#define __AID_H

struct saveparam {
    time_t seconds;	/* N秒, 数据被修改了N次 */
    int changes;
};

void redisLog(int level, const char *fmt, ...)
void oom(const char *msg)
void closeTimedoutClients(void)
void tryResizeHashTables(void)
void appendServerSaveParams(time_t seconds, int changes)
void ResetServerSaveParams()
void initServerConfig()

#endif
