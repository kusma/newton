#ifndef ADPCM_H
#define ADPCM_H

#ifdef __cplusplus
extern "C" {
#endif

/* gba interface */
void adpcm_gba_init(const void *data);
void adpcm_gba_close();
void adpcm_gba_vblank();
void adpcm_gba_frame();

void adpcm_gba_set_frame(int pos);
int  adpcm_gba_get_frame();

struct adpcm_state
{
	char valprev;        /* Previous output value */
	unsigned char index; /* Index into stepsize table */
};

void adpcm_init(void *adpcm_data);
void adpcm_render(signed char *dst, int len);

#ifdef __cplusplus
}
#endif

#endif /* ADPCM_H */
