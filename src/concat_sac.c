/**
 * @file concat_sac.c
 * @author Benjamin Yang (b98204032@gmail.com)
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
#include <math.h>
#include <errno.h>
/* */
#include <sachead.h>
#include <sac_proc.h>


/* */
#define PROG_NAME       "concat_sac"
#define VERSION         "1.0.0 - 2023-05-16"
#define AUTHOR          "Benjamin Ming Yang"
/* */
#define MAX_TOLERANCE_GAP_SEC  86400
/* */
static int  proc_argv( int , char * [] );
static void usage( void );
/* */
static char   *InputFile1 = NULL;
static char   *InputFile2 = NULL;
static FILE   *OutputFP   = NULL;

/*
 *
 */
int main( int argc, char **argv )
{
	struct SAChead sh0, sh1;
	float         *seis0 = NULL;
	float         *seis1 = NULL;
	float         *gapseis = NULL;
	double         starttime0, starttime1;
	double         gaptime;
	int            gapsamp = 0;
	int            _sh0samp = 0;
	int            result = -1;

/* Check command line arguments */
	if ( proc_argv( argc, argv ) ) {
		usage();
		return -1;
	}

/* */
	if ( sac_proc_sac_load( InputFile1, &sh0, &seis0 ) < 0 )
		goto end_process;
	starttime0 = sac_proc_reftime_fetch( &sh0 );
	fprintf(
		stderr, "First input SAC file ref. time is %.3f, end at %.3f. Total %d samples with %.3f delta.\n",
		starttime0, starttime0 + sh0.e, sh0.npts, sh0.delta
	);

	if ( sac_proc_sac_load( InputFile2, &sh1, &seis1 ) < 0 )
		goto end_process;
	starttime1 = sac_proc_reftime_fetch( &sh1 );
	fprintf(
		stderr, "Second input SAC file ref. time is %.3f, end at %.3f. Total %d samples with %.3f delta.\n",
		starttime1, starttime1 + sh1.e, sh1.npts, sh1.delta
	);

	if ( fabs(sh0.delta - sh1.delta) > 0.000001 ) {
		fprintf(stderr, "The delta between these two SAC files are different(%f & %f). Just exit!\n", sh0.delta, sh1.delta);
		goto end_process;
	}

/* */
	gaptime = starttime1 - (starttime0 + sh0.e) + sh0.delta * 0.1;
	gapsamp = gaptime / sh1.delta - 1;
	if ( gapsamp >= 0 && gapsamp < (int)(MAX_TOLERANCE_GAP_SEC / sh1.delta) ) {
		fprintf(stderr, "Gap is %.3f seconds(total %d samples).\n", gaptime, gapsamp);
		fprintf(stderr, "Filling the gap with %.6f...\n", (double)SACUNDEF);
		gapseis = (float *)calloc(gapsamp, sizeof(float));
		for ( int i = 0; i < gapsamp; i++ )
			gapseis[i] = SACUNDEF;
	/* */
		_sh0samp = sh0.npts;
		sh0.npts += gapsamp + sh1.npts;
		sh0.e += gaptime + sh1.e;

	/* */
		if ( fwrite(&sh0, 1, sizeof(struct SAChead), OutputFP) != sizeof(struct SAChead) ) {
			fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
			goto end_process;
		}
	/* */
		if ( fwrite(seis0, sizeof(float), _sh0samp, OutputFP) != _sh0samp ) {
			fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
			goto end_process;
		}
		if ( fwrite(gapseis, sizeof(float), gapsamp, OutputFP) != gapsamp ) {
			fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
			goto end_process;
		}
		if ( fwrite(seis1, sizeof(float), sh1.npts, OutputFP) != sh1.npts ) {
			fprintf(stderr, "Error writing SAC file: %s\n", strerror(errno));
			goto end_process;
		}
	/* */
		fprintf(stderr, "SAC files: %s & %s concatenating finished!\n", InputFile1, InputFile2);
		result = 0;
	}
	else if ( gapsamp >= (int)(MAX_TOLERANCE_GAP_SEC / sh1.delta) ) {
		fprintf(stderr, "ERROR! There is a gap over %d sec. between two SAC files. Just exit!\n", MAX_TOLERANCE_GAP_SEC);
	}
	else {
		fprintf(stderr, "ERROR! There is an overlap between two SAC files. Just exit!\n");
	}

end_process:
	fclose(OutputFP);
	if ( gapseis )
		free(gapseis);
	if ( seis0 )
		free(seis0);
	if ( seis1 )
		free(seis1);

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
		else if ( i == argc - 1 ) {
#ifdef _WINNT
			usage();
			return -1;
#else
			InputFile2 = argv[i];
			OutputFP   = stdout;
#endif
		}
		else if ( i == argc - 2 || i == argc - 3 ) {
			if ( InputFile1 ) {
				InputFile2 = argv[i];
				i++;
				break;
			}
			else {
				InputFile1 = argv[i];
			}
		}
		else {
			fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
			return -1;
		}
	}
/* */
	if ( !InputFile1 || !InputFile2 ) {
		fprintf(stderr, "Lack of specified input file; ");
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
	fprintf(stdout, "Usage: %s [options] <input SAC file 1> <input SAC file 2> > <output SAC file>\n", PROG_NAME);
	fprintf(stderr, "       or %s [options] <input SAC file 1> <input SAC file 2> <output SAC file>\n\n", PROG_NAME);
	fprintf(stdout,
		"*** Options ***\n"
		" -v    Report program version\n"
		" -h    Show this usage message\n"
		"\n"
		"This program will concatenate the two input SAC files together.\n"
		"\n"
	);

	return;
}
