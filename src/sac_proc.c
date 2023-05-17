/*
 *
 */

/* */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <float.h>
/* */
#include <sachead.h>
#include <sac_proc.h>

/*  */
static int    read_sac_header( FILE *, struct SAChead * );
static double fetch_sac_time( const struct SAChead * );
static float  dmean_sac_data( float *, const int, const float );
static char  *trim_sac_string( char *, const int );
static void   swap_order_4byte( void * );

/*
 *
 */
int sac_proc_sac_load( const char *filename, struct SAChead *sh, float **seis )
{
	FILE  *fd;
	float *_seis = NULL;
	int    i;
	int    result = -1;

/* Open the sac file */
	if ( (fd = fopen(filename, "rb")) == (FILE *)NULL ) {
		fprintf(stderr, "Error opening %s\n", filename);
		return -1;
	}
/* Read the sac header into a buffer */
	if ( (i = read_sac_header(fd, sh)) < 0 )
		goto end_process;
/* Read the sac data into a buffer */
	if ( (_seis = (float *)malloc((size_t)(sh->npts * sizeof(float)))) == (float *)NULL ) {
		fprintf(stderr, "ERROR! Out of memory for %d float samples\n", sh->npts);
		result = -2;
		goto end_process;
	}
	if ( fread(_seis, sizeof(float), sh->npts, fd) != (size_t)sh->npts ) {
		fprintf(stderr, "Error reading SAC data: %s\n", strerror(errno));
		free(_seis);
		goto end_process;
	}

/* */
	if ( i == 1 )
		for ( i = 0; i < sh->npts; i++ )
			swap_order_4byte( _seis + i );
/* */
	*seis  = _seis;
	result = sizeof(struct SAChead) + sh->npts * sizeof(float);

end_process:
	fclose(fd);
	return result;
}

/**
 * @brief
 *
 * @param sh
 * @param n_sta
 * @param n_chan
 * @param n_net
 * @param n_loc
 * @return struct SAChead*
 */
struct SAChead *sac_proc_scnl_modify(
	struct SAChead *sh, const char *n_sta, const char *n_chan, const char *n_net, const char *n_loc
) {
	int i;

/* Station name */
	if ( n_sta != NULL ) {
		i = strlen(n_sta);
		memcpy(sh->kstnm, n_sta, i >= K_LEN ? K_LEN : i);
		for ( ; i < K_LEN; i++ )
			sh->kstnm[i] = ' ';
	}
/* Channel code */
	if ( n_chan != NULL ) {
		i = strlen(n_chan);
		memcpy(sh->kcmpnm, n_chan, i >= K_LEN ? K_LEN : i);
		for ( ; i < K_LEN; i++ )
			sh->kcmpnm[i] = ' ';
	}
/* Network code */
	if ( n_net != NULL ) {
		i = strlen(n_net);
		memcpy(sh->knetwk, n_net, i >= K_LEN ? K_LEN : i);
		for ( ; i < K_LEN; i++ )
			sh->knetwk[i] = ' ';
	}
/* Location code */
	if ( n_loc != NULL ) {
		i = strlen(n_loc);
		memcpy(sh->khole, n_loc, i >= K_LEN ? K_LEN : i);
		for ( ; i < K_LEN; i++ )
			sh->khole[i] = ' ';
	}

	return sh;
}

/**
 * @brief
 *
 * @param sh
 * @param az
 * @param inc
 * @return struct SAChead*
 */
struct SAChead *sac_proc_az_inc_modfify( struct SAChead *sh, const float az, const float inc )
{
/*
 * Orientation of seismometer - determine the orientation based on the third character
 * of the component name
 */
	switch ( sh->kcmpnm[2] ) {
/* Vertical component */
	case 'Z' :
	case 'z' :
		sh->cmpaz  = az == SACUNDEF ? 0.0 : az;
		sh->cmpinc = inc == SACUNDEF ? 0.0 : inc;
		break;
/* North-south component */
	case 'N' :
	case 'n' :
		sh->cmpaz  = az == SACUNDEF ? 0.0 : az;
		sh->cmpinc = inc == SACUNDEF ? 90.0 : inc;
		break;
/* East-west component */
	case 'E' :
	case 'e' :
		sh->cmpaz  = az == SACUNDEF ? 90.0 : az;
		sh->cmpinc = inc == SACUNDEF ? 90.0 : inc;
		break;
/* Anything else */
	default :
		sh->cmpaz  = az;
		sh->cmpinc = inc;
		break;
	} /* switch */

	return sh;
}

/*
 *
 */
const char *sac_proc_scnl_print( struct SAChead *sh )
{
	static char result[64] = { 0 };

	char sta[K_LEN + 1]  = { 0 };
	char chan[K_LEN + 1] = { 0 };
	char net[K_LEN + 1]  = { 0 };
	char loc[K_LEN + 1]  = { 0 };

/* */
	strncpy(sta, sh->kstnm, K_LEN);
	trim_sac_string( sta, K_LEN );
/* */
	strncpy(chan, sh->kcmpnm, K_LEN);
	trim_sac_string( chan, K_LEN );
/* */
	strncpy(net, sh->knetwk, K_LEN);
	trim_sac_string( net, K_LEN );
/* */
	strncpy(loc, sh->khole, K_LEN);
	trim_sac_string( loc, K_LEN );

	sprintf(result, "%s.%s.%s.%s", sta, chan, net, loc);

	return result;
}

/*
 *
 */
double sac_proc_reftime_fetch( struct SAChead *sh )
{
	return fetch_sac_time( sh );
}

/*
 *
 */
float *sac_proc_data_preprocess( float *seis, const int npts, const float gain_fac )
{
	dmean_sac_data( seis, npts, gain_fac );

	return seis;
}

/**
 * @brief Read the header portion of a SAC file into memory.
 *
 * @param fp Pointer to an open file from which to read
 * @param psh Header pointer of the read-in buffer
 * @return int
 * @returns: 0 on success
 *          1 on success and if byte swapping is needed
 *         -1 on error reading file
 */
static int read_sac_header( FILE *fp, struct SAChead *psh )
{
	int i;
	int filesize;
	int result = 0;
	struct SAChead2 *psh2;

/* Obtain file size */
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	rewind(fp);

/* */
	psh2 = (struct SAChead2 *)psh;
	if ( fread(psh, sizeof(struct SAChead2), 1, fp) != 1 ) {
		fprintf(stderr, "Error reading SAC file: %s!\n", strerror(errno));
		return -1;
	}
/* */
	if ( filesize != (sizeof(struct SAChead) + (psh->npts * sizeof(float))) ) {
		result = 1;
		fprintf(stderr, "WARNING: Swapping is needed! (filesize %d, psh.npts %d)\n", filesize, psh->npts);
		for ( i = 0; i < NUM_FLOAT; i++ )
			swap_order_4byte( psh2->SACfloat + i );
		for ( i = 0; i < MAXINT; i++ )
			swap_order_4byte( psh2->SACint + i );
		if ( filesize != (sizeof(struct SAChead) + (psh->npts * sizeof(float))) ) {
			fprintf(stderr, "ERROR: Swapping is needed again! (filesize %d, psh.npts %d)\n", filesize, psh->npts);
			result = -1;
		}
	}

	return result;
}

/*
 *
 */
static double fetch_sac_time( const struct SAChead *sh )
{
	struct tm tms;
    double result;

    tms.tm_year  = sh->nzyear - 1900;
    tms.tm_mon   = 0;           /* Force the month to January */
    tms.tm_mday  = sh->nzjday;  /* tm_mday is 1 - 31; nzjday is 1 - 366 */
    tms.tm_hour  = sh->nzhour;
    tms.tm_min   = sh->nzmin;
    tms.tm_sec   = sh->nzsec;
    tms.tm_isdst = 0;
    result       = (double)timegm(&tms);
	result      += sh->nzmsec / 1000.0;

    return result;
}

/*
 *
 */
static float dmean_sac_data( float *input, const int npts, const float gain )
{
	int   i, i_head;
	int   gap_count = 0;
	float mean = 0.0;

/* */
	i_head = (int)(npts * 0.1);
	i_head = i_head ? i_head : npts;
	for ( i = 0; i < npts; i++ ) {
		if ( input[i] == SACUNDEF ) {
			input[i] = 0.0;
			if ( i < i_head )
				gap_count++;
			continue;
		}
		input[i] *= gain;
		if ( i < i_head )
			mean += input[i];
	}
	mean /= (float)(i_head - gap_count);
/* */
	for ( i = 0; i < npts; i++ )
		if ( input[i] != 0.0 )
			input[i] -= mean;
		else
			input[i] = mean;

	return mean;
}

/**
 * @brief
 *
 * @param sac_str
 * @param size
 * @return char*
 */
static char *trim_sac_string( char *sac_str, const int size )
{
	char *p = sac_str + size - 1;

/* */
	if ( sac_str != NULL && *sac_str != '\0' ) {
		while ( p >= sac_str && isspace(*p) ) {
			*p-- = '\0';
		}
	}

	return sac_str;
}

/**
 * @brief Do byte swapping on the given 4-byte integer or float.
 *
 * @param data
 */
static void swap_order_4byte( void *data )
{
	uint8_t temp;

	union {
		uint8_t c[4];
	} _data;

	memcpy( &_data, data, sizeof(uint32_t) );
	temp       = _data.c[0];
	_data.c[0] = _data.c[3];
	_data.c[3] = temp;
	temp       = _data.c[1];
	_data.c[1] = _data.c[2];
	_data.c[2] = temp;
	memcpy( data, &_data, sizeof(uint32_t) );

	return;
}
