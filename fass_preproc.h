#ifndef FASS_PREPROC_H
#define FASS_PREPROC_H
typedef unsigned short u16;
typedef bool(*cb_image_save)(const char* img);


#define PP_PUP_EXTDIAM      236                     // 59 pix final -> 236/bin4
#define PP_PUP_EXTRADIO     (PP_PUP_EXTDIAM/2)
#define PP_PUP_INTRADIO     ((PP_PUP_EXTRADIO*7)/15)
#define PP_IMGOUT           90
#define PP_BIN              4


void fasspreproc_init(int bin, double sz_pixel, int raw_w, int raw_h, cb_image_save fun_save);
u16 **fasspreproc_push(u16* data_raw);
void fasspreproc_close();
void fasspreproc_mutex(bool lock);
void fasspreproc_get_image(u16* img);
char* alloc_2d(int cols, int rows, int size_element);
void  free_2d(char *data);


#endif // FASS_PREPROC_H
