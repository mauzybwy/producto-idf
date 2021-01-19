#ifndef _PRODUCTO_HTTP_H_
#define _PRODUCTO_HTTP_H_

/* void http_init(void); */
void http_get(char* url, char *response_buffer);
void http_patch(char *url, char *patch_data);

#endif /* _PRODUCTO_HTTP_H_ */
