#include "uthash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
//#include <windows.h>
#include <sys/utsname.h> //import uname for OS

//authors: Alexander Barnhart, Dave Kelly
//uthash is an open source C hash library. http://troydhanson.github.io/uthash

//uname code: https://stackoverflow.com/questions/8706958/determine-os-during-runtime
//            https://en.wikipedia.org/wiki/Uname
//            https://stackoverflow.com/questions/3596310/c-how-to-use-the-function-uname
//            http://pubs.opengroup.org/onlinepubs/007908775/xsh/sysutsname.h.html

int linux = 0;   //flag for linux OS on machine
int windows = 0; //flag for windows OS on machine

struct program_list {
	char process[76];
	int memory_usage;
	UT_hash_handle hh;
};

struct program_list *programs = NULL;

struct utsname unameData;

void determineOS() {
	if(strncmp(unameData.sysname, "Linux", sizeof("Linux"))) {
		linux = 1;
	} else if(strncmp(unameData.sysname, "Windows_NT", sizeof("Windows_NT")) != 0             //busybox-32 on Win10
	 || strncmp(unameData.sysname, "CYGWIN_NT-5.1", sizeof("CYGWIN_NT-5.1")) != 0             //Cygwin on WinXP
	 || strncmp(unameData.sysname, "CYGWIN_NT-6.1", sizeof("CYGWIN_NT-6.1")) != 0             //Cygwin 1.7 on Win7 32-bit core i7
	 																						  //Cygwin 1.7 64-bit on Win7 64-bit
	 || strncmp(unameData.sysname, "CYGWIN_NT-6.1-WOW64", sizeof("CYGWIN_NT-6.1-WOW64")) != 0 //Cygwin 1.7 on Win7 64-bit
	 || strncmp(unameData.sysname, "CYGWIN_NT-10.0", sizeof("CYGWIN_NT-10")) != 0             //Cygwin 2.2 on Win10 64-bit
	 || strncmp(unameData.sysname, "MS-DOS", sizeof("MS-DOS")) != 0 						  //DJGPP v2 32-bit on Windows Server 2008
	 ||	strncmp(unameData.sysname, "WindowsNT", sizeof("WindowsNT")) != 0 					  //UnxUtils 2007 32-bit on Windows Server 2008
	 || strncmp(unameData.sysname, "UWIN-W7", sizeof("UWIN-W7")) != 0						  //UWIN 64-bit Win7 core i5
	 || strncmp(unameData.sysname, "ERROR", sizeof("ERROR"))) {								  //Default Windows
		windows = 1;
	}
}

//Debugging
void printOS() {
	printf("System name: %s\n", unameData.sysname);
	printf("Node name:   %s\n", unameData.nodename);
	printf("Release:     %s\n", unameData.release);
	printf("Version:     %s\n", unameData.version);
	printf("Machine:     %s\n", unameData.machine);
	printf("Add %s to flags\n", unameData.sysname);
}

//Debugging
void printTable() {
	struct program_list *s;

	for(s=programs; s != NULL; s=(struct program_list*)(s->hh.next)) {
		printf("Process name: %s\tMemory Usage: %d\n", s->process, s->memory_usage);
	}
}

//Add a program to the hash table
void add_program(char *process, int integer) {
	struct program_list *p;

	//printf("Program %s not found. Adding...\n", process);

	p = (struct program_list*)malloc(sizeof(struct program_list));
	strcpy(p->process, process);
	p->memory_usage = integer;
	HASH_ADD_STR(programs, process, p);

	//printf("Added %s. Hashed with value %d.\n", process, integer);

}

//Find a program in the hash table
struct program_list *find_program(char *process) {
	//printf("Finding %s...\n", process);

	struct program_list *s;
	HASH_FIND_STR(programs, process, s);

	//printf("Returning from find_program\n");

	return s;
}

//Hash a program
void hash_it(char *process, int memory_usage) {
	//printf("\n");
	//printf("Dave's function received: \n");
	//printf("Image Name: %s\n", process);
	//printf("Mem usage (M): %d\n", memory_usage);

	struct program_list *current_program;

	int x;
	current_program = find_program(process);
	//Is the input in the hash table already?
	if(current_program == NULL) {
		//If it isn't, add it
		add_program(process, memory_usage);
	}
	else if(memory_usage > (current_program->memory_usage * 3/2) && strcmp(process,"svchost.exe") != 0 && strcmp(process, "WmiPrvSE.exe") != 0 && strcmp(process, "fontdrvhost.exe") != 0
		&& strcmp(process, "NVDisplay.Container.exe") != 0 && strstr(process, "CEF") == NULL) {
		//If it is, check the stored memory usage
		//If the new memory usage is 3/2x bigger, kill the process
		//printf("%s determined to be potential rabbit.\n", process);
		//printf("The old memory usage was: %d\n", current_program->memory_usage);
		//printf("The new usage is: %d\n, memory_usage);
		//printf("Killing...\n");
		if(windows) {
			char *command = "taskkill /IM  /F";
			char kill_command[50];
			int insert = 13;
			strncpy(kill_command, command, insert);
			kill_command[insert] = '\0';
			strcat(kill_command, process);
			strcat(kill_command, command + 13);
			system(kill_command);
		} else if(linux) {
			char *command = "killall";
			char kill_command[50];
			int insert = 13;
			strncpy(kill_command, command, insert);
			kill_command[insert] = '\0';
			strcat(kill_command, process);
			strcat(kill_command, command + 13);
			system(kill_command);
		}
		//printf("\n----------------------------\n");
		//printf(kill_command);
		//printf("\n----------------------------\n");
		//printf("%s killed.\n", process);
		//And remove the process from the hash table
		//printf("Removing %s from hash table...\n", process);
		HASH_DEL(programs, current_program);
		free(current_program);
		//printf("%s removed.\n", process);
	} else {
		//Otherwise, update the memory usage
		//printf("%s was not identified as a potential rabbit\nUpdating memory usage from %d to %d", process, current_program->memory_usage, memory_usage);
		HASH_DEL(programs, current_program);
		free(current_program);
		add_program(process, memory_usage);
		//printf("%s updated.\n", process);
		
	}
}

int string_to_integer(char *string) {
	int return_int = 0;
	return_int = atoi(string);
	return return_int;
}

void parse_token(char *string) {
	//string size = 76
	char image_name[76];
	char mem_usg[76];
	int index = 0;
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == ' ' && string[i+1] == ' ') {
			index = i-1;
			break;
		}
	}
	memset(image_name, '\0', sizeof(image_name));
	strncpy(image_name, string, index+1);
	for (int i = 75; i > 0; i--) {
		if (string[i] == 'K') {
			string[i] = '\0';
			string[i - 1] = '\0';
		}
		if (string[i] == ' ' && string[i - 1] == ' ') {
			index = i;
			break;
		}
	}
	memset(mem_usg, '\0', sizeof(mem_usg));
	char * temp_string = string;
	temp_string += index+1;
	strncpy(mem_usg, temp_string, index); 
	hash_it(image_name, string_to_integer(mem_usg));
}

int main() {
	determineOS();
	if(windows) {
		while(1) {
			//method for taking _popen and placing it into string was borrowed from:
			//https://stackoverflow.com/questions/26932616/read-input-from-popen-into-char-in-c
			char buffer[100];
			char *out_string = NULL;
			char *temp = NULL;
			FILE *file;
			file = _popen("tasklist", "r");
			unsigned int size = 1;
			unsigned int length;
			char line[64];

			if (file == NULL) {
				printf("oh darn...");
			}

			while (fgets(line, sizeof(line), file)) {
				length = strlen(line);
				temp = realloc(out_string, size + length);
				out_string = temp;
				strcpy(out_string + size - 1, line);
				size += length;
			}

			_pclose(file);
			out_string += 232;
			char *token;
			char *temp_string;
			token = strtok(out_string, "\n");
			while (token != NULL) {
				temp_string = token;
				parse_token(temp_string);
				token = strtok(NULL, "\n");
			}
			//printf("Printing the hash table:\n");
			//printTable();
			//free up the buffers
		}
	} else if(linux) {
		while(1) {
			char buffer[100];
			char *out_string = NULL;
			char *temp = NULL;
			FILE *file;
			file = popen("ps -e", "r");
			unsigned int size = 1;
			unsigned int length;
			char line[64];

			if (file == NULL) {
				printf("oh darn...");
			}

			while (fgets(line, sizeof(line), file)) {
				length = strlen(line);
				temp = realloc(out_string, size + length);
				out_string = temp;
				strcpy(out_string + size - 1, line);
				size += length;
			}

			pclose(file);
			out_string += 232;
			char *token;
			char *temp_string;
			token = strtok(out_string, "\n");
			while (token != NULL) {
				temp_string = token;
				parse_token(temp_string);
				token = strtok(NULL, "\n");
			}
			//printf("Printing the hash table:\n");
			//printTable();
			//free up the buffers
		}
	} else {
		printOS();
	}
	return 0;
}