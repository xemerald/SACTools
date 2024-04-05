/**
 * @file sac_preproc.c
 * @author Benjamin Ming Yang (b98204032@gmail.com)
 * @brief
 * @version 1.0.1
 * @date 2024-04-04
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
#include <sac.h>
/* */
#define PROG_NAME       "sac_preproc"
#define VERSION         "1.0.1 - 2024-04-04"
#define AUTHOR          "Benjamin Ming Yang"
/* */
static int  proc_argv( int, char * [] );
static void usage( void );
/* */
static float GainFactor = 1.0;
static char *InputFile  = NULL;
static char *OutputFile = NULL;

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main( int argc, char **argv )
{
	struct SAChead sh;
	FILE    *ofp    = stdout;
	uint8_t *outbuf = NULL;
	float   *seis   = NULL;
	int      result = -1;
	int      datalen;

/* Check command line arguments */
	if ( proc_argv( argc, argv ) ) {
		usage();
		return -1;
	}

/* Load the SAC file to local memory */
	if ( sac_file_load( InputFile, &sh, &seis ) < 0 )
		goto end_process;
	fprintf(
		stderr, "SAC file: %s start at %4.4d,%3.3d,%2.2d:%2.2d:%2.2d.%4.4d %f\n",
		InputFile, sh.nzyear, sh.nzjday, sh.nzhour, sh.nzmin, sh.nzsec, sh.nzmsec, (double)sh.b + sac_reftime_fetch( &sh )
	);
/* Allocate the output buffer */
	datalen = (int)sh.npts * sizeof(float);
	outbuf  = malloc(sizeof(struct SAChead) + datalen);
	memcpy(outbuf, (char *)&sh, sizeof(struct SAChead));

/* The main process & copy the result to output buffer */
	sac_data_preprocess( &sh, seis, GainFactor );
	memcpy(outbuf + sizeof(struct SAChead), seis, datalen);

/* If user chose to output the result to local file, then open the file descript to write */
	if ( OutputFile && (ofp = fopen(OutputFile, "wb")) == (FILE *)NULL ) {
		fprintf(stderr, "ERROR!! Can't open %s for output! Exiting!\n", OutputFile);
		goto end_process;
	}
/* Real output block, write the output buffer to SAC file */
	datalen += sizeof(struct SAChead);
	if ( fwrite(outbuf, 1, datalen, ofp) != datalen ) {
		fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
		if ( OutputFile )
			remove(OutputFile);
	}
	else {
		fprintf(stderr, "SAC file: %s preprocessing finished!\n", InputFile);
		result = 0;
	}

end_process:
	if ( ofp != stdout )
		fclose(ofp);
	if ( seis )
		free(seis);
	if ( outbuf )
		free(outbuf);

	return result;
}

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
static int proc_argv( int argc, char *argv[] )
{
	for ( int i = 1; i < argc; i++ ) {
		if ( !strcmp(argv[i], "-v") ) {
			fprintf(stdout, "%s\n", PROG_NAME);
			fprintf(stdout, "Version: %s\n", VERSION);
			fprintf(stdout, "Author:  %s\n", AUTHOR);
			fprintf(stdout, "Compiled at %s %s\n", __DATE__, __TIME__);
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
			OutputFile = NULL;
#endif
		}
		else if ( i == argc - 2 ) {
			InputFile = argv[i++];
			OutputFile = argv[i];
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

	return 0;
}

/**
 * @brief
 *
 */
static void usage( void )
{
	fprintf(stdout, "\n%s\n", PROG_NAME);
	fprintf(stdout, "Version: %s\n", VERSION);
	fprintf(stdout, "Author:  %s\n", AUTHOR);
	fprintf(stdout, "Compiled at %s %s\n", __DATE__, __TIME__);
	fprintf(stdout, "***************************\n");
	fprintf(stdout, "Usage: %s [options] <input SAC file> > <output SAC file>\n", PROG_NAME);
	fprintf(stdout, "       or %s [options] <input SAC file> <output SAC file>\n\n", PROG_NAME);
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
