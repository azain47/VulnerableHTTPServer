#include<sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<pthread.h>
#include<regex.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

#define PORT 6969
// 1 MB buffer
#define BUFFER_SIZE 1048576

//route maps in regex
#define ROUTE_GET_ROOT "^GET / HTTP/1"
#define ROUTE_GET_FILE "^GET /file/([^ ]*) HTTP/1"
#define ROUTE_POST_ROOT "^POST / HTTP/1"
regex_t regex_GET_ROOT, regex_GET_FILE, regex_POST_ROOT;

int server;
struct sockaddr_in server_addr;

void compile_regexs(){
    regcomp(&regex_GET_ROOT, ROUTE_GET_ROOT, REG_EXTENDED);
    regcomp(&regex_GET_FILE, ROUTE_GET_FILE, REG_EXTENDED);
    regcomp(&regex_POST_ROOT, ROUTE_POST_ROOT, REG_EXTENDED);
}

void free_regexs(){
    regfree(&regex_GET_FILE);
    regfree(&regex_GET_ROOT);
    regfree(&regex_POST_ROOT); 
}

void build_http_header(char *header_buffer, int response_code, char *content_type, size_t content_len){
    char *response_msg = "OK";
    if (response_code == 404){
        response_msg = "Not Found";
    } else if (response_code == 400) {
        response_msg = "Bad Request";
    } else if (response_code == 500){
        response_msg = "Internal Server Error";
    }
    snprintf(header_buffer, BUFFER_SIZE,"HTTP/1.1 %d %s\r\nContent-Type:%s\r\nContent-Length:%zu\r\nConnection: close\r\n\r\n",
    response_code,response_msg, content_type, content_len);
}

void build_simple_http_response(int response_code, char *response_body, char *response_buffer, size_t *response_len){
    // currently we are serving txt only
    char *mime_type = "text/plain"; 
 
    char *body_content = (char *)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(body_content, BUFFER_SIZE,
        "%s\r\n", response_body);
    size_t body_len = strlen(body_content);
    
    char *header_buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
    build_http_header(header_buffer, response_code, mime_type, body_len);
    size_t header_len = strlen(header_buffer);

    // copy header and body
    memcpy(response_buffer, header_buffer, header_len);
    // offset by header_len bytes
    memcpy(response_buffer + header_len, body_content, body_len);

    *response_len = header_len + body_len;
    free(header_buffer);
    free(body_content);
}

void* handle_client(void* args){
    // cast to int, and get address of the args pointer
    int client = *((int *)args);
    free(args);

    char *request_buffer = malloc(BUFFER_SIZE * sizeof(char));

    ssize_t bytes_received = recv(client, request_buffer, BUFFER_SIZE, 0);

    if(bytes_received < 0){
        perror("recv failed.\n");
    } else if(bytes_received == 0){
        perror("no bytes received\n");
    }
    else{
        // create regex to filter get and post requests
        regmatch_t matches[2];

        printf("Received %zd bytes from client %d:\n---\n%s\n---\n", bytes_received, client, request_buffer);
        
        char *response_buffer = (char *) malloc(BUFFER_SIZE * 2 * sizeof(char));
        size_t response_len;
        
        if(regexec(&regex_GET_ROOT, request_buffer, 2, matches, 0) == 0){
        
            build_simple_http_response(200,"hello sugarplum! ^_^", response_buffer, &response_len);        
            send(client, response_buffer, response_len, 0);            
        
        } else if (regexec(&regex_GET_FILE, request_buffer, 2, matches, 0) == 0){
            // filename shouldnt be more than 100 chars else segfault XD
            // also this definitely has a critical vulnerability cuz im not sanitizing the file name tho
            char *file_name = (char *)malloc(100 * sizeof(char));
            strncpy(file_name, request_buffer + matches[1].rm_so, (size_t) matches[1].rm_eo - matches[1].rm_so);
            
            // read file 
            int file_fd = open(file_name, O_RDONLY);
            if (file_fd == -1){
                build_simple_http_response(404, "File not found on server", response_buffer, &response_len);
                send(client, response_buffer, response_len, 0);
            } else {
                
                char *file_contents = malloc(BUFFER_SIZE * sizeof(char));
                ssize_t bytes_read = read(file_fd, file_contents, BUFFER_SIZE);
                if(bytes_read < 0){
                    printf("error reading file");
                    build_simple_http_response(500, "", response_buffer, &response_len);
                    send(client, response_buffer, response_len, 0);
                }else{
                    build_simple_http_response(200, file_contents, response_buffer, &response_len);
                    send(client, response_buffer, response_len, 0);
                }
                free(file_contents);
                close(file_fd);
            }
            free(file_name);

        } else if(regexec(&regex_POST_ROOT, request_buffer, 2, matches, 0) == 0){
            build_simple_http_response(500,"POST not implemented yet", response_buffer, &response_len);
            send(client, response_buffer, response_len, 0);
        } else{            
            build_simple_http_response(500,"Route not implemented", response_buffer, &response_len);
            send(client, response_buffer, response_len, 0);
        }
        free(response_buffer);   
    }
    close(client);
    free(request_buffer);
    return NULL;
}

int main(){

    // create the socket with ipv4(AF_INET) domain and tcp(SOCK_STREAM) protocol 
    server = socket(AF_INET, SOCK_STREAM, 0);
    // returns -1 if socket creation fails 
    if(server < 0){
        perror("socket couldnt be created");
        exit(EXIT_FAILURE);
    }

    // to reuse addresses
    int opt = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR failed");
    }

    // create the server specifications: port, domain 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // bind the socket to server settings (port and domain)
    if(bind(
        server,
        (struct sockaddr *)&server_addr, 
        sizeof(server_addr)) < 0){
            perror("binding failed");
            exit(EXIT_FAILURE);
    }
    
    // start listening on the port, 10 means max connections in a queue
    if(listen(server, 10) < 0){
        perror("error while listening on " + PORT);
        exit(EXIT_FAILURE);
    }   
    printf("Server Listening on %d\n", PORT);
    
    // for routes
    compile_regexs();

    while(1){
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client = malloc(sizeof(int));

        *client = accept(server, ((struct sockaddr*)&client_addr), &client_addr_len);
        if(*client < 0){
            perror("Client connection failed");
            continue;
        }
        // create a new thread to handle a new client multiple clients->multiple threads 
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)client);
        pthread_detach(thread_id);
    }
    free_regexs();
    close(server);
    return 0;
}