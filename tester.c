#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "http.h"

bool check_str_cmp(char *a, char *b)
{
	int count = 0;
	for(char *ca = a, *cb = b; *ca != '\0' && *cb != '\0'; ca++, cb++)
	{
		if(*ca == *cb)
		{
			printf("%c %c\n", *ca, *cb);
			count++;		
		}
		else
		{
			printf("%c %c\n", *ca, *cb);
		}
	}	
	return strcmp(a, b);
}

int main(int argc, char *argv[]){

	if(argc<2)
	{
		printf("Parent: Input file name missing...exiting with error code -1\n");
		return -1;
	}

	struct stat st;
	stat(argv[1], &st);

	FILE *in = fopen(argv[1], "r");
	FILE *in2 = fopen(argv[2], "r");
	if(!in)
	{
		printf("Parent: Error in opening input file...exiting with error code -1\n");
		return -1;
	}

/* This is just a hint about reading the file
	fstat(fileno(in), &st);
	
	char* data = (char*)malloc((st.st_size*sizeof(char))+1);
	
	int i = 0;
	int c = 0;

	while((c=fgetc(in))!=EOF)
		data[i++]=c;
	data[i] ='\0';
	fclose(in);
*/	
	

	char *readOut = read_file(in); 
	char *readIn = read_file(in2);

	struct HTTPRequest *request = textToHTTPRequest(in);
	struct HTTPResponse *response = textToHTTPResponse(in2);
	
	char* dataOut = packHTTPRequest(request);	
	char *dataIn = packHTTPResponse(response);	
	
	printf("out: %d\n", check_str_cmp(readOut, dataOut));
	printf("in: %d\n", check_str_cmp(readIn, dataIn));
	
	printf("%s\n", dataOut);	
	fclose(in);
	
	//printf("%s",dataOut);
	return 0;
}
