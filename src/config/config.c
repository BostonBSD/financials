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

#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <errno.h>

#include "../include/sqlite.h"
#include "../include/gui_types.h"       /* window_data */
#include "../include/class_types.h"     /* equity_folder, metal, meta */
#include "../include/macros.h"

static void config_dir_processing ( const char *home_dir )
/* Check if the "~/.config" and "~/.config/financials" directories exist.
   if they do not exist then create them. */
{
    DIR *dp;
    int status;

    /* Append the .config directory to the end of home directory path. */
    size_t  len = strlen(home_dir) + strlen( CONFIG_DIR ) + 1;
    char *path = (char*) malloc( len );
    snprintf( path, len, "%s%s", home_dir, CONFIG_DIR );

    errno = 0;
    if ((dp = opendir( path )) == NULL) {
        switch (errno) {
            case EACCES: 
            	printf("Permission denied\n");
            	exit( EXIT_FAILURE ); 
            	break;
            case ENOENT: 
            	/* Make a directory with read/write/search permissions for owner and group, 
		           and with read/search permissions for others. */ 
            	status = mkdir( path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
                if( status != 0 ){
            		printf("Make directory failed: %s\nStatus Code: %d\n", path, status);
            		exit( EXIT_FAILURE );
            	}            	
            	break;
            case ENOTDIR: 
            	printf("'%s' is not a directory\n", path );
            	exit( EXIT_FAILURE ); 
            	break;
        }
    } else { 
        if (closedir(dp) == -1) perror("closedir");
    }
    free ( path );
    
    /* Append the config file directory to the end of the home directory path. */
    len = strlen(home_dir) + strlen( CONFIG_FILE_DIR ) + 1;
    path = (char*) malloc( len );
    snprintf( path, len, "%s%s", home_dir, CONFIG_FILE_DIR );
    
    if ((dp = opendir( path )) == NULL) {
        switch (errno) {
            case EACCES: 
            	printf("Permission denied\n");
            	exit( EXIT_FAILURE ); 
            	break;
            case ENOENT: 
            	/* Make a directory with read/write/search permissions for owner and group, 
		           and with read/search permissions for others. */
		        status = mkdir( path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
            	if( status != 0 ){
            		printf("Make directory failed: %s\nStatus Code: %d\n", path, status);
            		exit( EXIT_FAILURE );
            	}        	
            	break;
            case ENOTDIR: 
            	printf("'%s' is not a directory\n", path );
            	exit( EXIT_FAILURE ); 
            	break;
        }
    } else { 
        if (closedir(dp) == -1) perror("closedir");
    }
    free ( path );
}

void ReadConfig (metal* M, meta* D, equity_folder* F, window_data* Win){
    /* We aren't using a config text file any longer, but the sqlite db file
       serves as a config file for all intents and purposes. */
       
    /* Make sure the config directory exists. */
    config_dir_processing ( D->home_dir_ch );

    /* Process the sqlite db file and populate initial varables. */
    SqliteProcessing ( F, M, D, Win );
}