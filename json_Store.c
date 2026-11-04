#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>


struct json_object *parse_json_file(const char *filename)
{
	FILE *fp = fopen(filename,"r");

	if(fp == NULL)
		return NULL;

	fseek(fp,0,SEEK_END);
	long size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char *buffer = malloc(size+1);

	if(buffer == NULL) {
		fclose(fp);
		return NULL;
	}

	fread(buffer,1,size,fp);
	buffer[size] ='\0';

	fclose(fp);

	struct json_object *root = json_tokener_parse(buffer);

	free(buffer);

	return root;
}


/* Save JSON file */
int save_json_file(const char *filename,struct json_object *root)

{
    const char *json_string = json_object_to_json_string_ext( root, JSON_C_TO_STRING_PRETTY);


    FILE *fp = fopen(filename,"w");

    if(fp ==NULL)
	    return 0;

    fprintf(fp,"%s\n",json_string);

    fclose(fp);
    
    return 1;

}
