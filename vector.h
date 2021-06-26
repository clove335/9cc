#include "9cc.h"

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void test_vector();
void test_map();
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
