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

#include "../financials.h"

bool check_valid_string(const char* string)
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

bool check_if_string_double_number(const char* string)
{
	char *end_ptr;
    strtod( string, &end_ptr );

    /* If no conversion took place or if conversion not complete. */
    if( (end_ptr == string) || (*end_ptr != '\0') ){
        return false;
    }
	
	return true;
}

bool check_if_string_double_positive_number(const char* string)
{
	char *end_ptr;
    double num = strtod( string, &end_ptr );

    /* If no conversion took place or if conversion not complete. */
    if( (end_ptr == string) || (*end_ptr != '\0') ) return false;
    if( num < 0 ) return false;

	
	return true;
}

bool check_if_string_long_positive_number(const char* string)
{
	char *end_ptr;
    long num = strtol( string, &end_ptr, 10 );

    /* If no conversion took place or if conversion not complete. */
    if( (end_ptr == string) || (*end_ptr != '\0') ) return false;
    if ( num < 0 ) return false;
	
	return true;
}

void FormatStr(char *s)
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

char* StringToMonetary (const char *s) {
    
    double num = strtod(s, NULL);
    char* str = (char*) malloc ( strlen( "############.####" )+1 ); 

    /* From the locale man page: By default, C programs start in the "C" locale. */
    setlocale ( LC_ALL, LOCALE ); 
    strfmon ( str, strlen ( "############.####" ), "%(.3n", num );

    return str;
}

void LowerCaseStr(char *s){
    /* Convert the string to lowercase letters */
    for(short i = 0; s[i]; i++){
        s[i] = tolower( s[i] );
    }
}

void UpperCaseStr(char *s){
    /* Convert the string to uppercase letters */
    for(short i = 0; s[i]; i++){
        s[i] = toupper( s[i] );
    }
}

void chomp(char *s)
/* Locate first newline character '\n' in a string, replace with NULL */
{
	char *ch = strchr( s, (int)'\n' );
    if ( ch != NULL ) *ch = 0;
}

bool check_symbol( const char *s )
/* Depository share symbols include dollars signs, which we don't want. 
   The first line of the file includes the "Symbol" string, which we don't want.
   The last line of the file includes the "File Creation Time" string, which we don't want.
*/
{ 
    /* The string cannot contain a '$' character */
	if ( strpbrk( s, "$" ) ) return false;

    if ( strstr( s, "Symbol" ) ) return false;
    if ( strstr( s, "File Creation Time" ) ) return false;

    return true;
}