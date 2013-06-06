#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testhelp.h"
#include "sds.h"

sds sdsnewlen(const void *init, size_t initlen){
	struct sdshdr *sh;
	sh = malloc(sizeof(struct sdshdr) + initlen + 1);
	if ( sh == NULL ) return NULL;
	sh->len = initlen;
	sh->free = 0;
	if ( init != NULL && initlen > 0 ){
		memcpy(sh->buf,init,initlen);
	}
	sh->buf[initlen] = '\0';
	return (char *)sh->buf;
}

sds sdsnew(const void *init){
	size_t initlen = (init == NULL ) ? 0 : strlen(init);
	return sdsnewlen(init, initlen);
}
sds sdsempty(){
	return sdsnewlen("",0);
}

void sdsfree(sds s){
	if ( s == NULL ) return;
	struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
	free(sh);
}
sds sdscat(sds s, const char *t){
	if ( s == NULL ) return NULL;
	size_t len = strlen(t);
	return sdscatlen(s,t,len);
}
sds sdsMarkeRoomFor(sds s, size_t addlen){
	if ( s == NULL ) return NULL;
	struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
	if ( sh->free >= addlen ) return s;
	size_t newlen = sh->len+addlen;
	if ( newlen < SDS_MAX_PREALLOC ){
		newlen *= 2;
	}else{
		newlen += SDS_MAX_PREALLOC;
	}
	struct sdshdr *newsh = realloc(sh,sizeof(struct sdshdr)+newlen+1);
	if ( newsh == NULL ) return NULL;
	newsh->free = newlen - sh->len;
	return (char *)newsh->buf;
}

sds sdscatlen(sds s, const void *t, size_t len){
	if ( s == NULL || t == NULL ) return NULL;
	s =sdsMarkeRoomFor(s,len);
	if ( s == NULL ) return NULL;
	size_t curlen = sdslen(s);
	memcpy(s+curlen,t,len);
	struct sdshdr *sh=(void *)(s-(sizeof(struct sdshdr)));
	sh->free = sh->free - len;
	sh->len = curlen+len;
	s[curlen+len]='\0';
	return s;
}

sds sdscpylen(sds s, const char *t, size_t len){
	struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
	if ( sh->free + sh->len > len){
		s = sdsMarkeRoomFor(s, len-sh->len);
		if ( s == NULL ) return NULL;
		sh = (void *)(s-(sizeof(struct sdshdr)));
	}
	memcpy(sh->buf, t, len);
	sh->free = sh->len + sh->free - len;
	sh->len = len;
	sh->buf[len] = '\0';
	return sh->buf;
}
sds sdscpy(sds s, const char *t){
	if ( s == NULL ) return NULL;
	size_t len = strlen(t);
	return sdscpylen(s,t,len);
}
sds sdscatvprintf(sds s, const char *fmt, va_list ap){
	if ( s == NULL ) return NULL;
	va_list cpy;
	char *buf, *t;
	size_t buflen = 16;

	while(1){
		buf = malloc(buflen);
		if ( buf == NULL ) return NULL;
		buf[buflen-2] = '\0';
		va_copy(cpy,ap);
		vsnprintf(buf,buflen,fmt,cpy);
		if ( buf[buflen-2] != '\0' ){
			free(buf);
			buflen *=2;
			continue;
		}
		break;
	}
	t = sdscat(s,buf);
	free(buf);
	return t;
}
sds sdscatprintf(sds s,const char *fmt, ...){
	if ( s == NULL ) return NULL;
	va_list ap;
	char *t;
	va_start(ap,fmt);
	t = sdscatvprintf(s,fmt,ap);
	va_end(ap);
	return t;
}
sds sdstrim(sds s, const char *cset){
	if ( s == NULL ) return NULL;
	struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
	char *moveAbleStart, *moveAbleEnd, *start, *end;
	size_t len;
	moveAbleStart = start = sh->buf;
	moveAbleEnd = end = sh->buf + sh->len-1;
	while(moveAbleStart <= moveAbleEnd && strchr(cset,*moveAbleStart)) moveAbleStart ++;
	while(moveAbleStart <= moveAbleEnd && strchr(cset,*moveAbleEnd)) moveAbleEnd--;
	len = moveAbleEnd - moveAbleStart + 1;
	if ( moveAbleStart != sh->buf ) memmove(sh->buf, moveAbleStart, len);
	sh->buf[len]='\0';
	sh->free = sh->free + (sh->len-len);
	sh->len = len;
	return s;
}

//将返回 s 的从 start 开始到 end结束的 （ 改变s ）
sds sdsrange(sds s, int start, int end){
	struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
	size_t len = sdslen(s);
	if ( len == 0 ) return s;
	if ( start < 0 ){
		start = start + len;
		if ( start < 0 )	start = 0;
	}
	if ( end < 0 ){
		end = end + len;
		if ( end < 0 )	end = 0;
	}
	size_t newlen = end> start ? (end-start+1):0;
	if ( newlen != 0 ){
		if ( start >= (signed)len ){
			newlen = 0;
		}else if ( end >= (signed)len){
			end = len-1;
			newlen = ( start > end ) ? 0 : (end-start+1);
		}
	}else{
		start = 0;
	}
	if ( start > 0 && newlen > 0 ) memmove(sh->buf,sh->buf+start, newlen);
	sh->buf[newlen] = '\0';
	sh->free = sh->free + ( sh->len - newlen);
	sh->len = newlen;
	return s;
}
sds sdsdup(const sds s){
	return sdsnewlen(s,sdslen(s));
}
int sdscmp(const sds s1, const sds s2){
	size_t len1,len2, minLen;
	len1 = sdslen(s1);
	len2 = sdslen(s2);
	minLen = (len1 > len2 ) ? len2 : len1;
	int cmp = memcmp(s1,s2,minLen);
	if ( cmp == 0 ) return len1-len2;
	return cmp;
}

void sdstolower(sds s){
	int len = sdslen(s);
	int i;
	for(i=0; i<len; ++i ) s[i] = tolower(s[i]);
}
void sdstoupper(sds s){
	int len = sdslen(s);
	int i;
	for(i=0; i<len; ++i ) s[i] = toupper(s[i]);
}
void sdsupdatelen(sds s){
	printf("this func is inviale");
}
int main(void){

	test_cond("testHelp test",1)
	
	struct sdshdr *sh;
	sds x = sdsnew("foo");
	sds y;
	test_cond("create a string", memcmp(x,"foo\0", 4)==0)

	sdsfree(x);
	x = sdsnewlen("fooafdjksdlfjsadlfj",2);
	test_cond("create a string with len", memcmp(x,"fo\0",3) == 0)

	x = sdscat(x,"bar");
	test_cond("cat a str",sdslen(x) == 5 && memcmp(x,"fobar\0",6) == 0)

	x = sdscpy(x,"a");
	test_cond("copy ", sdslen(x) == 1 && memcmp(x,"a\0",2)==0)

	x = sdscpy(x,"xyzxxxxxxxxxyyyyyyyyyyxzzzzzzzzzz");
	test_cond("sdscpy with a longer string",
			sdslen(x) == 33 &&
			memcmp(x,"xyzxxxxxxxxxyyyyyyyyyyxzzzzzzzzzz\0",33)==0)

	sdsfree(x);
	x = sdscatprintf(sdsempty(),"%d",123);
	test_cond("sdscatprintf()",
			sdslen(x)==3 && memcmp(x,"123\0",4)==0);

	sdsfree(x);
	x = sdstrim(sdsnew("xxciaoyyy"),"xy");
	test_cond("sdstrim",sdslen(x) == 4 && memcmp(x,"ciao\0",5) == 0 );

	y = sdsrange(sdsdup(x),1,1);
	test_cond("sdsrange(...,1,1)",
		sdslen(y) == 1 && memcmp(y,"i\0",2) == 0)

	sdsfree(y);
	y = sdsrange(sdsdup(x),1,-1);
	test_cond("sdsrange(...,1,-1)",
		sdslen(y) == 3 && memcmp(y,"iao\0",4) == 0)

	sdsfree(y);
	y = sdsrange(sdsdup(x),-2,-1);
	test_cond("sdsrange(...,-2,-1)",
		sdslen(y) == 2 && memcmp(y,"ao\0",3) == 0)

	sdsfree(y);
	y = sdsrange(sdsdup(x),2,1);
	test_cond("sdsrange(...,2,1)",
		sdslen(y) == 0 && memcmp(y,"\0",1) == 0)

	sdsfree(y);
	y = sdsrange(sdsdup(x),1,100);
	test_cond("sdsrange(...,1,100)",
		sdslen(y) == 3 && memcmp(y,"iao\0",4) == 0)

	sdsfree(y);
	y = sdsrange(sdsdup(x),100,100);
	test_cond("sdsrange(...,100,100)",
		sdslen(y) == 0 && memcmp(y,"\0",1) == 0)

	sdsfree(y);
	sdsfree(x);
	x = sdsnew("foo");
	y = sdsnew("foa");
	test_cond("sdscmp(foo,foa)", sdscmp(x,y) > 0)

	sdsfree(y);
	sdsfree(x);
	x = sdsnew("bar");
	y = sdsnew("bar");
	test_cond("sdscmp(bar,bar)", sdscmp(x,y) == 0)

	sdsfree(y);
	sdsfree(x);
	x = sdsnew("aar");
	y = sdsnew("bar");
	test_cond("sdscmp(bar,bar)", sdscmp(x,y) < 0)

	sdsfree(y);
	sdsfree(x);
	x = sdsnew("AAABc");
	sdstolower(x);
	test_cond("sdstolower",memcmp(x,"aaabc\0",5)==0)

	sdsfree(x);
	x = sdsnew("abcdABCD");
	sdstoupper(x);
	test_cond(" sdstoupper",memcmp(x,"ABCDABCD\0",9)==0);

	sdsfree(x);
	return 0;
}
