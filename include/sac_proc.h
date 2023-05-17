/*
 *
 */
#pragma once
/* */
#include <sachead.h>

#define SAC_FILE_NAME_FORMAT  "%s/%s.%s.%s.%s"

/* */
int sac_proc_sac_load( const char *, struct SAChead *, float ** );
struct SAChead *sac_proc_scnl_modify( struct SAChead *, const char *, const char *, const char *, const char * );
struct SAChead *sac_proc_az_inc_modfify( struct SAChead *, const float, const float );
const char *sac_proc_scnl_print( struct SAChead * );
double sac_proc_reftime_fetch( struct SAChead * );
float *sac_proc_data_preprocess( float *, const int, const float );
