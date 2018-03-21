// -----------------------------------------------------------------
// Black body curve - include file
// -----------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int   kelvin;
    float x;
    float y;
    int   r;
    int   g;
    int   b;
} bb_point;

extern bb_point bb_curve[91];

#ifdef __cplusplus
}
#endif

