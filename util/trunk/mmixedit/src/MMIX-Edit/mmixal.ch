@x
@d dpanic(m,p) {@+err_buf[0]='!';@+sprintf(err_buf+1,m,p);@+
                                          report_error(err_buf);@+}
@y
@d dpanic(m,p) {return sprintf(err_buf+1,m,p);}
@z

@x
  if (!filename[cur_file]) filename[cur_file]="(nofile)";
  if (message[0]=='*')
    fprintf(stderr,"\"%s\", line %d warning: %s\n",
                 filename[cur_file],line_no,message+1);
  else if (message[0]=='!')
    fprintf(stderr,"\"%s\", line %d fatal error: %s\n",
                 filename[cur_file],line_no,message+1);
  else {
    fprintf(stderr,"\"%s\", line %d: %s!\n",
                 filename[cur_file],line_no,message);
@y
  char *error_message[256];
  if (!filename[cur_file]) filename[cur_file]="(nofile)";
  if (message[0]=='*') {
	sprintf(error_message,"warning: %s",
                 message+1);
	addListString(error_message, line_no, true);
  } else if (message[0]=='!') {
    sprintf(error_message,"fatal error: %s",
                 message+1);
	addListString(error_message, line_no, true);
  } else {
    sprintf(error_message,"%s!",
                 message);
	addListString(error_message, line_no, true);
@z

@x
Char *special_name[32]={"rB","rD","rE","rH","rJ","rM","rR","rBB",
 "rC","rN","rO","rS","rI","rT","rTT","rK","rQ","rU","rV","rG","rL",
 "rA","rF","rP","rW","rX","rY","rZ","rWW","rXX","rYY","rZZ"};
@y
extern Char *special_name[32];
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