#ifndef FASS_FILE_H
#define FASS_FILE_H

typedef struct
{
    int img_w;
    int img_h;
    char comment[1024];
} T_FileHeader;


int fass_alloc(int mbytes, T_FileHeader header, const char *file_name);
bool fass_push(const char* image);
bool fass_write2Disc();
int fass_frameCurrent();

#endif // FASS_FILE_H
