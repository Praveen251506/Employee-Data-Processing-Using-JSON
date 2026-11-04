#ifndef JSON_STORE_H
#define JSON_STORE_H

#include <json-c/json.h>

struct json_object *parse_json_file(const char *filename);

int save_json_file(const char *filename,
                   struct json_object *root);

#endif

