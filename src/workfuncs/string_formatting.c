/*
Copyright (c) 2022 BostonBSD. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    (1) Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    (2) Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

    (3)The name of the author may not be used to
    endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <string.h>

#include <stdbool.h>
#include <ctype.h>

#include <locale.h>
#include <monetary.h>

#include "../include/macros.h"

bool CheckValidString (const char *string)
{
	size_t len = strlen( string );
	
	/* The string cannot begin with these characters  */
	if ( strchr( " _\0" , (int) string[0] ) ) return false;
	
	/* The string cannot end with these characters  */
	if ( strchr( " _." , (int) string[ len - 1 ] ) ) return false;
	
	/* The string cannot contain these characters  */
	if ( strpbrk( string, "\n\"\'\\)(][}{~`, " ) ) return false;
	
	/* The string cannot contain a double space  */
	if ( strstr( string, "  " ) ) return false;
	
	return true;
}

bool CheckIfStringDoubleNumber (const char *string)
{
	char *end_ptr;
    strtod( string, &end_ptr );

    /* If no conversion took place or if conversion not complete. */
    if( (end_ptr == string) || (*end_ptr != '\0') ){
        return false;
    }
	
	return true;
}

bool CheckIfStringDoublePositiveNumber (const char *string)
{
	char *end_ptr;
    double num = strtod( string, &end_ptr );

    /* If no conversion took place or if conversion not complete. */
    if( (end_ptr == string) || (*end_ptr != '\0') ) return false;
    if( num < 0 ) return false;

	
	return true;
}

bool CheckIfStringLongPositiveNumber (const char *string)
{
	char *end_ptr;
    long num = strtol( string, &end_ptr, 10 );

    /* If no conversion took place or if conversion not complete. */
    if( (end_ptr == string) || (*end_ptr != '\0') ) return false;
    if ( num < 0 ) return false;
	
	return true;
}

void FormatStr (char *s)
/* Remove all dollar signs '$' and commas ',' from a string */
{
    /* Read character by character until the null character is reached. */
    for(int i = 0; s[i] != '\0'; i++){
        /* If we find a '$' OR ',' character */
        if(s[i]=='$' || s[i]==','){
            /* Read each character thereafter and */
            for(int j = i; s[j] != '\0'; j++){
                /* Shift the array down one character [remove the character] */
                s[j]=s[j+1];
            }
        /* Check the new value of this increment [if there were a duplicate character]. */
        i--; 
        }
    }
}

char* StringToMonetary (const char *s) 
/* Take in a string, convert to a monetary string.
   If the string cannot be converted to a double, undefined behavior.
   Must free return value. */
{
    double num = strtod(s, NULL);
    size_t len = strlen("###,###,###,###,###.###") + 1;
    char* str = (char*) malloc ( len ); 

    /* From the locale man page: By default, C programs start in the "C" locale. */
    setlocale ( LC_ALL, LOCALE ); 
    strfmon ( str, len, "%(.3n", num );

    return str;
}

char* DoubleToMonetary (const double num) 
/* Take in a const double and convert to a monetary format string. 
   Must free return value.
*/
{    
    size_t len = strlen("###,###,###,###,###.###") + 1;
    char* str = (char*) malloc( len ); 

    /* The C.UTF-8 locale does not have a monetary 
       format and is the default in C. 
    */
    setlocale(LC_ALL, LOCALE);  
    strfmon(str, len, "%(.3n", num);

    return str;
}

void LowerCaseStr (char *s){
    /* Convert the string to lowercase letters */
    for(short i = 0; s[i]; i++){
        s[i] = tolower( s[i] );
    }
}

void UpperCaseStr (char *s){
    /* Convert the string to uppercase letters */
    for(short i = 0; s[i]; i++){
        s[i] = toupper( s[i] );
    }
}

void Chomp (char *s)
/* Locate first newline character '\n' in a string, replace with NULL */
{
	char *ch = strchr( s, (int)'\n' );
    if ( ch != NULL ) *ch = 0;
}