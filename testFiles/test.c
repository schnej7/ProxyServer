#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char * getTagValue( char * data, char * tag );
char * getFirstWord( char * data );

int main(){

  char * data = "TagOne: one1\r\nTagTwo: two2\r\nTagThree: three3\r\n\r\n";
  char * tag = "TagTwo";

  printf( "data:\n%s\ntag: %s\n", data, tag );

  char * value = getTagValue( data, tag );
  
  printf( "value: %s\n", value );

  free( value );

  char * firstWord = getFirstWord( data );

  printf( "firstWord: %s\n", firstWord );

  free( firstWord );

  return EXIT_SUCCESS;
}

