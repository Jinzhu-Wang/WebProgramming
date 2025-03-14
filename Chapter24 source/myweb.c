#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define SMALL_BUF 1024

void* request_handler(void* arg);
void send_data(FILE* fp, char* ct, char* file_name);
char* content_type(char* file);
void send_error(FILE* fp);
void error_handling(char* message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_size;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 20) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_adr_size = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_size);
        if (clnt_sock == -1) continue;

        printf("Connection Request : %s:%d\n", 
               inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));

        int *clnt_sock_ptr = malloc(sizeof(int));
        if (clnt_sock_ptr == NULL) {
            close(clnt_sock);
            continue;
        }
        *clnt_sock_ptr = clnt_sock;

        if (pthread_create(&t_id, NULL, request_handler, clnt_sock_ptr) != 0) {
            free(clnt_sock_ptr);
            close(clnt_sock);
            continue;
        }
        pthread_detach(t_id);
    }
    close(serv_sock);
    return 0;
}

void* request_handler(void *arg)
{
    if (arg == NULL) return NULL;
    int clnt_sock = *((int*)arg);

    char req_line[SMALL_BUF];
    FILE *clnt_read = fdopen(clnt_sock, "r");
    FILE *clnt_write = fdopen(dup(clnt_sock), "w");
    if (!clnt_read || !clnt_write) {
        if (clnt_read) fclose(clnt_read);
        if (clnt_write) fclose(clnt_write);
        free(arg);
        return NULL;
    }

    if (fgets(req_line, SMALL_BUF, clnt_read) == NULL) {
        send_error(clnt_write); // send_error 会关闭 clnt_write
        fclose(clnt_read);
        free(arg);
        return NULL;
    }

    if (strstr(req_line, "HTTP/") == NULL) {
        send_error(clnt_write);
        fclose(clnt_read);
        free(arg);
        return NULL;
    }

    char method[10], ct[15], file_name[30];
    char *token = strtok(req_line, " /");
    if (!token) {
        send_error(clnt_write);
        fclose(clnt_read);
        free(arg);
        return NULL;
    }
    strncpy(method, token, sizeof(method) - 1);
    method[sizeof(method) - 1] = '\0';

    token = strtok(NULL, " /");
    if (!token) {
        send_error(clnt_write);
        fclose(clnt_read);
        free(arg);
        return NULL;
    }
    strncpy(file_name, token, sizeof(file_name) - 1);
    file_name[sizeof(file_name) - 1] = '\0';

    strcpy(ct, content_type(file_name));
    if (strcmp(method, "GET") != 0) {
        send_error(clnt_write);
        fclose(clnt_read);
        free(arg);
        return NULL;
    }

    fclose(clnt_read);
    send_data(clnt_write, ct, file_name); // send_data 会关闭 clnt_write
    free(arg); // 只在末尾释放
    return NULL;
}

void send_data(FILE* fp, char* ct, char* file_name)
{
    char protocol[] = "HTTP/1.0 200 OK\r\n";
    char server[] = "Server:Linux Web Server \r\n";
    char cnt_len[] = "Content-length:2048\r\n";
    char cnt_type[SMALL_BUF];
    char buf[BUF_SIZE];
    FILE *send_file;

    sprintf(cnt_type, "Content-type:%s\r\n\r\n", ct);
    send_file = fopen(file_name, "r");
    if (send_file == NULL) {
        send_error(fp); // send_error 会关闭 fp
        return;
    }

    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);

    while (fgets(buf, BUF_SIZE, send_file) != NULL) {
        fputs(buf, fp);
        fflush(fp);
    }
    fflush(fp);
    fclose(fp); // 正常关闭 fp
    fclose(send_file);
}

char* content_type(char* file)
{
    char extension[SMALL_BUF];
    char file_name[SMALL_BUF];
    strncpy(file_name, file, sizeof(file_name) - 1);
    file_name[sizeof(file_name) - 1] = '\0';

    char *token = strtok(file_name, ".");
    if (!token) return "text/plain";

    token = strtok(NULL, ".");
    if (!token) return "text/plain";

    strncpy(extension, token, sizeof(extension) - 1);
    extension[sizeof(extension) - 1] = '\0';

    if (!strcmp(extension, "html") || !strcmp(extension, "htm"))
        return "text/html";
    return "text/plain";
}

void send_error(FILE* fp)
{
    char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
    char server[] = "Server:Linux Web Server \r\n";
    char cnt_len[] = "Content-length:2048\r\n";
    char cnt_type[] = "Content-type:text/html\r\n\r\n";
    char content[] = "<html><head><title>NETWORK</title></head>"
                     "<body><font size=+5><br>Error occurred! Check request file or type!"
                     "</font></body></html>";

    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);
    fputs(content, fp);
    fflush(fp);
    fclose(fp); // 关闭 fp
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}