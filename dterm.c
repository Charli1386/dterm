// Project Dods Term - A terminal-based text editor written in C

#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct termios orig_termios; /* Declare a copy of termios struct to preserve it for future use */

void disableRawMode(){
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

	/*=====================================
	TCSAFLUSH -> termios.h
	It discards any unread output before 
	performing next process
	=====================================*/
}

void enableRawMode(){
	/*=======================================
	Function to turn off input echoing
	=======================================*/

	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(disableRawMode); /* stdlib.h -> calls attributes when program ends */

	struct termios raw = orig_termios; /* create termios struct raw and assign orig */

	raw.c_lflag &= ~(ECHO | ICANON | ISIG); 

	/*=======================================
	Bitwise NOT the ECHO and ICANON value of the terminal's flags and bitwise AND to assign it to raw's local flags

	ECHO - responsible for outputting the 
	input as user types

	ICANON - flag that allows to disable
	canon mode, meaning that inputs
	will be read byte-by-byte instead of
	line-by-line (e.g. No need to press
	enter to register input)
	=======================================*/

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main(){
	enableRawMode(); /* While program is running, no input will be echoed */


	char c;
	while(read(STDIN_FILENO, &c, 1 && c != 'q') == 1){
		if(iscntrl(c)){
			printf("%d\n", c);
		} else {
			printf("%d ('%c')\n", c, c);
		}
	}
	
	// While able to read or input not q

	/*===================================
	read() and STDIN_FILENO -> unistd
	read 1 byte from input into var c
	until there are no more bytes to read

	iscntrl() -> ctype
	tests whether input is a control char
	(ASCII codes 0-31, 127) -> cntrl chrs
	===================================*/

	return 0;
}