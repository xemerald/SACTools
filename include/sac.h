/*
 *
 */
#pragma once
/* */
#include <sachead.h>
/* */
#define SAC_FILE_NAME_FORMAT  "%s/%s.%s.%s.%s"
#define SAC_MAX_SCNL_LENGTH   64

/* */
int sac_file_load( const char *, struct SAChead *, float ** );
struct SAChead *sac_scnl_modify( struct SAChead *, const char *, const char *, const char *, const char * );
struct SAChead *sac_az_inc_modify( struct SAChead *, const float, const float );
const char *sac_scnl_print( struct SAChead * );
double sac_reftime_fetch( struct SAChead * );
float *sac_data_preprocess( struct SAChead *, float *, const float );
