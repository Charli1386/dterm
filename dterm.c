// Project Dods Term - A terminal-based text editor written in C

/* ==================== INCLUDES ============================================*/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

/*==============================END SECTION===================================*/





/*==================================DATA======================================*/

struct termios orig_termios; /* Declare a copy of termios struct to preserve it for future use */

/*==============================END SECTION===================================*/




/*============================TERMINAL========================================*/

void die(const char* s){
	/*======================================
	Function to print error messages when
	program encounters errors
	======================================*/
	perror(s); /* stdio.h */
	exit(1); /* stdlib.h */
}

void disableRawMode(){
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1){
		die("tcsetattr");
	}

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
	if(tcgetattr(STDIN_FILENO, &orig_termios) == -1){
		die("tcgetattr");
	}
	
	atexit(disableRawMode); /* stdlib.h -> calls attributes when program ends */

	struct termios raw = orig_termios; /* create termios struct raw and assign orig */

	/* Terminal Flags */
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); 

	/* Terminal Control */
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	/*=======================================
	VMIN, VTIME -> termios.h

	VMIN - sets the min number of bytes needed
	before read can return 

	VTIME - sets max amt of time to wait
	before read returns
	========================================*/



	/*=======================================
	Bitwise NOT the ECHO and ICANON value of the terminal's flags and bitwise AND to assign it to raw's local flags

	ECHO - responsible for outputting the 
	input as user types

	ICANON - flag that allows to disable
	canon mode, meaning that inputs
	will be read byte-by-byte instead of
	line-by-line (e.g. No need to press
	enter to register input)

	IXON - Input flag + XON (Ctrl+S, Ctrl + Q)
	which the frmr stops transmission of data
	and the ltr resumes transmission

	ICRNL - flag to make ctrl+m and enter carriage return

	IEXTEN - flag to disable ctrl+v and ctrl+o

	OPOST - flag to turn off output processing
	such as adding \n\r to new line

	## Legacy flags 

	BRKINT - turn off break condition much like ctrl+c

	INPCK - enables parity checkng

	ISTRIP - causes 8th bit of each input byte
	to be stripped (e.g. set to 0). Most probably
	off by default

	CS8 - not a flag but a bit mask which sets
	char size to 8 bits per byte
	=======================================*/

	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
		die("tcgetattr");
	}
}

/*=====================END SECTION===================================*/





/*============================INIT===================================*/
int main(){
	enableRawMode(); /* While program is running, no input will be echoed */
	
	while(1){
		char c = '\0';
		if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN){
			die("read");
		}
		if(iscntrl(c)){
			printf("%d\r\n", c);
		} else {
			printf("%d ('%d')", c, c);
		}
		if(c == 'q') break;
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

/*=====================END SECTION===================================*/