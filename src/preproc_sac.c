
/**
 * @file preproc_sac.c
 * @author Benjamin Ming Yang (b98204032@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2023-05-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
/* */
#include <sachead.h>
#include <sac_proc.h>
/* */
#define PROG_NAME       "preproc_sac"
#define VERSION         "1.0.0 - 2023-05-16"
#define AUTHOR          "Benjamin Ming Yang"
/* */
static int  proc_argv( int, char * [] );
static void usage( void );
/* */
static float   GainFactor = 1.0;
static char   *InputFile  = NULL;
static FILE   *OutputFP   = NULL;

/*
 *
 */
int main( int argc, char **argv )
{
	struct SAChead sh;
	uint8_t *outbuf;
	float   *seis;
	int      datalen;
	int      result = 0;

/* */
	if ( proc_argv( argc, argv ) ) {
		usage();
		return -1;
	}
/* */
	if ( sac_proc_sac_load( InputFile, &sh, &seis ) < 0)
		return -1;
	fprintf(
		stderr, "%s SAC start at %4.4d,%3.3d,%2.2d:%2.2d:%2.2d.%4.4d %f\n",
		sac_proc_scnl_print( &sh ),
		sh.nzyear, sh.nzjday, sh.nzhour, sh.nzmin, sh.nzsec, sh.nzmsec, (double)sh.b + sac_proc_reftime_fetch( &sh )
	);
/* */
	datalen = (int)sh.npts * sizeof(float);
	outbuf  = malloc(sizeof(struct SAChead) + datalen);
	memcpy(outbuf, (char *)&sh, sizeof(struct SAChead));
/* */
	sac_proc_data_preprocess( seis, sh.npts, GainFactor );
	memcpy(outbuf + sizeof(struct SAChead), seis, datalen);
/* */
	datalen += sizeof(struct SAChead);
	if ( fwrite(outbuf, 1, datalen, OutputFP) != datalen ) {
		fprintf(stderr, "Error writing sacfile: %s\n", strerror(errno));
		result = -1;
	}
	else {
		fprintf(stderr, "%s SAC preprocessing finished!\n", sac_proc_scnl_print( &sh ) );
	}
/* */
	fclose(OutputFP);
	free(seis);
	free(outbuf);

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
		else if ( !strcmp(argv[i], "-g") ) {
			GainFactor = atof(argv[++i]);
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
	fprintf(stdout, "Usage: %s [options] <input sacfile> > <output sacfile>\n", PROG_NAME);
	fprintf(stderr, "       or %s [options] <input sacfile> <output sacfile>\n\n", PROG_NAME);
	fprintf(stdout,
		"*** Options ***\n"
		" -v             Report program version\n"
		" -h             Show this usage message\n"
		" -g gain_factor Specify the gain factor, it should be floating value\n"
		"\n"
		"This program will fill the gap and apply the gain factor to the input SAC file.\n"
		"\n"
	);

	return;
}
