#include <arpa/inet.h>  //close
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //strlen
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/time.h>  //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <unistd.h>  //close
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define PORT 8888

#define BUFFER_CHUNK_SIZE 4096
#define BUFFER_MAX_HEADER_SIZE 4096
#define CLIENT_SIZE 64          // Limit of concurrent connections
#define MAX_MESSAGE_SIZE 16384  // We limit the incoming message size as we do not yet support multi-messages
#define MAX_WRITE_SIZE 16384    // We limit the sending buffer to make sure we send all data in one go - also depends on the client

/**
 * @brief Structure to pass the data between the threads and the fd_write select function.
 *
 */
struct MESSAGE_THREAD_RET_QUEUE {
  int thread_id;      // Current thread id
  int ret;            // Current return value
  int sd;             // Socket Descriptor used
  int finished_work;  // Thread has finished ALL work = 0 , otherwise set to -1
  int message_size;   // Size of the whole message
  int port;           // Client port
  char *ip;           // Client IP

  char *write_buffer;  // If thread finished all work then it may want to respond

} _ret_queue[CLIENT_SIZE];

/**
 * @brief Structure to pass the arguments to the thread.
 *
 */
struct MESSAGE_THREAD_ARGUMENT {
  char *msg;  // Message received from the client
  int sd;     // The Socket attached to the client connection
  int slot;   // Index of the MESSAGE_THREAD_RET_QUEUE array
  int port;   // Client port
  char *ip;   // Client IP
};

/**
 * @brief Message analysis thread.
 *
 * @param arg_struct is of type MESSAGE_THREAD_ARGUMENT
 * @return void*
 */
void *thread(void *arg_struct) {
  char *msg = ((struct MESSAGE_THREAD_ARGUMENT *)arg_struct)->msg;
  int sd = ((struct MESSAGE_THREAD_ARGUMENT *)arg_struct)->sd;
  int slot = ((struct MESSAGE_THREAD_ARGUMENT *)arg_struct)->slot;
  int port = ((struct MESSAGE_THREAD_ARGUMENT *)arg_struct)->port;
  char *client_ip = ((struct MESSAGE_THREAD_ARGUMENT *)arg_struct)->ip;

  pid_t thread_id = syscall(__NR_gettid);

  _ret_queue[slot].finished_work = -1;
  _ret_queue[slot].thread_id = thread_id;
  _ret_queue[slot].sd = sd;

  int message_length = strlen(msg);
  // printf("THREAD::.thread started with ID %d\n", thread_id);
  // printf("THREAD::.thread entered with argument message lenght of %d, with socket %d, on slot %d\n%s\n", message_length, sd, slot, msg);
  int ret = 1;

  /*
   * DO COMPUTATIONS HERE
   *
   */
  /*TEST ONLY STRAT LOAD FUNCTION*/

  struct timeval tv_begin, tv_current;
  gettimeofday(&tv_begin, NULL);

  srand(time(0));
  int upper = 150, lower = 50;
  int num = (rand() % (upper - lower + 1)) + lower;

  long long TIMEOUT = num * 10000u;
  for (;;) {
    gettimeofday(&tv_current, NULL);
    unsigned long long diff = (tv_current.tv_sec * 1000000 + tv_current.tv_usec) - (tv_begin.tv_sec * 1000000 + tv_begin.tv_usec);

    if (diff > TIMEOUT)
      break;
  }
  /*TEST ONLY END LOAD FUNCTION*/

  char end_message[MAX_WRITE_SIZE];
  sprintf(end_message, "Write data for thread ID %d on socket %d to ip %s and port %d.", thread_id, sd, client_ip, port);
  // printf("THREAD::.thread wnats to write to socket \n.---\n%s\n.---\n", end_message);
  // printf("THREAD::.thread exit with return value of %d\n", ret);

  _ret_queue[slot].ret = ret;
  _ret_queue[slot].write_buffer = (char *)malloc(MAX_WRITE_SIZE);
  memcpy(_ret_queue[slot].write_buffer, end_message, strlen(end_message));
  _ret_queue[slot].finished_work = 0;

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int opt = TRUE;
  int master_socket, addrlen, new_socket, max_clients = CLIENT_SIZE, activity, i, valread, sd;
  int client_socket[CLIENT_SIZE];
  int max_sd;
  struct sockaddr_in address;

  char r_buffer[CLIENT_SIZE][MAX_MESSAGE_SIZE];
  char w_buffer[CLIENT_SIZE][MAX_WRITE_SIZE];
  char **message;
  int message_chunks[CLIENT_SIZE];

  struct MESSAGE_THREAD_RET_QUEUE mtrq[CLIENT_SIZE];
  pthread_t process_threads[CLIENT_SIZE];

  fd_set readfds;
  fd_set writefds;
  fd_set except_set;

  for (i = 0; i < max_clients; i++) {
    client_socket[i] = 0;
    message = malloc(CLIENT_SIZE * sizeof(char *));
    message_chunks[i] = 0;
    message[i] = "\0";
  }

  // create a master socket
  if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    printf("Create socket failed.");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
    printf("Do setsockopt failed.");
    exit(EXIT_FAILURE);
  }

  // type of socket created
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // bind the socket to localhost port 8888
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    printf("Do bind failed.");
    exit(EXIT_FAILURE);
  }
  printf("MAIN::Listener on port %d \n", PORT);

  if (listen(master_socket, CLIENT_SIZE) < 0) {
    printf("Now we are listening.");
    exit(EXIT_FAILURE);
  }

  // accept the incoming connection
  addrlen = sizeof(address);
  printf("MAIN::Waiting for connections ...");
  time_t t;

  while (TRUE) {
    FD_ZERO(&readfds);
    FD_SET(master_socket, &readfds);

    FD_ZERO(&writefds);
    FD_SET(master_socket, &writefds);

    FD_ZERO(&except_set);
    FD_SET(master_socket, &except_set);

    max_sd = master_socket;

    // add child sockets to set
    for (i = 0; i < max_clients; i++) {
      sd = client_socket[i];

      if (sd > 0) {
        FD_SET(sd, &readfds);
        FD_SET(sd, &writefds);
        FD_SET(sd, &except_set);
      }

      if (sd > max_sd) {
        max_sd = sd;
      }
    }

    activity = select(max_sd + 1, &readfds, &writefds, &except_set, NULL);

    if ((activity < 0) && (errno != EINTR)) {
      printf("MAIN::select error:: %d -- %d -- %d\n", errno, max_sd, master_socket);
      // continue;
    }

    if (FD_ISSET(master_socket, &readfds)) {
      if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        // printf("accept");
        exit(EXIT_FAILURE);
      }

      time(&t);
      printf("%s - MAIN::New connection, socket fd is %d , ip is : %s , port : %d\n", strtok(ctime(&t), "\n"), new_socket,
             inet_ntoa(address.sin_addr), ntohs(address.sin_port));

      for (i = 0; i < max_clients; i++) {
        // if position is empty
        if (client_socket[i] == 0) {
          client_socket[i] = new_socket;
          // printf("MAIN::Adding to list of sockets as %d\n" , i);

          break;
        }
      }
    }

    for (i = 0; i < max_clients; i++) {
      sd = client_socket[i];

      if (FD_ISSET(sd, &readfds)) {
        valread = read(sd, r_buffer[i], MAX_MESSAGE_SIZE);
        // printf("MAIN::--read bytes: %d\n", valread);

        if (valread == 0) {
          // Somebody disconnected , get his details and print
          getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
          time(&t);
          printf("%s - MAIN::Host disconnected , ip %s , port %d \n", strtok(ctime(&t), "\n"), inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          close(sd);
          client_socket[i] = 0;
          _ret_queue[i].sd = 0;
          _ret_queue[i].thread_id = 0;
          _ret_queue[i].ret = 0;
          _ret_queue[i].finished_work = 0;
          memset(r_buffer[i], 0, MAX_MESSAGE_SIZE);
        } else if (valread < 0) {
          printf("MAIN:: READ ERRROR!! %d\n", valread);
        } else {
          // printf("MAIN::---->Entering first message for socket %d\n", sd);
          message[i] = malloc(valread * sizeof(char));
          strncpy(message[i], r_buffer[i], valread);

          struct MESSAGE_THREAD_ARGUMENT msgArg;
          msgArg.msg = message[i];
          msgArg.sd = sd;
          msgArg.slot = i;
          msgArg.port = ntohs(address.sin_port);
          int add_len = strlen(inet_ntoa(address.sin_addr)) + 1;
          char *client_ip = malloc(add_len * sizeof(char));
          strncpy(client_ip, inet_ntoa(address.sin_addr), add_len);
          client_ip[add_len] = '\0';
          msgArg.ip = client_ip;
          int thread_create = pthread_create(&process_threads[i], NULL, thread, &msgArg);

          // printf("MAIN::---->Started process message thread with status  %d and ID %lu\n", thread_create, process_threads[i]);
        }
      }

      // The socket is now writable - check to see if the thread finished processing and get the write buffer
      if (FD_ISSET(sd, &writefds)) {
        for (i = 0; i < max_clients; i++) {
          // check to see if any previous threads exited so we close and reuse the socket
          if (_ret_queue[i].sd > 0 && _ret_queue->finished_work == 0 && _ret_queue[i].write_buffer != NULL) {
            // printf("\nMAIN::On SLOT %d SOCKET %d has finished processing write buffer sais \n---\n%s\n---.\n", i, _ret_queue[i].sd,
            // _ret_queue[i].write_buffer); printf("\nMAIN::...TID[%d],RET[%d],SD[%d],FSH[%d]\n", _ret_queue[i].thread_id, _ret_queue[i].ret,
            // _ret_queue[i].sd,_ret_queue[i].finished_work);

            // Start writing the response
            int writen = write(_ret_queue[i].sd, _ret_queue[i].write_buffer, strlen(_ret_queue[i].write_buffer));
            time(&t);
            printf("%s - MAIN::Writen to socket %d - %d - %d\n", strtok(ctime(&t), "\n"), _ret_queue[i].thread_id, _ret_queue[i].sd, writen);

            // If write op has ended, end && clean socket for reuse && clean struct for reuse
            close(_ret_queue[i].sd);
            client_socket[i] = 0;

            _ret_queue[i].sd = 0;
            _ret_queue[i].thread_id = 0;
            _ret_queue[i].ret = 0;
            _ret_queue[i].finished_work = 0;
          }
        }
      }
    }
  }
}