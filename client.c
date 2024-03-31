#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

#include "helpers.h"
#include "buffer.h"
#include "parson.h"
#include "requests.h"

#define ip_address "34.254.242.81"
#define port 8080

char *buff, *log_cookie, *token;

int main(int argc, char *argv[]){
    
    int sockfd;
    buff = calloc(8000,sizeof(buff));
    log_cookie = calloc(8000,sizeof(log_cookie));
    token = calloc(8000, sizeof(token));
    int token_len = 0;
    int cookie_len = 0;
    while(1){
        fgets(buff, 8000, stdin);
        buff[strlen(buff) - 1] = 0; // eliminam terminatorul de sir
        sockfd = open_connection(ip_address, port, AF_INET, SOCK_STREAM,0);
        if(!strncmp(buff,"exit",strlen("exit"))){
            break;
        }
        else if(!strcmp(buff,"register")){
            char *username, *password, *to_send, *msg, *resp;
            username = calloc(100, sizeof(char));
            password = calloc(100, sizeof(char));
            msg      = calloc(2000,sizeof(char));
            resp     = calloc(2000,sizeof(char)); 

            printf("username= ");
            fgets(username,100,stdin);
            username[strlen(username) - 1] = '\0';
            if(has_no_space(username) == 1 && strlen(username) != 0){
                printf("password= ");
                fgets(password,100,stdin);
                password[strlen(password) - 1] = '\0';
                if(has_no_space(password) == 1 && strlen(password) != 0){
                    JSON_Value *val = json_value_init_object();
                    // adaugam intr-un json username-ul si parola pentru a fi preluate de server
                    JSON_Object *object = json_value_get_object(val);
                    json_object_set_string_with_len(object,"username",username,strlen(username));
                    json_object_set_string_with_len(object,"password",password,strlen(password));
                    to_send = json_serialize_to_string_pretty(val);

                    msg = compute_post_request(ip_address,"/api/v1/tema/auth/register",
                                                "application/json", &to_send, 1, NULL, 0, NULL);

                    send_to_server(sockfd, msg);
                    resp = receive_from_server(sockfd);

                    if(strstr(resp,"is taken") != NULL){
                        printf("Already registred!\n");
                    } else {
                        printf("Registration was successfull!\n");
                    }
                    json_value_free(val);
                } else {
                    printf("The credentials shouldn't contain spaces!\n");
                }
            } else {
                printf("The credentials shouldn't contain spaces!\n");
            }
            
            json_free_serialized_string(to_send);
            free(username);
            free(password);
            free(msg);
            free(resp);

        } else if(!strcmp(buff,"login")) {
            char *username, *password, *to_send, *msg, *resp;
            username = calloc(100, sizeof(char));
            password = calloc(100, sizeof(char));
            to_send  = calloc(2000, sizeof(char));
            msg      = calloc(2000,sizeof(char));
            resp     = calloc(2000,sizeof(char));

            printf("username= ");
            fgets(username,100,stdin);
            username[strlen(username) - 1] = '\0';
            if(has_no_space(username) == 1 && strlen(username) != 0){
                printf("password= ");
                fgets(password,100,stdin);
                password[strlen(password) - 1] = '\0';
                
                if(has_no_space(password) && strlen(password) != 0){
                    JSON_Value *val = json_value_init_object();
                    // adaugam intr-un json username-ul si parola pentru a fi preluate de server
                    JSON_Object *object = json_value_get_object(val);
                    json_object_set_string_with_len(object,"username",username,strlen(username));
                    json_object_set_string_with_len(object,"password",password,strlen(password));
                    to_send = json_serialize_to_string_pretty(val);

                    
                    msg = compute_post_request(ip_address,"/api/v1/tema/auth/login",
                                                "application/json", &to_send, 1, NULL, 0, NULL);
                    send_to_server(sockfd, msg);
                    resp = receive_from_server(sockfd);
                    // extragem cookie-ul de logare
                    log_cookie = extract_cookie(resp);
                    if(log_cookie == NULL) {
                        printf("Login failed!\n");
                    } else {
                        printf("You logged in successfully!\n");
                        cookie_len = strlen(log_cookie);
                    }
                    json_value_free(val);
                } else{
                    printf("The credentials shouldn't contain spaces!\n");
                }
            } else {
                printf("The credentials shouldn't contain spaces!\n");
            }

            json_free_serialized_string(to_send);
            free(username);
            free(password);
            free(msg);
            free(resp);

        } else if(!strcmp(buff,"enter_library")) {
            char *msg, *resp;
            msg  = calloc(2000,sizeof(char));
            resp = calloc(2000,sizeof(char));
            // verificam daca utilizatorul este logat
            if(cookie_len != 0){
                msg = compute_get_request(ip_address, "/api/v1/tema/library/access", NULL, log_cookie, 1, NULL);
                send_to_server(sockfd,msg);
                resp = receive_from_server(sockfd);
                char *start = strstr(resp, "token");
                if(start == NULL) {
                    printf("Unauthorized user!\n");
                } else {
                    start = start + sizeof("token") + 2; // formatul : "token":"..(token).." 
                                                        // pointam catre  primul caracter al tokenului 
                    memcpy(token,start,strlen(start));
                    token[strlen(start)-2] = '\0';
                    printf("You entered the library!\n");
                    // actualizam lungimea tokenului pentru verificari ulterioare
                    token_len = strlen(token);
                }
            } else {
                printf("You cannot enter the library without being logged in!\n");
            }
            free(msg);
            free(resp);

        } else if(!strcmp(buff,"get_books")) {
            char *books, *msg, *resp;
            msg  = calloc(8000,sizeof(char));
            resp = calloc(8000,sizeof(char));
            // verificam daca ne este permis accesul la biblioteca
            if(token_len != 0){
                msg = compute_get_request(ip_address, "/api/v1/tema/library/books", NULL, log_cookie, 1, token);
                send_to_server(sockfd,msg);
                resp = receive_from_server(sockfd);
                books = strstr(resp, "[");

                // parsam sirul de carti in functie de obiecte json
                JSON_Value *value = json_value_init_object();
                value = json_parse_string(books);
                char *pretty = json_serialize_to_string_pretty(value);
                printf("%s\n", pretty);
                json_free_serialized_string(pretty);
                json_value_free(value);
                free(msg);
                free(resp);
            } else {
                printf("You don't have access to the library.\n");
            }
            

        } else if(!strcmp(buff, "add_book")) {
            // verificam daca ne este permis accesul la biblioteca
            if(token_len != 0) {
                char *title, *author, *publisher, *genre, *to_send, *msg, *page_count;
                int chances = 2;
                title     = calloc(100, sizeof(char));
                author    = calloc(100, sizeof(char));
                publisher = calloc(100, sizeof(char));
                genre     = calloc(100, sizeof(char));
                to_send   = calloc(8000, sizeof(char));
                msg       = calloc(8000, sizeof(char));
                page_count= calloc(100,sizeof(char));

                // utilizatorul are 2 sanse sa introduca valori valide
                while(chances) {
                    printf("title= ");
                    fgets(title,100,stdin);
                    title[strlen(title) - 1] = '\0';
                    if(strlen(title) == 0 ){
                        printf("Invalid input!\n");
                        chances --;
                    } else {
                        chances = 0;
                    }
                }
                chances = 2;
                while(chances) {
                    printf("author= ");
                    fgets(author,100,stdin);
                    author[strlen(author) - 1] = '\0';
                    if(strlen(author) == 0){
                        printf("Invalid input!\n");
                        chances --;
                    } else {
                        chances = 0;
                    }
                }
                chances = 2;
                while(chances) {
                    printf("genre= ");
                    fgets(genre,100,stdin);
                    genre[strlen(genre) - 1] = '\0';
                    if(strlen(genre) == 0){
                        printf("Invalid input!\n");
                        chances --;
                    } else {
                        chances = 0;
                    }
                }
                chances = 2;
                int nr;
                while(chances) {
                    printf("page_count= ");
                    fgets(page_count,100,stdin);
                    page_count[strlen(page_count) - 1] = '\0';
                    if(sscanf(page_count,"%d",&nr)== -1 || has_no_chars(page_count) == 0){
                        printf("Invalid input!\n");
                        chances --;
                    } else {
                        chances = 0;
                    }
                }
                chances = 2;
                while(chances) {
                    printf("publisher= ");
                    fgets(publisher,100,stdin);
                    publisher[strlen(publisher) - 1] = '\0';
                    if(strlen(publisher) == 0 ){
                        printf("Invalid input!\n");
                        chances --;
                    } else {
                        chances = 0;
                    }
                }
                JSON_Value *val = json_value_init_object();
                // salvam informatiile intr-un json care va fi preluat de server
                JSON_Object *object = json_value_get_object(val);
                json_object_set_string_with_len(object,"title",title,strlen(title));
                json_object_set_string_with_len(object,"author",author,strlen(author));
                json_object_set_string_with_len(object,"genre",genre,strlen(genre));
                json_object_set_number(object,"page_count",nr);
                json_object_set_string_with_len(object,"publisher",publisher,strlen(publisher));

                to_send = json_serialize_to_string_pretty(val);

                msg = compute_post_request(ip_address,"/api/v1/tema/library/books",
                                            "application/json", &to_send, 1, NULL, 0, token);
                send_to_server(sockfd, msg);
                printf("The book %s was added successfully to your library.\n", title);
                free(title);
                free(author);
                free(genre);
                free(publisher);
                free(msg);
                free(to_send);
                free(page_count);

            } else {
                printf("You don't have access to the library.\n");
            }
        } else if(!strcmp(buff,"delete_book")) {
            char *new_url, *msg;
            new_url = calloc(200,sizeof(char));
            msg     = calloc(8000, sizeof(char));
            // verificam daca ne este permis accesul la biblioteca
            if(token_len != 0){
                int id;
                printf("id= ");
                if(scanf("%d",&id) == 1) {
                    if(id < 0) {
                        printf("Invalid input!\n");
                    } else {
                        sprintf(new_url,"%s/%d","/api/v1/tema/library/books",id);
                        new_url[strlen(new_url)] = '\0';
                        // trimitem o cerere DELETE catre server
                        msg = compute_delete_request(ip_address, new_url, NULL, log_cookie, 1, token);
                        send_to_server(sockfd, msg); 
                        printf("Book with the id %d was deleted successfully from your library.\n", id);
                        free(msg);
                        free(new_url);
                    }
                } else {
                    printf("Invalid input!\n");
                }
            } else {
                printf("You don't have access to the library.\n");
            }
        }else if(!strcmp(buff, "get_book")) {
            char *new_url, *msg, *resp;
            new_url = calloc(200,sizeof(char));
            msg     = calloc(8000, sizeof(char));
            resp    = calloc(8000, sizeof(char));
            // verificam daca ne este permis accesul la biblioteca
            if(token_len != 0) {
                int id;
                printf("id= ");
                if(scanf("%d",&id) == 1){
                    if(id < 0) {
                        printf("Invalid input!\n");
                    } else {
                        sprintf(new_url,"%s/%d","/api/v1/tema/library/books",id);
                        new_url[strlen(new_url)] = '\0';
                        // trimitem o cerere GET cu url_ul corespunzator 
                        msg = compute_get_request(ip_address, new_url, NULL, log_cookie, 1, token);
                        send_to_server(sockfd, msg);
                        resp = receive_from_server(sockfd);
                        // parsam dupa titlu
                        char *book = strstr(resp, "id");
                        if (book != NULL) {
                            book -= 2; // pointam catre {
                            char *end = strchr(book, '}'); // cautam acolada de inchidere
                            if (end != NULL) {
                                int len = (int)(end - book) + 1;
                                char* obj = calloc(len + 1, sizeof(char));
                                strncpy(obj, book, len);
                                printf("suntt aici");
                                

                                // cream un obiect json pentru a afisa respectand formatul standard
                                JSON_Value *val = json_parse_string(obj);
                                JSON_Object *object = json_value_get_object(val);
                                json_object_set_number(object, "id", id);
                                char *pretty = json_serialize_to_string_pretty(val);
                                printf("%s\n", pretty);

                                json_free_serialized_string(pretty);
                                json_value_free(val);
                                free(obj);
                            }
                    
                        }else {
                                printf("The book you are searching for is not there.\n");
                            }
                    }
                } else {
                    printf("Invalid input!\n");
                }
                free(msg);
                free(resp);
                free(new_url);
            } else {
                printf("You must enter the library!\n");
            }
        } else if(!strcmp(buff, "logout")) {
            // verificam daca utilizatorul este logat
            if(cookie_len != 0){
                char *msg;
                msg = calloc(8000,sizeof(char));
                msg = compute_get_request(ip_address,"/api/v1/tema/auth/logout",
                                    NULL, log_cookie, 1, NULL);
                send_to_server(sockfd, msg);
                free(token);
                free(log_cookie);
                // dupa logout, actualizam lungimile cookie-ului si token-ului
                cookie_len = 0;
                token_len = 0;
                
                printf("You are logged out.\n");
                free(msg);
            } else {
                printf("You are not logged in!\n");
            }
        }
    
    }
    // verificam daca au fost eliberate in prealabil
    if(cookie_len != 0){
        free(log_cookie);
    }
    if(token_len != 0){
        free(token);
    }
    free(buff);
    // inchidem socket-ul
    close(sockfd);
    return 0;
}