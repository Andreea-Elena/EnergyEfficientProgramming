// socket server, handles multiple clients using threads

#include <arpa/inet.h>  //inet_addr
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>  //for threading , link with lpthread
#include <stdio.h>
#include <stdlib.h>  //strlen
#include <string.h>  //strlen
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/time.h>  //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <unistd.h>  //write

#include "cJSON.h"
#include "headers/ht.h"
#include "headers/http_header.h"
#include "headers/mysql_utils.h"
#include "headers/run_lua.h"
#include "headers/str_utils.h"

#define PORT 8888
#define MAX_WRITE_SIZE \
  16384                      // We limit the sending buffer to make sure we send all data in one go - also
                             // depends on the client
#define CLIENT_SIZE 32       // Limit of concurrent connections
#define MAX_CHUNK_SIZE 1024  // Define the maximum size of each chunk

struct MESSAGE_THREAD_ARGUMENT {
  int sd;    // The Socket attached to the client connection
  int port;  // Client port
  char *ip;  // Client IP
};

// the thread function
void *thread(void *);

int main(int argc, char *argv[]) {
  print_mysql_version();

  int socket_desc, client_sock, c;
  struct sockaddr_in server, client;

  // Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1) {
    printf("Could not create socket\n");
  }
  printf("Socket created\n");

  // Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORT);

  // Bind
  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
    // print the error message
    printf("ERROR bind failed\n");
    return 1;
  }
  printf("bind done\n");

  // Listen
  listen(socket_desc, CLIENT_SIZE);

  // Accept and incoming connection
  printf("Waiting for incoming connections...\n");
  c = sizeof(struct sockaddr_in);

  while (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)) {
    printf("Connection accepted\n");

    pthread_t sniffer_thread;

    struct MESSAGE_THREAD_ARGUMENT *msgArg = malloc(sizeof(struct MESSAGE_THREAD_ARGUMENT));
    msgArg->sd = client_sock;
    msgArg->port = ntohs(client.sin_port);
    int add_len = strlen(inet_ntoa(client.sin_addr)) + 1;
    char *client_ip = malloc(add_len * sizeof(char));
    strncpy(client_ip, inet_ntoa(client.sin_addr), add_len);
    client_ip[add_len] = '\0';
    msgArg->ip = client_ip;

    time_t t;
    time(&t);
    printf("%s - MAIN::New connection, socket fd is %d , ip is : %s , port : %d\n", strtok(ctime(&t), "\n"), client_sock, inet_ntoa(client.sin_addr),
           ntohs(client.sin_port));
    if (pthread_create(&sniffer_thread, NULL, thread, msgArg) < 0) {
      printf("ERROR could not create thread\n");
      return 1;
    }

    printf("Handler assigned\n");
  }

  if (client_sock < 0) {
    printf("ERROR accept failed\n");
    return 1;
  }
  return 0;
}

char *analyze_method(char message[]) {
  http_header *hh_h = calloc(1, sizeof(http_header));
  int init_ok_h = init_header(message, hh_h);
  char *lua = "lua";
  lua_file_signature *lfs = get_lua_file_signature(hh_h, lua);
  char *response = capilua(lfs);
  printf("\n\nEXIT \n$$$$$$$$$$$$$$ [%d] [%s] $$$$$$$$$$$$$$$$\n\n", init_ok_h, response);
  free(hh_h);
  free(lfs);
  return response;
}

/*
  This will handle connection for each client
  */
void *thread(void *socket_desc) {
  struct MESSAGE_THREAD_ARGUMENT *pMsg = (struct MESSAGE_THREAD_ARGUMENT *)socket_desc;
  int sd = pMsg->sd;
  int port = pMsg->port;
  char *client_ip = strdup(pMsg->ip);
  int n;
  pid_t thread_id = syscall(__NR_gettid);
  char client_message[MAX_WRITE_SIZE];

  while ((n = recv(sd, client_message, MAX_WRITE_SIZE, 0)) > 0) {
    // call analyze_method on client_message
    char *result = analyze_method(client_message);
    send(sd, result, strlen(result), 0);
    close(sd);
  }

  free(pMsg->ip);
  free(pMsg);
  return 0;
}

int is_json(const char *string) {
  cJSON *json = cJSON_Parse(string);
  if (json == NULL) {
    // cJSON_Parse returns NULL if the string is not valid JSON
    return 0;
  }
  cJSON_Delete(json);
  return 1;
}
