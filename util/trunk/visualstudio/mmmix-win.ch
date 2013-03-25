To remove warning C4996: ... was declared deprecated
@x
#include <stdio.h>
@y
#pragma warning(disable : 4996 )
#include <stdio.h>
@z

