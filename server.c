/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
//added by me
#include <pthread.h>
extern void* get_stats(void*);
extern double percent_usage;
extern double running_avg;
extern double max_usage;
extern void* get_input(void*);
extern int flag;
int fd;
int sock;
char* reqCopy;
pthread_mutex_t lock_max;
pthread_mutex_t lock_last;
pthread_mutex_t lock_avg;
pthread_mutex_t lock_flag;
pthread_mutex_t lock_print;
pthread_mutex_t lock_fd;
char* jsText;




void readJSFile(){
    FILE* f = fopen("autoupdate.js", "r");
//               memset(fullJS, 0, sizeof(char)*3000);
              char* fullJS = malloc(sizeof(char)*3000);
              char* line = malloc(sizeof(char)*50);
              while(fgets(line, 50, f)!=NULL){
                  strcat(fullJS, line);
              }
//               printf("js: %s",fullJS);
              fclose(f);
    jsText = malloc(sizeof(char)*3000);
    strcpy(jsText, fullJS);
    
        free(fullJS);
    free(line);
}

int start_server(int PORT_NUMBER)
{

      // structs to represent the server and client
      struct sockaddr_in server_addr,client_addr;    
      
//       int sock; // socket descriptor
        int sock;
    
      // 1. socket: creates a socket descriptor that you later use to make other system calls
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Socket");
	exit(1);
      }
      int temp;
      if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
	perror("Setsockopt");
	exit(1);
      }

      // configure the server
      server_addr.sin_port = htons(PORT_NUMBER); // specify port number
      server_addr.sin_family = AF_INET;         
      server_addr.sin_addr.s_addr = INADDR_ANY; 
      bzero(&(server_addr.sin_zero),8); 
      
      // 2. bind: use the socket and associate it with the port number
      if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
	perror("Unable to bind");
	exit(1);
      }

      // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
      if (listen(sock, 1) == -1) {
	perror("Listen");
	exit(1);
      }
          
      // once you get here, the server is set up and about to start listening
      printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
      fflush(stdout);
     
    int count = 0; // count the number of pages requested (for debugging purposes)
    
    while(1) { // keep looping and accept additional incoming connections
      
       pthread_mutex_lock(&lock_flag);
        if(flag==1){
            pthread_mutex_unlock(&lock_flag);
//             printf("step1\n");
            break;
        }
        pthread_mutex_unlock(&lock_flag);
        
        // 4. accept: wait here until we get a connection on that port
        int sin_size = sizeof(struct sockaddr_in);
//       int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      
        pthread_mutex_lock(&lock_flag);
        if(flag==1){
            pthread_mutex_unlock(&lock_flag);
//             printf("step2\n");
            break;
        }
        pthread_mutex_unlock(&lock_flag);
        
        if (fd != -1) {
         printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        // buffer to read data into
        char request[1024];

        // 5. recv: read incoming message (request) into buffer
        int bytes_received = recv(fd,request,1024,0);
        // null-terminate the string
        request[bytes_received] = '\0';
        // print it to standard out
        printf("REQUEST:\n%s\n", request);

        count++; // increment counter for debugging purposes

        // this is the message that we'll send back
        // UPDATED BY ME
//         char* response = (char*)malloc(100 * sizeof(char));
//         sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><p>It works!<br>count=%d</p></html>", count);


          
           char* response = (char*)malloc(8000 * sizeof(char));
          

          
          
          //get the second token
          reqCopy = malloc(sizeof(char)*1024);
          strcpy(reqCopy, request);
          strtok(reqCopy, " ");
          char* typeOfRequest;
          typeOfRequest = strtok(NULL, " ");
          while(strtok(NULL, " ")!=NULL){
              strtok(NULL, " ");
          }
          
          //if its /data, send back json
          if(strcmp(typeOfRequest, "/data.json")==0){
               pthread_mutex_lock(&lock_last);
               pthread_mutex_lock(&lock_max);
               pthread_mutex_lock(&lock_avg);
              
              sprintf(response, "HTTP/1.1 200 OK\nContent-Type: application/json\n\n{ \"last\" : %f, \"max\" : %f, \"avg\" : %f }", percent_usage, max_usage, running_avg);
              
              pthread_mutex_unlock(&lock_last);
              pthread_mutex_unlock(&lock_max);
              pthread_mutex_unlock(&lock_avg);
          }
          
          //else, send the whole html
          else{   
              

              pthread_mutex_lock(&lock_last);
               pthread_mutex_lock(&lock_max);
               pthread_mutex_lock(&lock_avg);
              
//               sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><head><script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script><style>button {background-color: gray; }</style></head><body><button type=\"submit\" id=\"thebutton\">Auto-update</button><p id=\"last\">last: %f</p><p id=\"max\">max: %f</p><p id=\"avg\">average: %f</p><script type=\"text/javascript\">%s </script></body></html>",percent_usage, max_usage, running_avg, jsText);
              sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><head><script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script><style>button {background-color: gray; }</style></head><body><button type=\"submit\" id=\"thebutton\">Auto-update</button><p>last reading: <span id=\"last\">%f</span></p><p>max usage: <span id=\"max\"> %f</span></p><p>average usage: <span id=\"avg\">%f</span></p><script type=\"text/javascript\">%s </script></body></html>",percent_usage, max_usage, running_avg, jsText);
//                              sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><head><script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script><style>button {background-color: gray; }</style></head><body><button type=\"submit\" id=\"thebutton\">Auto-update</button><p id=\"last\">last: %f</p><p id=\"max\">max: %f</p><p id=\"avg\">average: %f</p><script type=\"text/javascript\"> </script></body></html>",percent_usage, max_usage, running_avg);

               pthread_mutex_unlock(&lock_last);
              pthread_mutex_unlock(&lock_max);
              pthread_mutex_unlock(&lock_avg);
              

          }

        
  
          
        printf("RESPONSE:\n%s\n", response);

        // 6. send: send the outgoing message (response) over the socket
        // note that the second argument is a char*, and the third is the number of chars	
        send(fd, response, strlen(response), 0);

//             printf("res\n");
          free(response);
//             printf("reqcopy\n");
          free(reqCopy);
          


        // 7. close: close the connection
        printf("close\n");
            close(fd);
        printf("Server closed connection\n");
       }
    }

    // 8. close: close the socket
//     printf("sock\n");
    close(sock);
    printf("Server shutting down\n");
  
    return 0;
} 



int main(int argc, char *argv[])
{
    /*
  // check the number of arguments
  if (argc != 2) {
      printf("\nUsage: %s [port_number]\n", argv[0]);
      exit(-1);
  }

  int port_number = atoi(argv[1]);
  if (port_number <= 1024) {
    printf("\nPlease specify a port number greater than 1024\n");
    exit(-1);
  }*/
    
    int port_number = 3000; // hard-coded for use on Codio

    pthread_mutex_init(&lock_max, NULL);
    pthread_mutex_init(&lock_last, NULL);
    pthread_mutex_init(&lock_avg, NULL);
    pthread_mutex_init(&lock_print, NULL);
    pthread_mutex_init(&lock_flag, NULL);
    pthread_mutex_init(&lock_fd, NULL);
    
    readJSFile();
    
  pthread_t t1;  
  int ret1 = pthread_create(&t1, NULL, &get_stats, NULL);
    if(ret1!=0){
        return 1;
    }  
    
  pthread_t t2;  
  int ret2 = pthread_create(&t2, NULL, &get_input, NULL);
    if(ret2!=0){
        return 1;
    }    
  start_server(port_number);
  
//   printf("done0\n");  
    
  void* r;
  void* r2;
     pthread_join(t1, &r);
//     printf("done1\n");
  pthread_join(t2, &r2);
//     printf("done\n");
 
    free(jsText);
    
    return 0;
  
}



