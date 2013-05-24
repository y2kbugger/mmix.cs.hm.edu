@x
@d dpanic(m,p) {@+err_buf[0]='!';@+sprintf(err_buf+1,m,p);@+
                                          report_error(err_buf);@+}
@y
@d dpanic(m,p) {return sprintf(err_buf+1,m,p);}
@z

@x
int main(argc,argv)
  int argc;@+
  char *argv[];
@y
char* mmixal(FILE* exFile, char* exFileName)
@z

@x
  @<Process the command line@>;
@y
  src_file_name = exFileName;
@z

@x
src_file=fopen(src_file_name,"r");
if (!src_file) dpanic("Can't open the source file %s",src_file_name);
@y
src_file = exFile;
@z

@x
exit(err_count);
@y
return src_file_name;
@z