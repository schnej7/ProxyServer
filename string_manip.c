/* JERRY SCHNEIDER */
#include <stdlib.h>
#include <string.h>

//returns 1 if data begins with tag
int beginsWith( char * data, char * tag ){
  int i;
  for( i = 0; i < strlen( tag ); i++ ){
    if( data[i] != tag[i] ){
      return 0;
    }
  }
  return 1;
}

//returns 1 if data ends with tag
int endsWith( char * data, char * tag ){
  int i;
  for( i = 0; i < strlen(tag); i++ ){
    if( data[(strlen(data) - i) - 1] != tag[(strlen(tag) - i) - 1] ){
      return 0;
    }
  }
  return 1;
}

//returns the first word in data
char * getFirstWord( char * data ){
  int len = 0;
  while( data[len] != ' ' && data[len] != '\r' && data[len] != '\n' ) len++;
  char * firstWord = (char *) malloc( len + 1 );
  int i;
  for( i = 0; i < len; i++){
    firstWord[i] = data[i];
  }
  firstWord[len] = '\0';
  return firstWord;
}

//returns a pointer to the beginning of the first occurance of tag in data
char * findString( char * data, char * tag ){
  int i;
  for( i = 0; i < strlen( data ) - strlen( tag ); i++ ){
    int j;
    for( j = 0; j < strlen( tag ); j++ ){
      if( tag[j] != data[i + j] ){
        //not found
        break;
      }
    }

    if( j == strlen( tag ) ){
      //found it
      return &data[i];
    }
  }
  return NULL;
}


//returns the value of tag in data
char * getTagValue( char * data, char * tag ){
  char * tagLoc = findString( data, tag );

  if( tagLoc == NULL ) return NULL;

  while( *tagLoc != ' ' ) tagLoc++;
  tagLoc++;
  
  int valLength = 0;
  char * tempChar = tagLoc;
  while( *tempChar != '\n' && *tempChar != '\r' ){
    valLength++;
    tempChar++;
  }

  char * tagValue = (char *) malloc( sizeof( char ) * ( valLength + 1) );
  int i;
  for( i = 0; i < valLength; i++ ){
    tagValue[i] = tagLoc[i];
  }
  tagValue[ valLength ] = '\0';

  return tagValue;
}

