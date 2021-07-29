#include "aid.h"

extern struct redisServer server; /* server global state */
extern struct redisCommand cmdTable[];

/* 尝试查找name对应的命令处理句柄 */
struct redisCommand *lookupCommand(char *name) {
    int j = 0;
    while(cmdTable[j].name != NULL) {
        if (!strcasecmp(name,cmdTable[j].name)) return &cmdTable[j];
        j++;
    }
    return NULL;
}

