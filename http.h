#ifndef __HEADER_FILE__
#define __HEADER_FILE__
#include <stdio.h>
#include <stdbool.h>

struct HTTPHead{
	char* name;
	char* value;
};

struct HTTPRequest{
	char* method;
	char* url;
	char* version;
	int headFieldCount;
	struct HTTPHead** headFields;
	char* body;
};

struct HTTPResponse{
	char* version;
	char* statusCode;
	char* phrase;
	int headFieldCount;
	struct HTTPHead** headFields;
	char* body;
};

struct HTTPRequest * textToHTTPRequest(FILE * f);

struct HTTPResponse * textToHTTPResponse(FILE * f);
char* packHTTPResponse(struct HTTPResponse * res);

char* packHTTPRequest(struct HTTPRequest * req);

bool compare(FILE *f);
char *read_file(FILE *fp);

#endif
