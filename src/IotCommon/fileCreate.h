// ------------------------------------------------------------------
// File create
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

int fileCreateMode( char * name, int mode );
int fileCreateRW( char * name );
int fileCreateRWX( char * name );
int fileTest( char * name );

#ifdef __cplusplus
}
#endif

