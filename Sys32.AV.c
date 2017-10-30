#include "uthash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
//#include <windows.h>

//authors: Alexander Barnhart, Dave Kelly
//uthash is an open source C hash library. http://troydhanson.github.io/uthash

struct program_list {
	char process[76];
	int memory_usage;
	UT_hash_handle hh;
};

struct program_list *programs = NULL;

void printTable() {
	struct program_list *s;

	for(s=programs; s != NULL; s=(struct program_list*)(s->hh.next)) {
		printf("Process name: %s\tMemory Usage: %d\n", s->process, s->memory_usage);
	}
}

void add_program(char *process, int integer) {
	struct program_list *p;

	printf("Program %s not found. Adding...\n", process);

	p = (struct program_list*)malloc(sizeof(struct program_list));
	strcpy(p->process, process);
	p->memory_usage = integer;
	HASH_ADD_STR(programs, process, p);

	printf("Added %s. Hashed with value %d.\n", process, integer);

}

struct program_list *find_program(char *process) {
	printf("Finding %s...\n", process);

	struct program_list *s;
	HASH_FIND_STR(programs, process, s);

	printf("Returning from find_program\n");

	return s;
}

void hash_it(char *process, int memory_usage)
{
	printf("\n");
	printf("Dave's function received: \n");
	printf("Image Name: %s\n", process);
	printf("Mem usage (M): %d\n", memory_usage);

	struct program_list *current_program;

	int x;
	current_program = find_program(process);
	//Is the input in the hash table already?
	if(current_program == NULL) {
		//If it isn't, add it
		add_program(process, memory_usage);
	}
	else if(memory_usage > (current_program->memory_usage * 3/2)) {
		//If it is, check the stored memory usage
		//If the new memory usage is 3/2x bigger, kill the process
		printf("%s determined to be potential rabbit.\nThe old memory usage was: %d\nThe new usage is: %d\nKilling...\n", process, current_program->memory_usage, memory_usage);
		char *command = "taskkill /IM  /F";
		char kill_command[50];
		int insert = 13;
		strncpy(kill_command, command, insert);
		kill_command[insert] = '\0';
		strcat(kill_command, process);
		strcat(kill_command, command + 13);
		printf("\n----------------------------\n");
		printf(kill_command);
		printf("\n----------------------------\n");
		system(kill_command);
		printf("%s killed.\n", process);
		//And remove the process from the hash table
		printf("Removing %s from hash table...\n", process);
		HASH_DEL(programs, current_program);
		free(current_program);
		printf("%s removed.\n", process);
	} else {
		//Otherwise, update the memory usage
		printf("%s was not identified as a potential rabbit\nUpdating memory usage from %d to %d", process, current_program->memory_usage, memory_usage);
		HASH_DEL(programs, current_program);
		free(current_program);
		add_program(process, memory_usage);
		printf("%s updated.\n", process);
	}
}
int string_to_integer(char *string)
{
	int return_int = 0;
	return_int = atoi(string);
	return return_int;
}
void parse_token(char *string)
{
	//string size = 76
	char image_name[76];
	char *mem_usg[76];
	int index = 0;
	for (int i = 0; i < strlen(string); i++)
	{
		if (string[i] == ' ' && string[i+1] == ' ')
		{
			index = i-1;
			break;
		}
	}
	memset(image_name, '\0', sizeof(image_name));
	strncpy(image_name, string, index+1);
	for (int i = 75; i > 0; i--)
	{
		if (string[i] == 'K')
		{
			string[i] = '\0';
			string[i - 1] = '\0';
		}
		if (string[i] == ' ' && string[i - 1] == ' ')
		{
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

int main()
{
	while(1)
	{
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

		if (file == NULL)
			printf("oh darn...");

		while (fgets(line, sizeof(line), file))
		{
			length = strlen(line);
			temp = realloc(out_string, size + length);
			out_string = temp;
			strcpy(out_string + size - 1, line);
			size += length;
		}

		_pclose(file);
		out_string += 232;
		char *token;
		char *token2;
		char *temp_string;
		token = strtok(out_string, "\n");
		while (token != NULL)
		{
			temp_string = token;
			parse_token(temp_string);
			token = strtok(NULL, "\n");
		}
		printf("Printing the hash table:\n");
		printTable();
	}
	return 0;
}