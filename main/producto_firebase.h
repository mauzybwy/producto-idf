#ifndef _PRODUCTO_FIREBASE_H_
#define _PRODUCTO_FIREBASE_H_

#include "cJSON.h"

void firebase_write(char *path, cJSON *json_root);
cJSON* firebase_read(char *path);

#endif /* _PRODUCTO_FIREBASE_H_ */
