#include<ctype.h>
#include<stdio.h>
int main()
 {
        char str[20];
	int i = 0;
	for(i=0;i <=20; i++) {
		if(isspace(i))
			sprintf(str, "%d : True\n\0", i);
		else
			sprintf(str, "%d : False\n\0", i);
	debug_board_write_str(str);
	}

	for(i=65;i <=75; i++) {
		if(isspace(i))
			sprintf(str, "%d : True\n\0", i);
		else
			sprintf(str, "%d : False\n\0", i);
	debug_board_write_str(str);
	}

/*	for(i=97;i <=122; i++) {
		if(isspace(i))
			sprintf(str, "%d : True\n\0", i);
		else
			sprintf(str, "%d : False\n\0", i);
	debug_board_write_str(str);
	}
*/

	return 0;
 }
