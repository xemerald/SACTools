/**
 * @file integral_sac.c
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
#include <stdint.h>
#include <errno.h>
/* */
#include <sachead.h>
#include <sac_proc.h>
#include <iirfilter.h>

/* */
#define PROG_NAME       "integral_sac"
#define VERSION         "1.0.0 - 2023-05-16"
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
static FILE   *OutputFP   = NULL;
static uint8_t FilterFlag = HP_FILTER_OFF;

/*
 *
 */
int main( int argc, char **argv )
{
	int      i, npts, datalen;
	float   *seis_raw;
	float   *seis_proc;
	float    acc0, vel0;
	uint8_t *outbuf = NULL;

	struct SAChead sh;
	IIR_FILTER     filter;
	IIR_STAGE     *stage;

/* */
	if ( proc_argv( argc, argv ) ) {
		usage();
		return -1;
	}
/* */
	if ( sac_proc_sac_load( InputFile, &sh, &seis_raw ) < 0 )
		return -1;
	if ( sh.delta < 0.001 ) {
		fprintf(stderr, "%s SAC sample delta too small: %f\n", sac_proc_scnl_print( &sh ), sh.delta);
		free(seis_raw);
		return -1;
	}
/* For Recursive Filter high pass 2 poles at 0.075 Hz */
	filter = designfilter( 2, IIR_HIGHPASS_FILTER, IIR_BUTTERWORTH, 0.075, 0.0, sh.delta );
	stage  = (IIR_STAGE *)calloc(filter.nsects, sizeof(IIR_STAGE));
	memset(stage, 0, sizeof(IIR_STAGE) * filter.nsects);
/* */
	fprintf(
		stderr, "%s SAC start at %4.4d,%3.3d,%2.2d:%2.2d:%2.2d.%4.4d %f\n",
		sac_proc_scnl_print( &sh ),
		sh.nzyear, sh.nzjday, sh.nzhour, sh.nzmin, sh.nzsec, sh.nzmsec, (double)sh.b + sac_proc_reftime_fetch( &sh )
	);
/* */
	npts    = (int)sh.npts;
	datalen = npts * sizeof(float);
	outbuf  = malloc(sizeof(struct SAChead) + datalen);
	memcpy(outbuf, (char *)&sh, sizeof(struct SAChead));
	seis_proc = (float *)(outbuf + sizeof(struct SAChead));
/* */
	sac_proc_data_preprocess( seis_raw, npts, GainFactor );
/* */
	acc0 = 0.0;
	vel0 = 0.0;
	for ( i = 0; i < npts; i++ ) {
		seis_proc[i] = (seis_raw[i] + acc0) * sh.delta * 0.5 + vel0;
		acc0  = seis_raw[i];
		vel0  = seis_proc[i];
	/* First time, forward filtering */
		if ( FilterFlag )
			seis_proc[i] = applyfilter( seis_proc[i], &filter, stage );
	}
/* Second time, backward filtering */
	if ( FilterFlag == HP_FILTER_ZP ) {
		memset(stage, 0, sizeof(IIR_STAGE) * filter.nsects);
		for ( i = npts - 1; i >= 0; i-- )
			seis_proc[i] = applyfilter( seis_proc[i], &filter, stage );
	}
/* */
	i = 0;
	datalen += sizeof(struct SAChead);
	if ( fwrite(outbuf, 1, datalen, OutputFP) != datalen ) {
		fprintf(stderr, "Error writing sacfile: %s\n", strerror(errno));
		i = -1;
	}
	else {
		fprintf(stderr, "%s SAC integral finished!\n", sac_proc_scnl_print( &sh ) );
	}
/* */
	fclose(OutputFP);
	free(seis_raw);
	free(outbuf);
	free(stage);

	return i;
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
		" -f             Turn on the high pass filter at 0.075 Hz\n"
		" -fz            Turn on the zero phase high pass filter at 0.075 Hz\n"
		"\n"
		"This program will integral the input SAC file once.\n"
		"\n"
	);

	return;
}
