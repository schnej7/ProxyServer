/* JERRY SCHNEIDER */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "networkIO.h"
#include <sys/types.h>
#include <sys/socket.h>

//returns a 405 web page frome the file 405
char * construct405(){
  int bytesRead = 0;
  FILE * fd = fopen( "405", "r" );

  char buffer[1024];
  bytesRead = fread( &buffer, sizeof( char ), 1024, fd );

  char * fourOhFive = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
  char * response = (char *) malloc( sizeof(char) * (bytesRead + strlen(fourOhFive) + 1));

  int i;
  for( i = 0; i < strlen(fourOhFive); i++ ){
    response[i] = fourOhFive[i];
  }
  for( i = 0; i < bytesRead; i++ ){
    response[i + strlen(fourOhFive)] = buffer[i];
  }

  response[ strlen(fourOhFive) + bytesRead ] = '\0';

  fclose( fd );

  return response;
}

//returns a 400 web page frome the file 400
char * construct400(){
  int bytesRead = 0;
  FILE * fd = fopen( "400", "r" );

  char buffer[1024];
  bytesRead = fread( &buffer, sizeof( char ), 1024, fd );

  char * fourOhFour = "HTTP/1.1 400 Bad Request\r\n\r\n";
  char * response = (char *) malloc( sizeof(char) * (bytesRead + strlen(fourOhFour) + 1));

  int i;
  for( i = 0; i < strlen(fourOhFour); i++ ){
    response[i] = fourOhFour[i];
  }
  for( i = 0; i < bytesRead; i++ ){
    response[i + strlen(fourOhFour)] = buffer[i];
  }

  response[ strlen(fourOhFour) + bytesRead ] = '\0';

  fclose( fd );

  return response;
}

//returns the length of head from fd
//and points **head to the data
int readHead( int fd, char ** head ){
  char tempBuffer;
  
  int bytesRead = 0;
  int totalLength = 0;

  char * localHead = NULL;

  while( 1 ){

    if(totalLength > 4 && 
     ( localHead[totalLength - 4] == '\r' &&
       localHead[totalLength - 3] == '\n' &&
       localHead[totalLength - 2] == '\r' &&
       localHead[totalLength - 1] == '\n' ) ) break;

    bytesRead = read( fd, &tempBuffer, 1 );
    if( bytesRead < 0 ){
      perror("read() failed\n");
      return EXIT_FAILURE;
    }

    char * tmpPtr = localHead;
    localHead = (char *) malloc( totalLength + bytesRead + 1 );

    int i;
    for( i = 0; i < totalLength; i++ ){
      localHead[i] = tmpPtr[i];
    }
    localHead[ totalLength ] = tempBuffer;
    localHead[ totalLength + 1 ] = '\0';

    if(tmpPtr != NULL) free( tmpPtr );
    totalLength += bytesRead;
  }

  *head = localHead;
  return totalLength;
}

//reads bytesToRead bytes from fd into a new buffer and points **buffer at it
int readBytes( int fd, char ** buffer, int bytesToRead ){
  int i;
  int totalBytes = 0;
  char * tempBuffer = (char *) malloc( bytesToRead );
  for( i = 0; i < bytesToRead; i++ ){
    char tempChar;
    int bytes = read( fd, &tempChar, 1 );
    tempBuffer[i] = tempChar;
    totalBytes += bytes;
  }
  *buffer = tempBuffer;
  return totalBytes;
}

//reads chunked data from fd into a buffer and points **buffer at it
int readChunked( int fd, char ** buffer ){

  char * tempBuffer = (char *) malloc( 1 );
  int totalSize = 0;

  int chunkSize = 1;
  char * localBuffer;
  int bytesRead = readChunk( fd, &localBuffer, &chunkSize );

  while( chunkSize > 0 ){
    char * newBuffer = (char *) malloc( bytesRead + totalSize + 1);
    int i;
    for( i = 0; i < totalSize; i++ ){
      newBuffer[i] = tempBuffer[i];
    }
    for( i = 0; i < bytesRead; i++ ){
      newBuffer[i + totalSize] = localBuffer[i];
    }
    newBuffer[totalSize + bytesRead] = '\0';
    free( tempBuffer );
    tempBuffer = newBuffer;
    free( localBuffer );
    totalSize += bytesRead;
    bytesRead = readChunk( fd, &localBuffer, &chunkSize );
  }
  *buffer = tempBuffer;
  return totalSize;
}

//reads a single chunk from fd and points **buffer at it
int readChunk( int fd, char ** buffer, int * chunkSize ){

  //printf("reading chunk...\n");
  char tempBuffer[10];
  int bytesRead = 0;
  int totalLength = 0;
  int i;
  for( i = 0; i < 10; i++ ){
    tempBuffer[i] = '\0';
  }

  //get chunk size
  i = 0;
  while( i == 0 || tempBuffer[i - 1] != '\r' ){
    //printf("i = %d\n", i);
    bytesRead += read( fd, &tempBuffer[i], 1 );
    totalLength += 1;
    i++;
  }
  tempBuffer[i] = '\0';
  //printf("ChunkSizeString: %s\n", tempBuffer);
  int chunkLen = strtol( tempBuffer, NULL, 16 );
  *chunkSize = chunkLen;

  //done with chunks
  if( chunkLen == 0 ){
    char * totalChunk = (char *) malloc( bytesRead );
    for( i = 0; i < bytesRead; i++ ){
      totalChunk[i] = tempBuffer[i];
    }
    *buffer = totalChunk;
    return bytesRead;
  }

  chunkLen += 3;
  //printf("ChunkSize: %d\n", chunkLen);

  //make new buffer for chunk
  char * chunk;
  char * totalChunk = (char *) malloc( chunkLen + bytesRead );
  for( i = 0; i < bytesRead; i++ ){
    totalChunk[i] = tempBuffer[i];
  }
  
  int chunkBytesRead = readBytes( fd, &chunk, chunkLen );

  for( i = 0; i < chunkBytesRead; i++ ){
    totalChunk[i + bytesRead] = chunk[i];
  }

  //printf("total chunk:\n%s\n", totalChunk);

  *buffer = totalChunk;
  return chunkBytesRead + bytesRead;

}

