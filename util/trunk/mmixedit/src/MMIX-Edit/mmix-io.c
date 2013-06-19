#define sign_bit ((unsigned) 0x80000000)  \

/*1:*/
#line 16 "mmix-io.w"

/*2:*/
#line 26 "mmix-io.w"

#include <stdio.h> 
#include <stdlib.h> 
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
#ifndef FILENAME_MAX
#define FILENAME_MAX 256
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

/*:2*/
#line 17 "mmix-io.w"

/*3:*/
#line 47 "mmix-io.w"

typedef unsigned int tetra;
typedef struct{tetra h,l;}octa;

/*:3*//*5:*/
#line 68 "mmix-io.w"

typedef struct{
FILE*fp;
int mode;
}sim_file_info;

/*:5*/
#line 18 "mmix-io.w"

/*4:*/
#line 56 "mmix-io.w"

extern char stdin_chr ARGS((void));
extern int mmgetchars ARGS((char*buf,int size,octa addr,int stop));
extern void mmputchars ARGS((unsigned char*buf,int size,octa addr));
extern octa oplus ARGS((octa,octa));
extern octa ominus ARGS((octa,octa));
extern octa incr ARGS((octa,int));
extern octa zero_octa;
extern octa neg_one;

/*:4*/
#line 19 "mmix-io.w"

/*6:*/
#line 74 "mmix-io.w"

sim_file_info sfile[256];

/*:6*//*9:*/
#line 110 "mmix-io.w"

char*mode_string[]= {"r","w","rb","wb","w+b"};
int mode_code[]= {0x1,0x2,0x5,0x6,0xf};

/*:9*//*24:*/
#line 391 "mmix-io.w"

char*trip_warning[]= {
"TRIP",
"integer divide check",
"integer overflow",
"float-to-fix overflow",
"invalid floating point operation",
"floating point overflow",
"floating point underflow",
"floating point division by zero",
"floating point inexact"};

/*:24*/
#line 20 "mmix-io.w"

/*7:*/
#line 79 "mmix-io.w"

void mmix_io_init ARGS((void));
void mmix_io_init()
{
sfile[0].fp= stdin,sfile[0].mode= 1;
sfile[1].fp= stdout,sfile[1].mode= 2;
sfile[2].fp= stderr,sfile[2].mode= 2;
}

/*:7*//*8:*/
#line 92 "mmix-io.w"

octa mmix_fopen ARGS((unsigned int,octa,octa));
octa mmix_fopen(handle,name,mode)
unsigned int handle;
octa name,mode;
{
char name_buf[FILENAME_MAX];
if(mode.h||mode.l> 4)goto failure;
if(mmgetchars(name_buf,FILENAME_MAX,name,0)==FILENAME_MAX)goto failure;
if(sfile[handle].mode!=0&&handle> 2)fclose(sfile[handle].fp);
sfile[handle].fp= fopen(name_buf,mode_string[mode.l]);
if(!sfile[handle].fp)goto failure;
sfile[handle].mode= mode_code[mode.l];
return zero_octa;
failure:sfile[handle].mode= 0;
return neg_one;
}

/*:8*//*10:*/
#line 117 "mmix-io.w"

void mmix_fake_stdin ARGS((FILE*));
void mmix_fake_stdin(f)
FILE*f;
{
sfile[0].fp= f;
}

/*:10*//*11:*/
#line 125 "mmix-io.w"

octa mmix_fclose ARGS((unsigned int));
octa mmix_fclose(handle)
unsigned int handle;
{
if(sfile[handle].mode==0)return neg_one;
if(handle> 2&&fclose(sfile[handle].fp)!=0)return neg_one;
sfile[handle].mode= 0;
return zero_octa;
}

/*:11*//*12:*/
#line 136 "mmix-io.w"

octa mmix_fread ARGS((unsigned int,octa,octa));
octa mmix_fread(handle,buffer,size)
unsigned int handle;
octa buffer,size;
{
register unsigned char*buf;
register unsigned int n;
octa o;
o= neg_one;
if(!(sfile[handle].mode&0x1))goto done;
if(sfile[handle].mode&0x8)sfile[handle].mode&= ~0x2;
if(size.h)goto done;
buf= (unsigned char*)calloc(size.l,sizeof(char));
if(!buf)goto done;
/*13:*/
#line 158 "mmix-io.w"

if(sfile[handle].fp==stdin){
register unsigned char*p;
for(p= buf,n= size.l;p<buf+n;p++)*p= stdin_chr();
}else{
clearerr(sfile[handle].fp);
n= fread(buf,1,size.l,sfile[handle].fp);
if(ferror(sfile[handle].fp)){
free(buf);
goto done;
}
}

/*:13*/
#line 151 "mmix-io.w"
;
mmputchars(buf,n,buffer);
free(buf);
o.h= 0,o.l= n;
done:return ominus(o,size);
}

/*:12*//*14:*/
#line 171 "mmix-io.w"

octa mmix_fgets ARGS((unsigned int,octa,octa));
octa mmix_fgets(handle,buffer,size)
unsigned int handle;
octa buffer,size;
{
char buf[256];
register int n,s;
register char*p;
octa o;
int eof= 0;
if(!(sfile[handle].mode&0x1))return neg_one;
if(!size.l&&!size.h)return neg_one;
if(sfile[handle].mode&0x8)sfile[handle].mode&= ~0x2;
size= incr(size,-1);
o= zero_octa;
while(1){
/*15:*/
#line 197 "mmix-io.w"

s= 255;
if(size.l<(unsigned int)s&&!size.h)s= (int)size.l;
if(sfile[handle].fp==stdin)
for(p= buf,n= 0;n<s;){
*p= stdin_chr();
n++;
if(*p++=='\n')break;
}
else{
if(!fgets(buf,s+1,sfile[handle].fp))return neg_one;
eof= feof(sfile[handle].fp);
for(p= buf,n= 0;n<s;){
if(!*p&&eof)break;
n++;
if(*p++=='\n')break;
}
}
*p= '\0';

/*:15*/
#line 188 "mmix-io.w"
;
mmputchars((unsigned char*)buf,n+1,buffer);
o= incr(o,n);
size= incr(size,-n);
if((n&&buf[n-1]=='\n')||(!size.l&&!size.h)||eof)return o;
buffer= incr(buffer,n);
}
}

/*:14*//*16:*/
#line 226 "mmix-io.w"

octa mmix_fgetws ARGS((unsigned int,octa,octa));
octa mmix_fgetws(handle,buffer,size)
unsigned int handle;
octa buffer,size;
{
char buf[256];
register tetra n,s;
register char*p;
octa o;
int eof= 0;
if(!(sfile[handle].mode&0x1))return neg_one;
if(!size.l&&!size.h)return neg_one;
if(sfile[handle].mode&0x8)sfile[handle].mode&= ~0x2;
buffer.l&= -2;
size= incr(size,-1);
o= zero_octa;
while(1){
/*17:*/
#line 254 "mmix-io.w"

s= 127;
if(size.l<s&&!size.h)s= size.l;
if(sfile[handle].fp==stdin)
for(p= buf,n= 0;n<s;){
*p++= stdin_chr();*p++= stdin_chr();
n++;
if(*(p-1)=='\n'&&*(p-2)==0)break;
}
else for(p= buf,n= 0;n<s;){
if(fread(p,1,2,sfile[handle].fp)!=2){
eof= feof(sfile[handle].fp);
if(!eof)return neg_one;
break;
}
n++,p+= 2;
if(*(p-1)=='\n'&&*(p-2)==0)break;
}
*p= *(p+1)= '\0';

/*:17*/
#line 244 "mmix-io.w"
;
mmputchars((unsigned char*)buf,2*n+2,buffer);
o= incr(o,n);
size= incr(size,-(int)n);
if((n&&buf[2*n-1]=='\n'&&buf[2*n-2]==0)||(!size.l&&!size.h)||eof)
return o;
buffer= incr(buffer,2*n);
}
}

/*:16*//*18:*/
#line 274 "mmix-io.w"

octa mmix_fwrite ARGS((unsigned int,octa,octa));
octa mmix_fwrite(handle,buffer,size)
unsigned int handle;
octa buffer,size;
{
char buf[256];
register unsigned int n;
if(!(sfile[handle].mode&0x2))return ominus(zero_octa,size);
if(sfile[handle].mode&0x8)sfile[handle].mode&= ~0x1;
while(1){
if(size.h||size.l>=256)n= mmgetchars(buf,256,buffer,-1);
else n= mmgetchars(buf,size.l,buffer,-1);
size= incr(size,-(int)n);
if(fwrite(buf,1,n,sfile[handle].fp)!=n)return ominus(zero_octa,size);
fflush(sfile[handle].fp);
if(!size.l&&!size.h)return zero_octa;
buffer= incr(buffer,n);
}
}

/*:18*//*19:*/
#line 295 "mmix-io.w"

octa mmix_fputs ARGS((unsigned int,octa));
octa mmix_fputs(handle,string)
unsigned int handle;
octa string;
{
char buf[256];
register unsigned int n;
octa o;
o= zero_octa;
if(!(sfile[handle].mode&0x2))return neg_one;
if(sfile[handle].mode&0x8)sfile[handle].mode&= ~0x1;
while(1){
n= mmgetchars(buf,256,string,0);
if(fwrite(buf,1,n,sfile[handle].fp)!=n)return neg_one;
o= incr(o,n);
if(n<256){
fflush(sfile[handle].fp);
return o;
}
string= incr(string,n);
}
}

/*:19*//*20:*/
#line 319 "mmix-io.w"

octa mmix_fputws ARGS((unsigned int,octa));
octa mmix_fputws(handle,string)
unsigned int handle;
octa string;
{
char buf[256];
register unsigned int n;
octa o;
o= zero_octa;
if(!(sfile[handle].mode&0x2))return neg_one;
if(sfile[handle].mode&0x8)sfile[handle].mode&= ~0x1;
while(1){
n= mmgetchars(buf,256,string,1);
if(fwrite(buf,1,n,sfile[handle].fp)!=n)return neg_one;
o= incr(o,n>>1);
if(n<256){
fflush(sfile[handle].fp);
return o;
}
string= incr(string,n);
}
}

/*:20*//*21:*/
#line 345 "mmix-io.w"

octa mmix_fseek ARGS((unsigned int,octa));
octa mmix_fseek(handle,offset)
unsigned int handle;
octa offset;
{
if(!(sfile[handle].mode&0x4))return neg_one;
if(sfile[handle].mode&0x8)sfile[handle].mode= 0xf;
if(offset.h&sign_bit){
if(offset.h!=0xffffffff||!(offset.l&sign_bit))return neg_one;
if(fseek(sfile[handle].fp,(int)offset.l+1,SEEK_END)!=0)return neg_one;
}else{
if(offset.h||(offset.l&sign_bit))return neg_one;
if(fseek(sfile[handle].fp,(int)offset.l,SEEK_SET)!=0)return neg_one;
}
return zero_octa;
}

/*:21*//*22:*/
#line 363 "mmix-io.w"

octa mmix_ftell ARGS((unsigned int));
octa mmix_ftell(handle)
unsigned int handle;
{
register long x;
octa o;
if(!(sfile[handle].mode&0x4))return neg_one;
x= ftell(sfile[handle].fp);
if(x<0)return neg_one;
o.h= 0,o.l= x;
return o;
}

/*:22*//*23:*/
#line 380 "mmix-io.w"

void print_trip_warning ARGS((int,octa));
void print_trip_warning(n,loc)
int n;
octa loc;
{
if(sfile[2].mode&0x2)
fprintf(sfile[2].fp,"Warning: %s at location %08x%08x\n",
trip_warning[n],loc.h,loc.l);
}

/*:23*/
#line 21 "mmix-io.w"


/*:1*/
