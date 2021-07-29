/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2009, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SDS_H
#define __SDS_H

#include <sys/types.h>

typedef char *sds;

/* 
 * context : str = "hello"
 * len  = 5
 * free = 5
 * 整个byte.space=5+1+5   自动多一个字节出来用于保存'\0'
 * buf放在结构体的最后，这个设计是故意的，其它的函数对此有依赖，不能随意改动
 */
struct sdshdr {
    long len;		/* 已使用的空间长度（不包含'\0' */
    long free;		/* 空间预分配，惰性释放 */
    char buf[]; 	/* 字节空间的首地址 */
};

/* 构建包含指定长度数据的sds(数据内部可以有'\0') */
sds sdsnewlen(const void *init, size_t initlen);
/* 构建指定数据的sds(数据内部不得有'\0') */
sds sdsnew(const char *init);
/* 构建一个"空字符串"的sds */
sds sdsempty();
/* 存储的数据长度(不包含自动添加的\0) */
size_t sdslen(const sds s);
/* 复制sds */
sds sdsdup(const sds s);
/* 释放sds */
void sdsfree(sds s);
/* 剩余的free空间大小 */
size_t sdsavail(sds s);
/* 将指定长度的数据t,len拷贝到s的尾处 */
sds sdscatlen(sds s, void *t, size_t len);
sds sdscat(sds s, char *t);
/* 放弃s原来的数据，将指定长度的数据拷贝到s的头处 */
sds sdscpylen(sds s, char *t, size_t len);
sds sdscpy(sds s, char *t);
/* 将带有格式的数据拷贝到s的尾处 */
sds sdscatprintf(sds s, const char *fmt, ...);
/* 对s的两端进行修剪，移除cset中的内容 */
sds sdstrim(sds s, const char *cset);
/* 截取原始的部分内容 */
sds sdsrange(sds s, long start, long end);
void sdsupdatelen(sds s);
/* 比较两个s1,s2,当前仅当内容，长度相等才return.0 */
int sdscmp(sds s1, sds s2);
sds *sdssplitlen(char *s, int len, char *sep, int seplen, int *count);
void sdstolower(sds s);

#endif
