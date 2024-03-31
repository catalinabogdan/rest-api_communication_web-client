#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookies, int cookies_count, char *authorization)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));


    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    sprintf(line,"Host: %s", host);
    compute_message(message, line);

    if (authorization != NULL) {
        sprintf(line, "Authorization: Bearer %s", authorization);
        compute_message(message, line);
    }


    if (cookies != NULL) {
       sprintf(line,"Cookie: %s", cookies);
       compute_message(message, line);
    }

    compute_message(message, "");
    free(line);
    return message;
}
char *compute_delete_request(char *host, char *url, char *query_params,
                            char *cookies, int cookies_count, char *authorization)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));


    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    sprintf(line,"Host: %s", host);
    compute_message(message, line);

    if (authorization != NULL) {
        sprintf(line, "Authorization: Bearer %s", authorization);
        compute_message(message, line);
    }


    if (cookies != NULL) {
       sprintf(line,"Cookie: %s", cookies);
       compute_message(message, line);
    }

    compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char *cookies, int cookies_count,
                            char *authorization)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));
  //  memcpy(body_data_buffer,body_data,body_data_fields_count);

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    sprintf(line, "Host: %s", host);
    compute_message(message,line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    int content_len = 0;
    sprintf(line, "%s", body_data[0]);
    content_len += strlen(body_data[0]);
    strcat(body_data_buffer, line);

    sprintf(line, "Content-Length: %d", content_len);
    compute_message(message, line);

    if (authorization != NULL) {
        sprintf(line, "Authorization: Bearer %s", authorization);
        compute_message(message, line);
    }

    if (cookies != NULL) {
        sprintf(line, "Cookie: %s", cookies);
        compute_message(message, line);
    }
    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}


