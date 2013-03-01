/* server.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
  int sock = socket( PF_INET, SOCK_STREAM, 0 );
                              /* SOCK_DGRAM for unreliable UDP */

  if ( sock < 0 ) {
    perror( "socket() failed" );
    return EXIT_FAILURE;
  }

  unsigned short port = 8123; /* port number to listen on */

  struct sockaddr_in server;
  server.sin_family = PF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( port );
                    /* host-to-network-short() convert to big endian */
  int len = sizeof( server );

  if ( bind( sock, (struct sockaddr *)&server, len ) < 0 )
  {
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }

  /* activate the socket as a listener */
  listen( sock, 5 );   /* 5 is number of backlogged waiting client requests */
  printf( "Listener socket created and bound to port %d on fd %d\n", port, sock );

  struct sockaddr_in client;

  while ( 1 )
  {
    printf( "Blocked on accept()\n" );
    int fromlen;
    int newsock = accept( sock, (struct sockaddr *)&client, &fromlen );
                  /* accept() blocks */
    printf( "Accepted client connection\n" );

    char buffer[1024];
    int n = read( newsock, buffer, 1024 );
    if ( n < 1 )
    {
      perror( "read() failed" );
    }
    else
    {
      buffer[n] = '\0';
      printf( "Rcvd message from client: %s\n", buffer );

      char * msg = "GOT IT";

      /* write a message out on the socket connection */
      int n = write( newsock, msg, strlen( msg ) );

      if ( n < strlen( msg ) )
      {
        perror( "write() failed" );
        return EXIT_FAILURE;
      }
    }

    close( newsock );
  }

  return EXIT_SUCCESS;
}

