@x
mmo_file=fopen(mmo_file_name,"rb");
if (!mmo_file) {
  register char *alt_name=(char*)calloc(strlen(mmo_file_name)+5,sizeof(char));
  if (!alt_name) panic("Can't allocate file name buffer");
@.Can't allocate...@>
  sprintf(alt_name,"%s.mmo",mmo_file_name);
  mmo_file=fopen(alt_name,"rb");
  if (!mmo_file) {
    fprintf(stderr,"Can't open the object file %s or %s!\n",
@.Can't open...@>
               mmo_file_name,alt_name);
    exit(-3);
  }
  free(alt_name);
}
@y
mmo_file=exFile;
@z

@x
(ll-5)->tet=argc; /* and $\$0=|argc|$ */
@y
// (ll-5)->tet=argc; /* and $\$0=|argc|$ */
@z

@x
g[rN].l=ABSTIME; /* see comment and warning above */
@y
g[rN].l=time(NULL); /* see comment and warning above */
@z

@x
#include "abstime.h"
@y
@z

@x
int main(argc,argv)
  int argc;
  char *argv[];
@y
char* mmixsim(FILE* exFile)
@z

@x
  @<Process the command line@>;
@y
@z

@x
loc=incr(x,8*(argc+1));
for (k=0; k<argc; k++,cur_arg++) {
  ll=mem_find(x);
  ll->tet=loc.h, (ll+1)->tet=loc.l;
  ll=mem_find(loc);
  mmputchars((unsigned char *)*cur_arg,strlen(*cur_arg),loc);
  x.l+=8, loc.l+=8+(strlen(*cur_arg)&-8);
}
@y
@z