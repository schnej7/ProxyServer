/* client.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

int main()
{
  int sock = socket( PF_INET, SOCK_STREAM, 0 );

  if ( sock < 0 )
  {
    perror( "socket() failed" );
    return EXIT_FAILURE;
  }

  struct sockaddr_in server;
  struct hostent * hp;

  server.sin_family = PF_INET;
  hp = gethostbyname( "localhost" );
  if ( hp == NULL )
  {
    perror( "Unknown host" );
    return EXIT_FAILURE;
  }

  bcopy( (char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length );
  int port = 8123;
  server.sin_port = htons( port );

  if ( connect( sock, (struct sockaddr *)&server, sizeof( server ) ) < 0 )
  {
    perror( "connect() failed" );
    return EXIT_FAILURE;
  }

  char * msg = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  /* write a message out on the socket connection */
  int n = write( sock, msg, strlen( msg ) );

  if ( n < strlen( msg ) )
  {
    perror( "write() failed" );
    return EXIT_FAILURE;
  }

  char buffer[1024];
  n = read( sock, buffer, 1024 );
  if ( n < 1 )
  {
    perror( "read() failed" );
    return EXIT_FAILURE;
  }
  else
  {
    buffer[n] = '\0';
    printf( "Rcvd message from server: %s\n", buffer );
  }

  close( sock );

  return EXIT_SUCCESS;
}

