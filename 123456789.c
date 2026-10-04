#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "http.h"


#define ERROR(x) { error(x, __FILE__, __LINE__); } 

void error(char *str, char *file, int line)
{
	fprintf(stderr, "[%s:%d]=%s\n", file, line, str);
	exit(1);
}


void print_string_as_hex(char *str)
{
	char *scroll = str;
	for(;*scroll != '\0'; ++scroll)
		printf("0x%2x ", *scroll); 
	printf("\n");
}


int get_length_file(FILE *fp)
{
	if(fp == NULL)
		ERROR("file ptr is invalid");
	int current = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, current,SEEK_SET);
	return length;
}

char *read_file(FILE *fp)
{
	int length = get_length_file(fp);
	char *file = (char *)calloc(1, length + 1);
	if(!file)
		ERROR("bad malloc");

	fread(file, 1, length, fp);
	fseek(fp, 0, SEEK_SET);
	file[length] = 0;
	return file;
}





struct dstring
{
	char *buffer;
	int length;
	int alloc;
	int scroll;
};


struct dstring *init_dstring(unsigned int alloc)
{
	
	struct dstring *dstr = malloc(sizeof(struct dstring));
	if(!dstr)
		ERROR("dstr malloc failed");
	char *pbuf = calloc(alloc, sizeof(char));
	if(!pbuf)
		ERROR("pbuf calloc failed");
	dstr->alloc = alloc;
	dstr->buffer = pbuf;
  	dstr->length = 0;
	dstr->scroll = 0;	
	return dstr;
}

void add_char_dstring(struct dstring *dstr, char ch)
{
	//printf("%d %d\n", dstr->length + 1, dstr->alloc);	
	if(dstr->length + 1 >= dstr->alloc) 
	{
		int new_alloc = dstr->alloc * 2 + 1;
		char *temp = realloc(dstr->buffer, new_alloc * sizeof(char));
		if(!temp)
			ERROR("realloc failed");
		dstr->buffer = temp;
		dstr->alloc = new_alloc;
		
	}
	dstr->buffer[dstr->scroll++] = ch;
	dstr->length++;
	return;
}	

void add_str_dstring(struct dstring *dstr, char *in)
{
	for(char *scroll = in; *scroll != '\0'; ++scroll)
		add_char_dstring(dstr, *scroll);
}

char *get_str_dstring(struct dstring *dstr)
{
	add_char_dstring(dstr, 0);
	return dstr->buffer;
}
/*
bool check_str_cmp(char *a, char *b)
{
	int count = 0;
	for(char *ca = a, *cb = b; *ca != '\0' && *cb != '\0'; ca++, cb++)
	{
		if(*ca == *cb)
		{
			count++;		
		}
		else
		{
			printf("%c %c\n", *ca, *cb);
		}
	}	
	return true;
}
*/


void print_http_head_field(struct HTTPHead *p)
{
	printf("%s: %s\n", p->name, p->value);
}

void print_http_head_fields_list(struct HTTPHead **list, int count)
{
	printf("printing fields\n");
	for(int i = 0; i < count; ++i)
	{
		print_http_head_field(list[i]);
	}
}

char *copy_subset(char *str, int offset, int length)
{
	char *new = malloc(length + 1);
	if(new == NULL)
	{
		ERROR("malloc failed");
	}
	for(int i = 0; i < length; ++i)
	{
		char c = *(str + i + offset);
		if(c == '\0')
			ERROR("string length and offset thats too long");
		new[i] = c;
	}

	new[length ] = 0; 
	return new;
}


int header_field_pull(char *buf, int buflen, char *str)
{
	char *scroll = str;
	char last = 0;
	int i = 0;
	for(; *scroll != '\0' && !(last == '\r' && *scroll == '\n'); last = *(scroll++), i++)
	{
		if(i >= buflen)
			ERROR("bad");
		buf[i] = *scroll;
	}	
	if(i == 1 || i == 0)
		return 0;
	if(i == buflen)
		ERROR("biggest bad boy");
	buf[i] = 0; 
	buf[i-1] = 0;
	return i;
}

struct HTTPHead *http_head(char *line)
{

	int length = strlen(line);
	int split = 0;


	struct HTTPHead *head = malloc(sizeof(struct HTTPHead));
	if(head == NULL)
		ERROR("failed to malloc");
	
	for(int i = 0; i < length; ++i)
	{
		if(line[i] != ':')
			split++;
		else
			break;
	}
	if(split == length)
	{
		printf("%s\n", line); 
		ERROR("split == length")
	}

	char *name = copy_subset(line, 0, split);
	char *value = copy_subset(line, split + 2, length - split - 2 );
	if(!name || !value)
		ERROR("name | value == null");



   	head->name = name;
	head->value = value;
	return head;
}

int fill_http_head(char **text, struct HTTPHead ***list)
{
	int text_len = strlen(*text);
	char *scroll = *text;
	char *buf = malloc(text_len + 1);
	if(buf == NULL)
		ERROR("malloc failed");

	//god you are ugly
	//
	
	int count = 0;
	bool is_line = false;
	bool repeat = false;
	for(char *check_ahead = scroll, last = 0; *check_ahead != EOF && *check_ahead != '\0'; last = *(check_ahead++))
	{
		if(*check_ahead == '\n' && last == '\r')
		{	
			if(repeat == true)
			{
				//printf("how many %d\n", count);
				break;
			}
			else
			{

				//printf("[%d]\n", count);
				//print_string_as_hex(check_ahead);	
				count++;
				repeat = true;
			}
		}
		else if(repeat == true && *check_ahead == '\r')	
		{
			repeat = true;
		}
		else 
		{
			repeat = false;
		}
	}
	
	struct HTTPHead **heads = malloc(count * sizeof(struct HTTPHead *));
	if(heads == NULL)
		ERROR("malloc failed");
	//printf("heads! %s\n", scroll);
	for(int i = 0; i < count; ++i)
	{
		int header_data = header_field_pull(buf, text_len + 1, scroll);
		if(header_data == 0)
			break;
		scroll += header_data + 1;
		heads[i] = http_head(buf);
	}	
	free(buf);
	*list = heads;
	*text = scroll;
	return  count;

}


bool compare(FILE *f)
{
	char *file_data = read_file(f);

	struct HTTPRequest *request = textToHTTPRequest(f);

	char *request_to_text = packHTTPRequest(request);

	int cmp = strcmp(file_data, request_to_text);
	return (cmp == 0)? true:false;
   	
}



struct HTTPRequest * textToHTTPRequest(FILE * f)
{

	struct HTTPRequest *req = calloc(1, sizeof(struct HTTPRequest));
	if(req == NULL)
		ERROR("req == null");
	char *text = read_file(f);
	char *start = text;
	char *scroll = text;

	for(; *scroll != ' '&& *scroll != '\n' && *scroll != '\0' ; ++scroll);
	
	int delta1 = scroll - start;
	char *method =  copy_subset(text, 0, delta1);
	
	char *old_scroll1 = ++scroll;
	
	for(; *scroll != ' '&& *scroll != '\n' && *scroll != '\0' ; ++scroll);
	int delta2 = scroll - old_scroll1;
	//printf("delta2 %d\n",delta2);
	char *url =  copy_subset(text, delta1 + 1, delta2);
		
	scroll++;
	char *old_scroll2 = scroll;
	for(; *scroll != ' ' && *scroll != '\n' && *scroll != '\0' && *scroll != '\r'; ++scroll);
	
	int delta3 = scroll - old_scroll2;
	//printf("delta3 %d\n", delta3);
	
	char *version = copy_subset(text, delta2 + delta1 + 2, delta3);

	//printf("<%s> <%s> <%s>\n", command, next, http1_1); 
	scroll+=2;
	struct HTTPHead **list;
	int count = fill_http_head(&scroll, &list);	
	
	
	req->method = method; 
	req->url = url;
	req->version = version;
	req->headFieldCount = count;
	req->headFields = list;	
	req->body = copy_subset(scroll, 2, strlen(scroll)-2);		
	free(text);
	return req;
}

struct HTTPResponse * textToHTTPResponse(FILE * f)
{
	struct HTTPResponse * res = calloc(1, sizeof(struct HTTPResponse));
	if(!res)
		ERROR("calloc failed");
	

	char *text = read_file(f);
	if(!text)
		ERROR("read file failed");

	char *scroll = text;
	
	for(; *scroll != ' '&& *scroll != '\n' && *scroll != '\0' ; ++scroll);
	
	int delta1 = scroll - text;
	char *ref1 = scroll;
	char *version = copy_subset(text, 0, delta1);
	scroll++;

	
	for(; *scroll != ' '&& *scroll != '\n' && *scroll != '\0' ; ++scroll);
	
	int delta2 = scroll - ref1;
	char *ref2 = scroll;
	char *status_code = copy_subset(text, delta1 + 1, delta2 - 1);
	scroll++;
	
	for(; *scroll != '\r' && *scroll != '\n' && *scroll != '\0' ; ++scroll);

	int delta3 = scroll - ref2;
	
	char *message = copy_subset(text, delta1 + delta2 + 1, delta3 - 1);
	scroll+=2;
	struct HTTPHead **list;
	int count = fill_http_head(&scroll, &list);	
	
	//print_http_head_fields_list(list, count);		
	
	res->version = version; 	
	res->statusCode = status_code;
	res->phrase = message;
	res->headFieldCount = count;
	res->headFields = list;

	res->body = copy_subset(scroll, 2, strlen(scroll)-2);		
	free(text);

	return res;
}




char* packHTTPRequest(struct HTTPRequest * req)
{
	
	struct dstring *dstr = init_dstring(1);	
	
	add_str_dstring(dstr,req->method);
	
	add_char_dstring(dstr, ' ');	
	
	add_str_dstring(dstr,req->url);

	add_char_dstring(dstr, ' ');
	
	add_str_dstring(dstr, req->version);
	add_char_dstring(dstr, '\r');
	add_char_dstring(dstr, '\n');

	for(int i = 0; i < req->headFieldCount; ++i)
	{
		struct HTTPHead *head = req->headFields[i];

		add_str_dstring(dstr, head->name);
		add_char_dstring(dstr, ':');
		add_char_dstring(dstr, ' ');
		add_str_dstring(dstr, head->value);
		add_char_dstring(dstr, '\r');
		add_char_dstring(dstr, '\n');


	}	

	
	add_char_dstring(dstr, '\r');
	add_char_dstring(dstr, '\n');

	add_str_dstring(dstr, req->body);

	char* out =  get_str_dstring(dstr);
	return out;
}

char* packHTTPResponse(struct HTTPResponse * res)
{
	struct dstring *dstr = init_dstring(1);	
	
	add_str_dstring(dstr,res->version);
	
	add_char_dstring(dstr, ' ');	
	
	add_str_dstring(dstr,res->statusCode);

	add_char_dstring(dstr, ' ');
	
	add_str_dstring(dstr, res->phrase);
	add_char_dstring(dstr, '\r');
	add_char_dstring(dstr, '\n');

	for(int i = 0; i < res->headFieldCount; ++i)
	{
		struct HTTPHead *head = res->headFields[i];

		add_str_dstring(dstr, head->name);
		add_char_dstring(dstr, ':');
		add_char_dstring(dstr, ' ');
		add_str_dstring(dstr, head->value);
		add_char_dstring(dstr, '\r');
		add_char_dstring(dstr, '\n');


	}	

	add_char_dstring(dstr, '\r');
	add_char_dstring(dstr, '\n');
	add_str_dstring(dstr, res->body);
	

	char* out =  get_str_dstring(dstr);
	
	//print_string_as_hex(out);

	return out;
}

