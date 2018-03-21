// ------------------------------------------------------------------
// System table helpers
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

int newDbSystemGetInt( char * name );
char * newDbSystemGetString( char * name, char * str );

int newDbSystemSaveIntval( char * name, int intval );
int newDbSystemSaveStringval( char * name, char * stringval );
int newDbSystemSaveVal( char * name, int intval, char * stringval );

#ifdef __cplusplus
}
#endif
