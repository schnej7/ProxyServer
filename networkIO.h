/* JERRY SCHNEIDER */
char * construct400();
char * construct405();
int readBytes( int fd, char ** data, int bytesToRead );
int readHead( int fd, char ** data );
int readChunk( int fd, char ** buffer, int * chunkSize );
int readChunked( int fd, char ** buffer );

