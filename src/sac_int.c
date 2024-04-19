/**
 * @file sac_int.c
 * @author Benjamin Ming Yang (b98204032@gmail.com)
 * @brief
 * @version 1.0.1
 * @date 2024-04-05
 *
 * @copyright Copyright (c) 2023-now
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
#include <iirfilter.h>

/* */
#define PROG_NAME       "sac_int"
#define VERSION         "1.0.1 - 2024-04-05"
#define AUTHOR          "Benjamin Ming Yang"
/* */
#define HP_FILTER_OFF  0
#define HP_FILTER_ON   1
#define HP_FILTER_ZP   2
/* */
static int  proc_argv( int , char * [] );
static void usage( void );
/* */
static float   GainFactor = 1.0;
static char   *InputFile  = NULL;
static char   *OutputFile = NULL;
static uint8_t FilterFlag = HP_FILTER_OFF;

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main( int argc, char **argv )
{
	int      npts, datalen;
	int      result    = -1;
	FILE    *ofp       = stdout;
	uint8_t *outbuf    = NULL;
	float   *seis_raw  = NULL;
	float   *seis_proc = NULL;
	float    half_delta;
	float    last_raw, last_proc;

	struct SAChead sh;
	IIR_FILTER     filter;
	IIR_STAGE     *stage = NULL;

/* Check command line arguments */
	if ( proc_argv( argc, argv ) ) {
		usage();
		return -1;
	}

/* Load the SAC file to local memory */
	if ( sac_file_load( InputFile, &sh, &seis_raw ) < 0 )
		goto end_process;
/* Then check the sampling rate, it should not larger than 1000 Hz */
	if ( sh.delta < 0.001 ) {
		fprintf(stderr, "SAC file: %s sample delta too small: %f\n", InputFile, sh.delta);
		goto end_process;
	}
/* For Recursive Filter high pass 2 poles at 0.075 Hz */
	filter = iirfilter_design( 2, IIR_HIGHPASS_FILTER, IIR_BUTTERWORTH, 0.075, 0.0, sh.delta );
	stage  = (IIR_STAGE *)calloc(filter.nsects, sizeof(IIR_STAGE));
	memset(stage, 0, sizeof(IIR_STAGE) * filter.nsects);

/* Start the main process */
	fprintf(
		stderr, "SAC file: %s start at %4.4d,%3.3d,%2.2d:%2.2d:%2.2d.%4.4d %f\n",
		InputFile, sh.nzyear, sh.nzjday, sh.nzhour, sh.nzmin, sh.nzsec, sh.nzmsec, (double)sh.b + sac_reftime_fetch( &sh )
	);
/* Setup some parameters which will be used later */
	npts       = (int)sh.npts;
	half_delta = sh.delta * 0.5;
	datalen    = npts * sizeof(float);
	outbuf     = malloc(sizeof(struct SAChead) + datalen);
	seis_proc  = (float *)(outbuf + sizeof(struct SAChead));
/* Copy the original header to the new buffer */
	memcpy(outbuf, (char *)&sh, sizeof(struct SAChead));
/* First, preprocess the raw seismic data */
	sac_data_preprocess( &sh, seis_raw, GainFactor );
/* Then, do the integration */
	last_raw  = 0.0;
	last_proc = 0.0;
	for ( int i = 0; i < npts; i++ ) {
		seis_proc[i] = (seis_raw[i] + last_raw) * half_delta + last_proc;
		last_raw  = seis_raw[i];
		last_proc = seis_proc[i];
	/* First time, forward filtering */
		if ( FilterFlag )
			seis_proc[i] = iirfilter_apply( seis_proc[i], &filter, stage );
	}
/* Second time, backward filtering if needed! */
	if ( FilterFlag == HP_FILTER_ZP ) {
		memset(stage, 0, sizeof(IIR_STAGE) * filter.nsects);
		for ( int i = npts - 1; i >= 0; i-- )
			seis_proc[i] = iirfilter_apply( seis_proc[i], &filter, stage );
	}

/* If user chose to output the result to local file, then open the file descript to write */
	if ( OutputFile && (ofp = fopen(OutputFile, "wb")) == (FILE *)NULL ) {
		fprintf(stderr, "ERROR!! Can't open %s for output! Exiting!\n", OutputFile);
		exit(-1);
	}
/* Real output block, write the output buffer to SAC file */
	datalen += sizeof(struct SAChead);
	if ( fwrite(outbuf, 1, datalen, ofp) != datalen ) {
		fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
		if ( OutputFile )
			remove(OutputFile);
	}
	else {
		fprintf(stderr, "SAC file: %s integration finished!\n", InputFile);
		result = 0;
	}

end_process:
	if ( ofp != stdout )
		fclose(ofp);
	if ( seis_raw )
		free(seis_raw);
	if ( outbuf )
		free(outbuf);
	if ( stage )
		free(stage);

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
		else if ( !strcmp(argv[i], "-f") ) {
			FilterFlag = HP_FILTER_ON;
		}
		else if ( !strcmp(argv[i], "-fz") ) {
			FilterFlag = HP_FILTER_ZP;
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
		" -f             Turn on the high pass filter at 0.075 Hz\n"
		" -fz            Turn on the zero phase high pass filter at 0.075 Hz\n"
		"\n"
		"This program will integral the input SAC file once.\n"
		"\n"
	);

	return;
}
