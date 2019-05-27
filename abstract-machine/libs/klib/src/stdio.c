#include "klib.h"
#include <stdarg.h> 
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
char buf[100000];
int width;
int printf(const char *fmt, ...) {
  va_list ext;  
  va_start(ext,fmt);
  int cnt=vsprintf(buf,fmt,ext),i=0;
  while(buf[i])
    _putc(buf[i]),i++;
  return cnt;
}
int vsprintf(char *out, const char *fmt, va_list ext) {
  int cnt=0;
  while(*fmt!='\0')	{
   	if(*fmt!='%')	{
		*out=*fmt;
		out++;
		fmt++;
	}
   	else {
		cnt++;
 		fmt++;
		if(*fmt=='%'){
			*out=*fmt;
			fmt++;
			continue;
		}
		if(*fmt=='.'||*fmt=='0')	{
			fmt++;
			width=0;
 			while( '0'<=(*fmt) && (*fmt)<='9')	{
				width=width*10+*fmt-'0';
				fmt++;
			}
		}
		else width=1;
	 	if(*fmt=='x'||*fmt=='p')	{
			char *tmp=out;
			uint64_t Gol;
			if(*fmt=='x')
				Gol=va_arg(ext,uintptr_t);
			else 
				Gol=(uint32_t)va_arg(ext,uint32_t);
			uint64_t j=0;
			while(Gol!=0) 	{
			    if((Gol&0xf)>=10)
				*out=(Gol&0xf)-10+'a';
			    else
				*out=(Gol&0xf)+'0';
			    j++;
			    out++;
			    Gol>>=4;
			} 
			while(width>j){
				*out='0';
				out++;
				j++;
			} 
			for(int i=0;i<j/2;i++){
				char temp=*(tmp+i);
				*(tmp+i)=*(out-1-i);
				*(out-1-i)=temp;
			}	
		}  
		else if(*fmt=='d')	{ 
			int Gol=va_arg(ext,int);
			char *tmp=out;
			int j=0;
			if(Gol==0) {
			   *out='0';
			   out++;
			}
			while(Gol>0)	{
				*out=Gol%10+'0';
				out++;
				Gol/=10;
				j++;	
			}
			for(int i=0;i<j/2;i++){
				char temp=*(tmp+i);
				*(tmp+i)=*(out-1-i);
				*(out-1-i)=temp;
			} 
 		}
		else if (*fmt=='u')	{
			uint32_t Gol=va_arg(ext,uint32_t);
			char *tmp=out;
			int j=0;
			while(Gol>0)	{
				*out=Gol%10+'0';
				out++;
				Gol/=10;
				j++;	
			} 
			for(int i=0;i<j/2;i++){
				char temp=*(tmp+i);
				*(tmp+i)=*(out-1-i);
				*(out-1-i)=temp;
			} 
		}	 
 		else if(*fmt=='s')	{
			char* Gol=va_arg(ext,char*);
			while(*Gol!=0)	{
				*out=*Gol;
				Gol++;
				out++;
  			}
 		}
		else if(*fmt=='c')	{
			*out=*(va_arg(ext,char *));
			out++;
		}
		else{ printf("No such kind's definition as");_putc(*fmt);_putc('\n');}//assert(0);}
		fmt++;
	}
   }
   *out=0;
   out++;  
   return cnt;
}
int sprintf(char *out, const char *fmt, ...) {
  va_list ext;  
  va_start(ext,fmt);
  int cnt=vsprintf(out,fmt,ext);
  return cnt;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
