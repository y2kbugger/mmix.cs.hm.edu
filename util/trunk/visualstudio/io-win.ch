
@x
#ifndef SEEK_END
#define SEEK_END 2
#endif
@y
#ifndef SEEK_END
#define SEEK_END 2
#endif
#pragma warning(disable : 4996 )
@z

fread returns size_t 
@x
  n=fread(buf,1,size.l,sfile[handle].fp);
@y
  n=(unsigned int)fread(buf,1,size.l,sfile[handle].fp);
@z