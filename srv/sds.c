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

#include "sds.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "zmalloc.h"

static void sdsOomAbort(void) {
    fprintf(stderr,"SDS: Out Of Memory (SDS_ABORT_ON_OOM defined)\n");
    abort();
}

/* 
** KEYCODE 按照指定的字符串长度申请一个sds
** 
** 不会预留多余的free
** NOTE: initlen可以为0
*/
sds sdsnewlen(const void *init, size_t initlen) {
    struct sdshdr *sh;

	/* 看到这里的内存布局了么？不明白的，看下面的sdslen的实现 */
    sh = zmalloc(sizeof(struct sdshdr)+initlen+1);	/* +1：自动在末位添加\0 */
#ifdef SDS_ABORT_ON_OOM
    if (sh == NULL) sdsOomAbort();
#else
    if (sh == NULL) return NULL;
#endif
    sh->len = initlen;	/* 这里不包含下面自动添加的\0,做到对调用层的透明 */
    sh->free = 0;
    if (initlen) {
        if (init) 
			memcpy(sh->buf, init, initlen);
        else 
			memset(sh->buf,0,initlen);
    }
    sh->buf[initlen] = '\0';	/* 自动添加\0 */
    return (char*)sh->buf;
}

sds sdsempty(void) {
    return sdsnewlen("",0);
}

sds sdsnew(const char *init) {
    /* strlen 不包含末尾的\0 */
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return sdsnewlen(init, initlen);
}

/* 存储的数据长度 */
size_t sdslen(const sds s) {
	/* 这里提取sdshdr的方法可以整理成一个宏定义 */
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    return sh->len;
}

sds sdsdup(const sds s) {
    return sdsnewlen(s, sdslen(s));
}

void sdsfree(sds s) {
    if (s == NULL) return;
    zfree(s-sizeof(struct sdshdr));
}

size_t sdsavail(sds s) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    return sh->free;
}

void sdsupdatelen(sds s) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    int reallen = strlen(s);
    sh->free += (sh->len-reallen);
    sh->len = reallen;
}

/* 准备一个不短于addlen长度的free内存空间 */
static sds sdsMakeRoomFor(sds s, size_t addlen) {
    struct sdshdr *sh, *newsh;
    size_t free = sdsavail(s);
    size_t len, newlen;

    if (free >= addlen) return s;	/* 剩余空间足够，无需另外申请空间，直接返回 */
	
    len = sdslen(s);
    sh = (void*) (s-(sizeof(struct sdshdr)));
    newlen = (len+addlen)*2;	// 直接申请多一倍的冗余空间
    newsh = zrealloc(sh, sizeof(struct sdshdr)+newlen+1);
#ifdef SDS_ABORT_ON_OOM
    if (newsh == NULL) sdsOomAbort();
#else
    if (newsh == NULL) return NULL;
#endif

    newsh->free = newlen - len;
    return newsh->buf;
}

/* 将指定长度len的内存空间tcat到s的尾上 */
sds sdscatlen(sds s, void *t, size_t len) {
    struct sdshdr *sh;
    size_t curlen = sdslen(s);

    s = sdsMakeRoomFor(s,len);	/* 扩展free内存以便增加len */
    if (s == NULL) return NULL;
    sh = (void*) (s-(sizeof(struct sdshdr)));
    memcpy(s+curlen, t, len);
    sh->len = curlen+len;
    sh->free = sh->free-len;
    s[curlen+len] = '\0';		/* 不忘自动添加一个\0 */
    return s;
}

sds sdscat(sds s, char *t) {
    return sdscatlen(s, t, strlen(t));
}

/* 将指定长度的数据t,len复制到指定s的head开始处
** 放弃原来s的数据
*/
sds sdscpylen(sds s, char *t, size_t len) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    size_t totlen = sh->free+sh->len;

    if (totlen < len) {
        s = sdsMakeRoomFor(s,len-totlen);
        if (s == NULL) return NULL;
        sh = (void*) (s-(sizeof(struct sdshdr)));
        totlen = sh->free+sh->len;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    sh->len = len;
    sh->free = totlen-len;
    return s;
}

sds sdscpy(sds s, char *t) {
    return sdscpylen(s, t, strlen(t));
}

sds sdscatprintf(sds s, const char *fmt, ...) {
    va_list ap;
    char *buf, *t;
    size_t buflen = 32;

    while(1) {
        buf = zmalloc(buflen);
#ifdef SDS_ABORT_ON_OOM
        if (buf == NULL) sdsOomAbort();
#else
        if (buf == NULL) return NULL;
#endif
        buf[buflen-2] = '\0';	
        va_start(ap, fmt);
        vsnprintf(buf, buflen, fmt, ap);
        va_end(ap);
		/* 这里buflen-2而不是buflen-1就有意思了吧 */
        if (buf[buflen-2] != '\0') {	
            zfree(buf);
            buflen *= 2;	/* 长度不够，加长一倍,直到找到一个合适的长度 */
            continue;
        }
        break;
    }
    t = sdscat(s, buf);
    zfree(buf);
    return t;
}

/*
**对 sds 左右两端进行修剪(trim)，清除其中 cset 指定的所有字符
**比如 sdsstrim(xxyyabcyyxy, "xy") 将返回 "abc"
*/
sds sdstrim(sds s, const char *cset) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+sdslen(s)-1;
	/* strchr函数功能为在一个串中查找给定字符的第一个匹配之处 */
    while(sp <= end && strchr(cset, *sp)) 
		sp++;
    while(ep > start && strchr(cset, *ep))
		ep--;
	
	/* sp>ep意味着整个sds都被裁剪掉了 */
    len = (sp > ep) ? 0 : ((ep-sp)+1);
	
	/* 开头的这一部分需要被trim掉 */
    if (sh->buf != sp) 
	    memmove(sh->buf, sp, len);
	
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
    return s;
}

sds sdsrange(sds s, long start, long end) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    size_t newlen, len = sdslen(s);

	/* 这个前置判断很有必要 */
    if (len == 0) return s;

	/* 超过头了，纠正为数组头 */
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
	
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
		/* 超过尾了,纠正为数组尾 */
        if (start >= (signed)len)
			start = len-1;
        if (end >= (signed)len)
			end = len-1;
        newlen = (start > end) ? 0 : (end-start)+1;
    } else {
        start = 0;
    }
    if (start != 0) memmove(sh->buf, sh->buf+start, newlen);
    sh->buf[newlen] = 0;
    sh->free = sh->free+(sh->len-newlen);
    sh->len = newlen;
    return s;
}

/* 将所有的大写字母转为小写 */
void sdstolower(sds s) {
    int len = sdslen(s), j;

	/* 将大写字母转为小写(非大写则不处理) */
    for (j = 0; j < len; j++) 
		s[j] = tolower(s[j]);
}

/* 将所有的小写字母转为大写 */
void sdstoupper(sds s) {
    int len = sdslen(s), j;

    for (j = 0; j < len; j++) 
		s[j] = toupper(s[j]);
}

/* 比较两个s1,s2,当前仅当内容，长度相等才return.0                 */
int sdscmp(sds s1, sds s2) {
    size_t l1, l2, minlen;
    int cmp;

    l1 = sdslen(s1);
    l2 = sdslen(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1,s2,minlen);
	/* 这个判断条件有意思哈 */
    if (cmp == 0)
		return l1-l2;
    return cmp;
}

/* Split 's' with separator in 'sep'. An array
 * of sds strings is returned. *count will be set
 * by reference to the number of tokens returned.
 *
 * On out of memory, zero length string, zero length
 * separator, NULL is returned.
 *
 * Note that 'sep' is able to split a string using
 * a multi-character separator. For example
 * sdssplit("foo_-_bar","_-_"); will return two
 * elements "foo" and "bar".
 *
 * This version of the function is binary-safe but
 * requires length arguments. sdssplit() is just the
 * same function but for zero-terminated strings.
 */
sds *sdssplitlen(char *s, int len, char *sep, int seplen, int *count) {
    int elements = 0, slots = 5, start = 0, j;

    sds *tokens = zmalloc(sizeof(sds)*slots);
#ifdef SDS_ABORT_ON_OOM
    if (tokens == NULL) sdsOomAbort();
#endif

    /* 长度参数异常，返回NULL */
    if (seplen < 1 || len < 0 || tokens == NULL) 
		return NULL;	/* 这里还要释放上面申请的tokens的内存啊 */

	/* 整个算法没问题，自己画图分解下所有可能的情况遍可确认 */
    for (j = 0; j < (len-(seplen-1)); j++) {	
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {	/* 2==next+final*/
            sds *newtokens;

            slots *= 2;
            newtokens = zrealloc(tokens,sizeof(sds)*slots);
            if (newtokens == NULL) {
#ifdef SDS_ABORT_ON_OOM
                sdsOomAbort();
#else
                goto cleanup;
#endif
            }
            tokens = newtokens;
        }
		
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = sdsnewlen(s+start,j-start);
            if (tokens[elements] == NULL) {
#ifdef SDS_ABORT_ON_OOM
                sdsOomAbort();
#else
                goto cleanup;
#endif
            }
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
	
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = sdsnewlen(s+start,len-start);
    if (tokens[elements] == NULL) {
#ifdef SDS_ABORT_ON_OOM
                sdsOomAbort();
#else
                goto cleanup;
#endif
    }
    elements++;
    *count = elements;
    return tokens;

#ifndef SDS_ABORT_ON_OOM
cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) 
			sdsfree(tokens[i]);
        zfree(tokens);
        return NULL;
    }
#endif
}
