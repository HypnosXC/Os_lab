#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int i=0;
  while(1){
	if(s[i]=='\0')
		break;
	i++;
  }
  return i;
}

char *strcpy(char* dst,const char* src) {
  int l=strlen(src);
  for(int i=0;i<l;i++)
	  dst[i]=src[i];
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  for(int i=0;i<n;i++)
	  dst[i]=src[i];
  return dst;
}

char* strcat(char* dst, const char* src) {
  int s=strlen(dst),l=strlen(src);
  for(int i=s;i<s+l;i++)
	  dst[i]=src[i-s];
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  int l1=strlen(s1),l2=strlen(s2);
  int l=(l1<l2 ? l1 :l2);
  for(int i=0;i<=l;i++)	{
		if(s1[i]-s2[i]!=0)
			return (int)s1[i]-s2[i];
  }
  return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
 for(int i=0;i<n ;i++)	{
  if(s1[i]-s2[i]!=0)
   return (int)(s1[i]-s2[i]);
 }
 return 0;
}

void* memset(void* v,int c,size_t n) {
	char *vv=(char *)v;
  for(int i=0;i<n;i++)
	  *(vv+i)=(char)c;
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
 assert(out!=NULL&&in!=NULL);
 unsigned char *oot=(unsigned char  *)out;
 const unsigned char *iin=(const unsigned char *)in;
 for(int i=0;i<n;i++)
	  *(oot+i)=*(iin+i);
 return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
 assert(s1!=NULL&&s2!=NULL);
 const unsigned char *oot=(unsigned char  *)s1;
 const unsigned char *iin=(const unsigned char *)s2;
 for(int i=0;i<n;i++)
	 if(*(oot+i)!=*(iin+i))
		return (int)*(oot+i)-*(iin+i);
  return 0;
}

#endif
