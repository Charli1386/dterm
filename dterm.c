// Project Dods Term - A terminal-based text editor written in C

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define CTRL_KEY(k) ((k) & 0x1f) /* Define macro of pressing CTRL */


typedef struct editorConfig{
	int screenrows, screencols;
	struct termios orig_termios;
}Editor; 	/* Declare a copy of editor config struct to preserve it for future use */

Editor E; /* Declare instance of Editor as E */

void die(const char* s){
	/*======================================
	Function to print error messages when
	program encounters errors
	======================================*/
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	/* Clear screen on exit */

	perror(s); /* stdio.h */
	exit(1); /* stdlib.h */
}

void disableRawMode(){
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1){
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
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1){
		die("tcgetattr");
	}
	
	atexit(disableRawMode); /* stdlib.h -> calls attributes when program ends */

	struct termios raw = E.orig_termios; /* create termios struct raw and assign orig */

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

char editorKeyRead(){
	/*=======================================
	Function to refactor keyboard input
	=======================================*/
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) == -1){
		if(nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

void editorKeyress(){
	/*=======================================
	Main terminal editor window
	=======================================*/
	char c = editorKeyRead();

	switch(c){
		case CTRL_KEY('q'): {
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
		}
	}
}

int getCursorPosition(int *rows, int *cols){
	/*=====================================
	Function to get cursor position
	======================================*/
	char buf[32];
	unsigned int i = 0;

	if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4){
		return -1;
	}

	while(i < sizeof(buf) - 1){
		if(read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if(buf[i] == 'R') break;
		i++;
	}
	buf[i] = '\0';

	if(buf[0] != '\x1b' || buf[1] != '[') return -1;
	if(sscanf((&buf[2]), "%d;%d", rows, cols) != 2) return -1;

	return 0;


}

int getWindowSize(int *rows, int *cols){
	/*================================================
	Get window size - ioctl() and fallback method
	================================================*/
	struct winsize ws;

	if(1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
		if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12){
			return -1;
		}
		return getCursorPosition(rows, cols);
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
	/*================================================
	TICOCGWINSZ -> Terminal IOCtl (Input/Output Control
	) Get WINdow SiZe

	on success ioctl() will place columns wide and
	number of rows as high the terminal is given into
	given winsize struct

	Fallback method 
	-> position cursor at the bottom-right of the screen
	then use escape sequences that query the position 
	of the cursor that tells how many rows anc cols 
	are on the screen
	
	C -> command to move cursor to the right
	B -> command to move cursor down

	Use largest values to ensure that the cursor
	would be positioned at the bottom right [999C and [999B

	C and B are specifically documented to stop cursor
	from going past the  edge of the screen

	Append 1 || at the beginning of 1st condition
	to test if confition temporarilly

	================================================*/
}

void initEditor(){
	if(getWindowSize(&E.screenrows, &E.screencols) == -1){
		die("getWindowSize");
	}
}

void editorDrawRows(){
	/*========================
	Function to draw tildes
	on the side of screen like
	vim
	========================*/
	for(int r=0; r<E.screenrows; r++){ /* draw up to 25 rows for now */
		write(STDOUT_FILENO, "~", 1);

		if(r < E.screenrows - 1){
			write(STDOUT_FILENO, "\r\n", 2);
		}
	}
}

void editorRefreshScreen(){
	/*=======================================
	Function to clear screen after each
	keypress
	=======================================*/
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	/*======================================
	write and STDOUT_FILENO -> unistd.h

	we write 4 bytes out to the terminal
	\x1b - 1st byte (escape char ; 27 dec)
	[2J -> other 3 bytes

	\x1b[2J -> escape sequence, always starts
	with \x1b followed by [

	J -> command byte to clear screen (erase on
	display) - takes in arguments before the 
	command
	1J -> clear screen up to where cursor is
	0J -> clear screen from cursor up to end
	of screen

	2J -> clear everything

	###

	\x1b[H -> 3 byte escape sequence, uses 
	H (cursor position) command which takes
	two arguments (row num, col num) which is.
	the cursor's position

	default values for args, 1
	======================================*/

	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3); /* Bring cursor to col 1 row 1 of terminal */
}


int main(){
	enableRawMode(); /* While program is running, no input will be echoed */
	initEditor(); /* initialize the editor */

	while(1){
		editorRefreshScreen();
		editorKeyress();
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

