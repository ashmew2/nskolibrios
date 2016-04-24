#include<ctype.h>
#include<stdio.h>
int main()
 {
        char str[20];
	int i = 0;
	for(i=0;i <=255; i++) {
		if(isspace(i))
		  sprintf(str, "%c: %d : True\n\0", i, (int)i);
		else
		  sprintf(str, "%c: %d : False\n\0", i, (int)i);
	debug_board_write_str(str);
	}

	return 0;
 }
