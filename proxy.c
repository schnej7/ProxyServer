/* JERRY SCHNEIDER */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/select.h>
#include "string_manip.h"
#include "networkIO.h"

//print the request made by the client to the terminal
void print_request( char * name, char * request, int filtered ){
  int requestLen = 0;
  char * end = findString( request, " HTTP" );
  while( &request[requestLen] != end ) requestLen++;
  char * output = (char *) malloc( requestLen + 2 );
  int i = 0;
  while( i < requestLen ){
    output[i] = request[i];
    i++;
  }
  output[i] = '\0';
  if(filtered) printf("%s: %s [FILTERED]\n", name, output);
  else printf("%s: %s\n", name, output);
}

//edit the request to contain only the needed information
int editRequest( char * request, char * type, char * host, char * contentLength, char ** returnRequest ){

  char * hostLine = "Host: ";
  char * lengLine = "Content-Length: ";
  char * newRequest = (char *) malloc( 1024 );
  int requestLen = 0;
  while( request[ requestLen ] != '\r' && request[ requestLen ] != '\n' ){
    newRequest[requestLen] = request[requestLen];
    requestLen++;
  }
  newRequest[requestLen++] = '\r';
  newRequest[requestLen++] = '\n';

  int i;
  for( i = 0; i < strlen(hostLine); i++ ){
    newRequest[requestLen++] = hostLine[i];
  }
  for( i = 0; i < strlen(host); i++ ){
    newRequest[requestLen++] = host[i];
  }
  newRequest[requestLen++] = '\r';
  newRequest[requestLen++] = '\n';

  //in case of post
  if( !strcmp( type, "POST" ) ){

    for( i = 0; i < strlen(lengLine); i++ ){
      newRequest[requestLen++] = lengLine[i];
    }
    for( i = 0; i < strlen(contentLength); i++ ){
      newRequest[requestLen++] = contentLength[i];
    }
    newRequest[requestLen++] = '\r';
    newRequest[requestLen++] = '\n';

  }

  newRequest[requestLen++] = '\r';
  newRequest[requestLen++] = '\n';
  newRequest[requestLen] = '\0';
  *returnRequest = newRequest;
  return requestLen;
}

//Get the host from the first line of the request
char * getHost( char * request ){

  int i = 1;
  while( (request[i] != '/' || request[i - 1] != '/') && request[i] != '\n' ) i++;
  if( request[i] == '\n' ) return NULL;
  i++;
  int len = 0;
  while( request[i + len] != '/' && request[i + len] != ' ' && request[i + len] != '\n') len++;
  if( request[i + len] == '\n' ) return NULL;
  char * host = (char *) malloc( len + 1 );
  int j;
  for( j = 0; j < len; j++ ){
    host[j] = request[i + j];
  }
  host[len] = '\0';
  return host;
}

//Connect to the server and send the request and get the response
int http_request( char * request, char ** buffer, char * host ){

  int length = 0;
  int sock = socket( PF_INET, SOCK_STREAM, 0 );

  if ( sock < 0 )
  {
    perror( "socket() failed" );
    return EXIT_FAILURE;
  }

  struct sockaddr_in server;
  struct hostent * hp;

  server.sin_family = PF_INET;
  hp = gethostbyname( host );
  if ( hp == NULL )
  {
    perror( "Unknown host" );
    char * response = construct400();
    *buffer = response;
    return strlen( response );
  }

  bcopy( (char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length );
  int port = 80;
  server.sin_port = htons( port );

  if ( connect( sock, (struct sockaddr *)&server, sizeof( server ) ) < 0 )
  {
    perror( "connect() failed" );
    return EXIT_FAILURE;
  }

  /* write a message out on the socket connection */
  int n = write( sock, request, strlen( request ) );
  //printf("request:\n%s\n", request);

  if ( n < strlen( request ) )
  {
    perror( "write() failed" );
    return EXIT_FAILURE;
  }

  //read head
  char * head;
  n = readHead( sock, &head );
  if ( n < 1 )
  {
    perror( "read() failed" );
    return EXIT_FAILURE;
  }
  else
  {
    //printf( "Rcvd message from server: %s\n", head );
  }

  char * content;
  //use chunked reading or not...
  char * contentLengthString = getTagValue( head, "Content-Length:");
  char * transferEncoding = getTagValue( head, "Transfer-Encoding:");
  if( transferEncoding != NULL && !strcmp( transferEncoding, "chunked" ) ){
    //printf("Chunked\n");
    length = readChunked( sock, &content );
  }
  else if( contentLengthString != NULL){
    int contentLength = atoi( contentLengthString );
    //printf("contentLength: %d\n", contentLength);
    length = readBytes( sock, &content, contentLength );
  }

  char * fullMessage = (char *) malloc( length + n );
  int i;
  for( i = 0; i < n; i++ ){
    fullMessage[i] = head[i];
  }
  for( i = 0; i < length; i++ ){
    fullMessage[i + n] = content[i];
  }

  //printf("content:\n%s\n", content);

  *buffer = fullMessage;

  close( sock );

  return length + n;
}

int main( int argc, char * argv[] ){

  int sock = socket( PF_INET, SOCK_STREAM, 0 );

  int optval = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  if ( sock < 0 ) {
    perror( "socket() failed" );
    return EXIT_FAILURE;
  }

  unsigned short port = 8123; /* port number to listen on */
  if( argc > 1 ){
    port = atoi( argv[1] );
  }

  struct sockaddr_in server;
  server.sin_family = PF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( port );
  int len = sizeof( server );

  if ( bind( sock, (struct sockaddr *)&server, len ) < 0 )
  {
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }

  /* activate the socket as a listener */
  //listen( sock, 5 );   /* 5 is number of backlogged waiting client requests */
  int rc = listen( sock, 5 );   /* 5 is number of backlogged waiting client requests */
  if( rc < 0 ){
    perror("listen() failed\n");
    return EXIT_FAILURE;
  }
  printf( "Listener socket created and bound to port %d on fd %d\n", port, sock );

  struct sockaddr_in client;

  //keep accepting client connections
  //each connection is only 1 request and response
  while ( 1 )
  {
    //printf( "Blocked on accept()\n" );
    unsigned int fromlen = sizeof(client);
    int newsock = accept( sock, (struct sockaddr *)&client, &fromlen );
                  /* accept() blocks */
    if( newsock < 0 ){
      perror("accept() failed\n");
      return EXIT_FAILURE;
    }
    //printf( "Accepted client connection\n" );

    int localRequest = 0;
    char * buffer;
    int requestLen = readHead( newsock, &buffer );
    if( requestLen > 0 ){
      //printf( "len: %d\n%s\n", requestLen, buffer );
      //Check type of request
      char * type = getFirstWord( buffer );
      char * contentLength;
      //printf( "type:%s\n", type );

      //filter on the request type
      if( strcmp( type, "GET" ) && strcmp( type, "POST" ) && strcmp( type, "HEAD" ) ){
        //printf("405\n");
        //send 405 error
        char * fourOhFive = construct405();
        int n = write( newsock, fourOhFive, strlen(fourOhFive) );
        if( n < strlen(fourOhFive) ){
          perror("Write() failed\n");
          return EXIT_FAILURE;
        }
        free( fourOhFive );
        localRequest = 1;
      }
      else if( !strcmp( type, "POST" ) ){
        //get content length
        contentLength = getTagValue( buffer, "Content-Length" );
        //If not content length send a 400 Bad Request
        if( contentLength == NULL ){
          char * fourOhFour = construct400();
          int n = write( newsock, fourOhFour, strlen(fourOhFour));
          if( n < strlen(fourOhFour )){
            perror("Write() failed\n");
            return EXIT_FAILURE;
          }
          free( fourOhFour );
          localRequest = 1;
        }
      }

      //try to get the hostname
      char * host;
      if( !localRequest ){
        host = getHost( buffer );
        if( host == NULL ){
          char * fourOhFour = construct400();
          int n = write( newsock, fourOhFour, strlen(fourOhFour));
          if( n < strlen(fourOhFour) ){
            perror("Write() failed\n");
            return EXIT_FAILURE;
          }
          free( fourOhFour );
          localRequest = 1;
        }
      }

      //get client name here
      struct sockaddr_in sockAddr;
      socklen_t sockLen = sizeof(sockAddr);
      int rc = getpeername(newsock, (struct sockaddr*)&sockAddr, &sockLen);
      if( rc < 0 ){
        perror( "getpeername() failed\n");
        return EXIT_FAILURE;
      }
      struct hostent *hp;

      if ((hp = gethostbyaddr((const void *) &sockAddr.sin_addr, sizeof( sockAddr.sin_addr ) , AF_INET)) == NULL) perror("no name associated\n");

      //filter on hostname here
      int filter;
      if( !localRequest){
        for( filter = 2; filter < argc; filter++ ){
          if( beginsWith( host, argv[filter] ) || endsWith( host, argv[filter] ) ) {
            localRequest = 1;
            //Filterd request send a 400
            char * fourOhFour = construct400();
            int n = write( newsock, fourOhFour, strlen(fourOhFour));
            if( n < strlen(fourOhFour )){
              perror("Write() failed\n");
              return EXIT_FAILURE;
            }
            free( host );
            free( fourOhFour );
            break;
          }
        }
      }

      //print request here
      if( localRequest ){
        free( buffer );
        close( newsock );
        continue;
      }
      else{
        print_request( hp->h_name, buffer, localRequest );
      }

      //edit request
      char * tmpBuffer;
      requestLen = editRequest( buffer, type, host, contentLength, &tmpBuffer );
      free( buffer );
      buffer = tmpBuffer;

      if(  !strcmp( type, "POST" ) ){
        //read and append content
        char * content;
        int contentRead = readBytes( newsock, &content, atoi(contentLength) );
        //printf("%s, %d\n", contentLength, contentRead);
        free( contentLength );
        char * tmpBuffer = (char *) malloc( contentRead + requestLen + 1 );
        int i;
        for( i = 0; i < requestLen; i++ ){
          tmpBuffer[i] = buffer[i];
        }
        for( i = 0; i < contentRead; i++ ){
          tmpBuffer[i + requestLen] = content[i];
        }
        tmpBuffer[requestLen + contentRead] = '\0';
        free( buffer );
        buffer = tmpBuffer;
      }

      //Call http_request
      //printf("request:\n%s\n", buffer);
      char * response;
      int responseLen = http_request( buffer, &response, host );
      //printf( "%s\n", response );
      int n = write( newsock, response, responseLen );
      if( n < responseLen ){
        perror("write() failed\n");
        return EXIT_FAILURE;
      }

      free( response );
      free( buffer );
      free( host );
    }
    else{
      //printf("no data read\n");
    }

    close( newsock );
  }


  return EXIT_SUCCESS;
}

