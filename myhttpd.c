#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
int queueLength = 1;

//declare all the functions here so that I can put them in random order later in the file
void processRequest(int socket);
int checkRequest(char* request, char** path);
void print404Header(int socket);
void printHeader(int socket, char* doc_type, char* other_info);
void printFileData (int socket, FILE* fp);
char* getFileType(char* extension);
int handleAuthorization(char* request, int socket, char** otherInfo);

//declare all the functions for the different concurrency types
void iterativeServer(int port);
void newProcessAfterRequest(int port);
void newThreadForRequest(int port);
void poolOfThreads(int port);
void* startSlaveSocket(void* masterSocket);

//main function
int main( int argc, char **argv) {
  //parse the command line arguments
  //default port is 1451 and the default mode is iterative
  int port = 1451;
  char mode = 0;
  if (argc == 3) {
    //if there are 3 arguments, then there is a part and a mode
    port = atoi(argv[2]);
    mode = argv[1][1];
  } else if (argc == 2) {
    //if there are two, then check if port or mode is specified
    if (strchr(argv[1], '-') != NULL) {
      mode = argv[1][1];
    } else {
      port = atoi(argv[1]);
    }
  }

  //take the mode and call the right method
  switch(mode) {
    case '\0':
      iterativeServer(port);
      break;
    case 'f':
      newProcessAfterRequest(port);
      break;
    case 't':
      newThreadForRequest(port);
      break;
    case 'p':
      poolOfThreads(port);
      break;
    default:
      printf("Wrong choice, enter a valid choice\n");
      break;
  }

}

//iterative server - one thread and one process
void iterativeServer(int port) {
  //declare and create the serverIPAddress like we did in class
  struct sockaddr_in serverIPAddress;
  memset(&serverIPAddress, 0, sizeof(serverIPAddress));
  serverIPAddress.sin_family = AF_INET;
  serverIPAddress.sin_addr.s_addr = INADDR_ANY;
  serverIPAddress.sin_port = htons((u_short) port);

  //create the master socket, this is for the server
  int masterSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (masterSocket < 0) { //error check
    perror("socket");
    exit(-1);
  }

  //call setsockopt so that the port (? i think?) can be reused
  int optval = 1;
  int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,
      (char *) &optval, sizeof(int));

  //bind the master socket and the serverIPAddress
  int error = bind(masterSocket, (struct sockaddr *)&serverIPAddress, sizeof(serverIPAddress));
  if (error) { //error check
    perror("bind");
    exit(-1);
  }

  //have the master socket look out for a client
  error = listen(masterSocket, queueLength);
  if (error) { //error check
    perror("listen");
    exit(-1);
  }

  //while true
  while (1) {
    //create the clientIPAddress
    struct sockaddr_in clientIPAddress;
    int alen = sizeof(clientIPAddress);

    //wait for a client to connect
    int slaveSocket = accept(masterSocket, (struct sockaddr*)&clientIPAddress, (socklen_t*)&alen);
    if (slaveSocket < 0) { //error check
      perror("accept");
      exit(-1);
    }

    //process the request (passing in the slave socket)
    processRequest(slaveSocket);
    //CLOSE the slave socket
    close(slaveSocket);
  }

}

//new process after each request (service the request in another process)
void newProcessAfterRequest(int port) {
  //same as before, create and declare the serverIPAddress
  struct sockaddr_in serverIPAddress;
  memset(&serverIPAddress, 0, sizeof(serverIPAddress));
  serverIPAddress.sin_family = AF_INET;
  serverIPAddress.sin_addr.s_addr = INADDR_ANY;
  serverIPAddress.sin_port = htons((u_short) port);

  //create the master socket
  int masterSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (masterSocket < 0) { //error check
    perror("socket");
    exit(-1);
  }

  //make sure the port is reuseable
  int optval = 1;
  int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,
      (char *) &optval, sizeof(int));

  //bind the master socket and the server ip address
  int error = bind(masterSocket, (struct sockaddr *)&serverIPAddress, sizeof(serverIPAddress));
  if (error) { //error check
    perror("bind");
    exit(-1);
  }

  //make sure that the master socket is listening
  error = listen(masterSocket, queueLength);
  if (error) { //error check
    perror("listen");
    exit(-1);
  }

  //while true
  while (1) {
    //create a clientIPAddress
    struct sockaddr_in clientIPAddress;
    int alen = sizeof(clientIPAddress);

    //wait for a client to connect
    int slaveSocket = accept(masterSocket, (struct sockaddr*)&clientIPAddress, (socklen_t*)&alen);
    if (slaveSocket < 0) { //error check
      perror("accept");
      continue;
    }

    //create a child process
    int ret = fork();
    if (ret == 0) {
      //if child, then process the request
      processRequest(slaveSocket);
      exit(0);
    }

    //CLOSE the slave socket
    close(slaveSocket);
    while (waitpid(ret, NULL, 0) > 0);
  }


}

//wrapper method to call the process function
void* callProcessFunction(int socket) {
  //in which we process the request
  processRequest(socket);
  //close the socket here only
  close(socket);
}

//create a new thread for each request
void newThreadForRequest(int port) {
  //create the serverIPAddress
  struct sockaddr_in serverIPAddress;
  memset(&serverIPAddress, 0, sizeof(serverIPAddress));
  serverIPAddress.sin_family = AF_INET;
  serverIPAddress.sin_addr.s_addr = INADDR_ANY;
  serverIPAddress.sin_port = htons((u_short) port);

  //create the master socket
  int masterSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (masterSocket < 0) { //error check
    perror("socket");
    exit(-1);
  }

  //make sure the port is reusable
  int optval = 1;
  int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,
      (char *) &optval, sizeof(int));

  //bind the master socket and the serverIPAddress
  int error = bind(masterSocket, (struct sockaddr *)&serverIPAddress, sizeof(serverIPAddress));
  if (error) { //error check
    perror("bind");
    exit(-1);
  }

  //put the masterSocket in listening mode
  error = listen(masterSocket, queueLength);
  if (error) { //error check
    perror("listen");
    exit(-1);
  }

  //while true
  while (1) {
    //create a clientIPAddress
    struct sockaddr_in clientIPAddress;
    int alen = sizeof(clientIPAddress);

    //listen for a client connection
    int slaveSocket = accept(masterSocket, (struct sockaddr*)&clientIPAddress, (socklen_t*)&alen);
    if (slaveSocket < 0) { //error check
      perror("accept");
      continue;
    }

    //create a thread and call the wrapper method
    //make sure to pass the slaveSocket
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_create(&thread, &attr, (void* (*)(void *))callProcessFunction, (void*)slaveSocket);
  }
}

//create a pool of threads
void poolOfThreads(int port) {
  //create the serverIPAddress
  struct sockaddr_in serverIPAddress;
  memset(&serverIPAddress, 0, sizeof(serverIPAddress));
  serverIPAddress.sin_family = AF_INET;
  serverIPAddress.sin_addr.s_addr = INADDR_ANY;
  serverIPAddress.sin_port = htons((u_short) port);

  //create the master socket
  int masterSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (masterSocket < 0) { //error check
    perror("socket");
    exit(-1);
  }

  //make sure the port is reusable
  int optval = 1;
  int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,
      (char *) &optval, sizeof(int));

  //bind the masterSocket and the serverIPAddress
  int error = bind(masterSocket, (struct sockaddr *)&serverIPAddress, sizeof(serverIPAddress));
  if (error) { //error check
    perror("bind");
    exit(-1);
  }

  //put the masterSocket in listening mode
  error = listen(masterSocket, queueLength);
  if (error) { //error check
    perror("listen");
    exit(-1);
  }

  //make and array of 4 threads
  pthread_t threads[5];

  //create all the threads and call the wrapper function to start the slave socket
  for (int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, startSlaveSocket, (void*)masterSocket);
  }

  //call the wrapper function for the 5th and main thread (same)
  startSlaveSocket((void*)masterSocket);

}

//wrapper function for starting the slave socket
void* startSlaveSocket(void* masterS) {
  //cast the master socket
  int masterSocket = *(int *)&masterS;

  //while true
  while (1) {
    //create the clientIPAddress
    struct sockaddr_in clientIPAddress;
    int alen = sizeof(clientIPAddress);

    //listen for a client connection
    int slaveSocket = accept(masterSocket, (struct sockaddr*)&clientIPAddress, (socklen_t*)&alen);
    if (slaveSocket < 0) { //error check
      perror("accept");
      exit(-1);
    }

    //process that request
    processRequest(slaveSocket);
    //close the slave socket
    close(slaveSocket);
  }
}

//process the request
void processRequest(int socket) {
  //max length of a request is 1024
  const int maxLength = 1024;
  char request [maxLength + 1];
  //clear all the values in request[] just in case
  for (int i = 0; i < maxLength; i++) { request[i] = '\0'; }
  int curr = 0;
  int n;

  //keep track of the current character and the last character
  unsigned char newChar;
  unsigned char lastChar = 0;

  //flag to see if there was a newline was there already
  int sawNextLine = 0;

  //read until \r\n\r\n
  while (curr < maxLength && (n = read(socket, &newChar, sizeof(newChar))) > 0) {
    //if a \r\n is detected
    if (newChar == '\n' && lastChar == '\r') {
      //if we already saw a new line right before
      if (sawNextLine == 1) {
        //break out of the loop
        request[curr] = '\0';
        break;
      } else {
        //set flag to true and keep looking for another
        sawNextLine = 1;
        request[curr] = newChar;
        curr++;
      }
    } else {
      //if not, save the character
      request[curr] = newChar;
      curr++;

      //make sure that \r\n text \r\n isn't detected as the end
      if (newChar != '\r') {
        sawNextLine = 0;
      }
    }

    //update the last character
    lastChar = newChar;
  }

  //put the request in a char* to make it easier for me :)
  char* req = (char*)malloc(curr);
  strncpy(req, request, curr);
  req[curr] = '\0'; //null-terminate the req

  //create a double pointer to pass to another method
  char** cwd = (char**)malloc(sizeof(char**));
  if (checkRequest(req, cwd) == 1) {
    //there was an error
    print404Header(socket);
    return;
  }

  //create a double pointer to pass to another method
  char** otherInfo = (char**)malloc(sizeof(char**));
  //check if there is authorization done
  if (handleAuthorization(req, socket, otherInfo) == 1) {
    return;
  }

  //open the file that is specified
  FILE* fp = fopen(*cwd, "rb");
  //get the extension
  char* extension = strrchr(*cwd, '.');
  //print the header
  printHeader(socket, getFileType(extension), *otherInfo);
  //print the file data
  printFileData(socket, fp);
  //CLOSE the file
  fclose(fp);

}

//handle the authorization
int handleAuthorization(char* request, int socket, char** otherInfo) {
  //check if its in the correct format
  char* auth_str = (char*)"Authorization: Basic ";
  char* auth = strstr(request, auth_str);
  //if not, then print an error to the client
  if (auth == NULL) {
    char* a = (char*)"HTTP/1.1 401 Unauthorized\n";
    char* b = (char*)"WWW-Authenticate: Basic realm=\"The_Never_Sea_oh_wait_wrong_realm\"\n";
    printf("Error: not authorized, please try again\n");
    //write to the client
    write(socket, a, strlen(a));
    write(socket, b, strlen(b));
    //return 1 - that means an error occurred
    return 1;
  }

  //skip past to the password
  char* actualUsrpass = auth + strlen(auth_str);

  //get the other information
  *otherInfo = strstr(actualUsrpass, (char*)"\r\n");
  //skip past the crlf
  *otherInfo += 2;
  //cut off the \r\n\r\n at the end
  char* crlf = strchr(*otherInfo, '\r');
  int pos = crlf - *otherInfo;
  //substring it out
  strncpy(*otherInfo, *otherInfo, pos);
  (*otherInfo)[pos] = '\0'; //null-terminate it

  //cut off the password not
  crlf = strchr(auth, '\r');
  pos = crlf - actualUsrpass;
  strncpy(actualUsrpass, actualUsrpass, pos);
  actualUsrpass[pos] = '\0'; //null-terminate it

  //expected username and password
  char* expUsrpass = (char*)"Z2RiOmxldGl0dGFsa3RveW91";
  //if they are not equal, then return an error to the client
  if(strcmp(actualUsrpass, expUsrpass) != 0) {
    char* a = (char*)"HTTP/1.1 401 Unauthorized\n";
    char* b = (char*)"WWW-Authenticate: Basic realm=\"The_Never_Sea_oh_wait_wrong_realm\"\n";
    printf("Error: not authorized, please try again\n");
    //write to the client
    write(socket, a, strlen(a));
    write(socket, b, strlen(b));
    //return 1 - that means an error occurred
    return 1;
  }

  //return 0 - that means there was no error
  return 0;

}

//from the extension, get the file tyep
char* getFileType(char* extension) {
  //make sure that extension is not null
  //if .html then return text/html
  if (!extension || strcmp(extension, (char*)".html") == 0) {
    return (char*)"text/html";
  } else if (!extension || strcmp(extension, (char*)".gif") == 0) {
    //if .gif then return image/gif
    return (char*)"image/gif";
  } else {
    //otherwise just return text/plain
    return (char*)"text/plain";
  }
}

int checkRequest(char* req, char** path) {
  //make sure its all in the correct format
  char *get = strstr(req, (char*)"GET");
  if (get == NULL) {
    return 1;
  }

  if (strstr(req, (char*)"..")) { return 1; }

  //get the document requested
  char *tmp = strchr(req, ' ');
  if (tmp == NULL) { return 1; }
  tmp++;
  char *substr = strchr(tmp, ' ');
  if (substr == NULL) { return 1; }
  int pos = substr - tmp;

  char* doc = (char*)malloc(pos + 1);
  strncpy(doc, tmp, pos);
  //doc is the name of the document requested
  doc[pos] = '\0';

  //get the whole path of the document
  //get the path to the current directory
  char cwd[256];
  for (int i = 0; i < 256; i++) { cwd[i] = '\0'; }
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    return 1;
  }
  //add the /http-root-dir/ to it
  strcat(cwd, (char*)"/http-root-dir");

  //find the first part of it
  char* iconsCheck = strstr(doc, (char*)"/icons");
  char* htdocsCheck = strstr(doc, (char*)"/htdocs");
  char* cgibinCheck = strstr(doc, (char*)"/cgi-bin");
  char* cgisrcCheck = strstr(doc, (char*)"/cgi-src");

  //if it starts with /icons or /htdocs then add that
  if (iconsCheck != NULL || htdocsCheck != NULL || cgibinCheck != NULL || cgisrcCheck != NULL) {
    strcat(cwd, doc);
  } else if (strcmp(doc, (char*)"/") == 0) {
    //if it is just /, then add the below path
    strcat(cwd, (char*)"/htdocs/index.html");
  } else {
    //else ad htdocs and the path
    strcat(cwd, (char*)"/htdocs");
    strcat(cwd, doc);
  }

  //try to open if
  FILE* fp = fopen(cwd, "r");
  if (fp == NULL) {
    //if it doesnt exist, then return
    return 1;
  }
  fclose(fp); //CLOSE the fp

  //save the path so that the other method can get it later
  *path = (char*)malloc(strlen(cwd) + 1);
  strncpy(*path, cwd, strlen(cwd));
  (*path)[strlen(cwd)] = '\0'; //null-terminate it

  //no error
  return 0;

}

//when there is no file found
void print404Header(int socket) {
  //taken from the handout
  char* a = (char*)"HTTP/1.1 404 File Not Found\r\n";
  char* b = (char*)"Server: CS 252 lab5\r\n";
  char* c = (char*)"Content-type: text/plain\r\n";
  char* d = (char*)"\r\n";
  char* e = (char*)"Could not find the specified URL. The server returned an error.\r\n";

  //write it to the socket
  write(socket, a, strlen(a));
  write(socket, b, strlen(b));
  write(socket, c, strlen(c));
  write(socket, d, strlen(d));
  write(socket, e, strlen(e));
}

//print the header for a focument
void printHeader(int socket, char* doc_type, char* other_info) {
  //taken from the handout
  char* a = (char*)"HTTP/1.0 200 Document follows\r\n";
  char* b = (char*)"Server CS 252 lab5\r\n";
  char* c = (char*)"Content-type ";
  char* crlf = (char*)"\r\n";

  //write each part
  write(socket, a, strlen(a));
  write(socket, b, strlen(b));
  write(socket, c, strlen(c));
  write(socket, doc_type, strlen(doc_type));
  write(socket, crlf, strlen(crlf));
  //other_info can be null, so if it is then dont print it
  if (other_info != NULL) {
    write(socket, other_info, strlen(other_info));
    write(socket, crlf, strlen(crlf));
  }
  //print a new line
  write(socket, crlf, strlen(crlf));
  return;
}

//print the contents of a file
void printFileData(int socket, FILE* fp) {
  //get the length of the file by using fseek
  //240 wibes lol
  fseek(fp, 0, SEEK_END);
  long fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  //read the contents of the whole thing in a buffer
  char* contents = (char*)malloc(fileSize + 1);
  fread(contents, 1, fileSize, fp);
  contents[fileSize] = '\0'; //null-terminate it

  //write to the client connection
  write(socket, contents, fileSize);

}
