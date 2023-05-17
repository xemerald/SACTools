/**
 * @file scnl_mod_sac.c
 * @author Benjamin Ming Yang (b98204032@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2023-05-16
 *
 * @copyright Copyright (c) 2023-now
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
/* */
#include <sachead.h>
#include <sac_proc.h>

/* */
#define PROG_NAME       "scnl_mod_sac"
#define VERSION         "1.0.0 - 2023-05-16"
#define AUTHOR          "Benjamin Ming Yang"
/* */
static int  proc_argv( int , char * [] );
static void usage( void );
/* */
static char *InputFile = NULL;
static FILE *OutputFP  = NULL;
static char *NewSta    = NULL;
static char *NewChan   = NULL;
static char *NewNet    = NULL;
static char *NewLoc    = NULL;

/*
 *
 */
int main( int argc, char **argv )
{
	struct SAChead sh;
	float         *seis = NULL;
	int            size = 0;
	int            result = -1;
	char           orig_scnl[64] = { 0 };

/* Check command line arguments */
	if ( proc_argv( argc, argv ) ) {
		usage();
		return -1;
	}

/* */
	if ( (size = sac_proc_sac_load( InputFile, &sh, &seis )) < 0 )
		goto end_process;
/* */
	strcpy(orig_scnl, sac_proc_scnl_print( &sh ));
	sac_proc_scnl_modify( &sh, NewSta, NewChan, NewNet, NewLoc );
	sac_proc_az_inc_modfify( &sh, SACUNDEF, SACUNDEF );
/* */
	if ( fwrite(&sh, 1, sizeof(struct SAChead), OutputFP) != sizeof(struct SAChead) ) {
		fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
		goto end_process;
	}
/* */
	size -= sizeof(struct SAChead);
	if ( fwrite(seis, 1, size, OutputFP) != size ) {
		fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
		goto end_process;
	}
/* */
	fprintf(stderr, "SAC file: %s SCNL has been modified (%s -> %s)!\n", InputFile, orig_scnl, sac_proc_scnl_print( &sh ));
	result = 0;

end_process:
	fclose(OutputFP);
	if ( seis )
		free(seis);

	return result;
}

/*
 *
 */
static int proc_argv( int argc, char *argv[] )
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( !strcmp(argv[i], "-v") ) {
			fprintf(stdout, "%s\n", PROG_NAME);
			fprintf(stdout, "version: %s\n", VERSION);
			fprintf(stdout, "author:  %s\n", AUTHOR);
			exit(0);
		}
		else if ( !strcmp(argv[i], "-h") ) {
			usage();
			exit(0);
		}
		else if ( !strcmp(argv[i], "-s") ) {
			NewSta = argv[++i];
		}
		else if ( !strcmp(argv[i], "-c") ) {
			NewChan = argv[++i];
		}
		else if ( !strcmp(argv[i], "-n") ) {
			NewNet = argv[++i];
		}
		else if ( !strcmp(argv[i], "-l") ) {
			NewLoc = argv[++i];
		}
		else if ( i == argc - 1 ) {
#ifdef _WINNT
			usage();
			return -1;
#else
			InputFile = argv[i];
			OutputFP  = stdout;
#endif
		}
		else if ( i == argc - 2 ) {
			InputFile = argv[i];
			i++;
			break;
		}
		else {
			fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
			return -1;
		}
	}
/* */
	if ( !NewSta && !NewChan && !NewNet && !NewLoc ) {
		fprintf(stderr, "No new SCNL was specified; ");
		fprintf(stderr, "exiting with error!\n\n");
		return -1;
	}
/* */
	if ( !InputFile ) {
		fprintf(stderr, "No input file was specified; ");
		fprintf(stderr, "exiting with error!\n\n");
		return -1;
	}
/* */
	if ( !OutputFP ) {
		if ( (OutputFP = fopen(argv[i], "wb")) == (FILE *)NULL ) {
			fprintf(stderr, "ERROR!! Can't open %s for output! Exiting!\n", argv[i]);
			exit(-1);
		}
	}

	return 0;
}

/*
 *
 */
static void usage( void )
{
	fprintf(stdout, "\n%s\n", PROG_NAME);
	fprintf(stdout, "version: %s\n", VERSION);
	fprintf(stdout, "author:  %s\n", AUTHOR);
	fprintf(stdout, "***************************\n");
	fprintf(stdout, "Usage: %s [options] <input SAC file> > <output SAC file>\n", PROG_NAME);
	fprintf(stderr, "       or %s [options] <input SAC file> <output SAC file>\n\n", PROG_NAME);
	fprintf(stdout,
		"*** Options ***\n"
		" -v               Report program version\n"
		" -h               Show this usage message\n"
		" -s station_code  Specify the new station code, max length is 8\n"
		" -c channel_code  Specify the new channel code, max length is 8\n"
		" -n network_code  Specify the new network code, max length is 8\n"
		" -l location_code Specify the new location code, max length is 8\n"
		"\n"
		"This program will change the SCNL of the input SAC file.\n"
		"\n"
	);

	return;
}
