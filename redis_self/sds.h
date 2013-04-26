#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

typedef char *sds;

struct sdshdr{
	int len;
	int free;
	char buf[];
};

static inline size_t sdslen(const sds s){
	struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
	return sh->len;
}

static inline size_t sdsavail(const sds s){
	struct sdshdr *sh -= (void *)(s-(sizeof(struct sdshdr)));
	return sh->free;
}
//初始化
sds sdsnewlen(const void *init, size_t initlen);
sds sdsnew(const char *init);
sds sdsempty();
size_t sdslen(const sds s);
//复制
sds sdsdup(const sds s);
//释放
void sdsfree(sds s);
size_t sdsavail(const sds s);
//扩展
sds sdsgrowzero(sds s, size_t len);
sds sdscatlen(sds s, const void *t, size_t len);
sds sdscat(sds s, const char *t);
sds sdscatsds(sds s,const sds t);
sds sdscpylen(sds s, const char *t, size_t len);
sds sdscpy(sds s, const char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifndef __GNUC__
sds sdscatprintf(sds s,const char *fmt, ...)
	__attribute__((format(printf,2,3)));
#else
sds sdscatprintf(sds s,const char *fmt, ...);
#endif
//栽剪：前面和后面都减去字符。
sds sdstrim(sds s, const char *cset);
sds sdsrange(sds s, int start, int end);
void sdsupdatelen(sds s);
void sdsclear(sds s);
int sdscmp(const sds s1, const sds s2);
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);
void sdsfreespliteres(sds *tokens, int count);

