#include "fmain.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include "fass_preproc.h"
void debug();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //debug();
    //return 0;
    FMain w;
    w.show();
    return a.exec();
}

void show(ushort** d, int w, int h)
{
    // Dialogo para mostrar
    int dlg_w=w*4, dlg_h=h*4;
    QDialog dlg;
    dlg.resize(dlg_w,dlg_h);

    // Autorango
    int max=256;
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++)
            if(d[y][x]>max) max=d[y][x];

    // Copiar desde 16bits a ARGB32 (sino QImagen::scaled no funca)
    QImage img(w ,h, QImage::Format_ARGB32);
    uint* dst = (uint*)img.bits();
    for(int y=0; y<h; y++)
    {
        for(int x=0; x<w; x++)
        {
            uint gris = ((d[y][x]*255)/max) & 0xff;
            dst[y*w+x]= 0xff000000 | (gris<<16) | ( gris<<8) | (gris<<0);
        }
    }

    // Mostrar
    QPixmap pxm;
    pxm.convertFromImage(img.scaled(dlg_w,dlg_h));
    QLabel container(&dlg);
    container.setGeometry(0,0,dlg_w,dlg_h);
    container.setPixmap(pxm);
    container.show();
    dlg.exec();
}


#include "fass_preproc.h"
void debug()
{
    int w=0;
    int h=0;
    int frames;
    QFile f("../frames.bin");
    if(!f.open(QIODevice::ReadOnly))
        return;

    f.read((char*)&frames, 4);
    f.read((char*)&w, 4);
    f.read((char*)&h, 4);
    if(w<10 || w>2048 || h<10 || h>2048 || frames <1 || frames>2048000)
    {
        qDebug("datos callamperos: %d %d %d", frames, w, h);
        f.close();
        return;
    }

    qDebug("datos: %d %d %d", frames, w, h);
    u16* raw = (u16*)malloc(2*w*h*frames);
    f.read((char*)raw, 2*w*h*frames);
    f.close();

    int bin=5;
    fasspreproc_init(bin,2.9,w,h,NULL);
    u16** img = (u16**)alloc_2d(w,h,sizeof(u16));
    qDebug("comenzando");
    QElapsedTimer tpo;
    tpo.start();
    for(int i=0; i<frames; i++)
    {
        u16** p = fasspreproc_push(&raw[i*w*h]); // ~8.6ms
        //show(p, w/bin, h/bin);
    }
    qDebug("elap:%lld", tpo.elapsed());

    free(raw);
    free_2d((char*)img);
    fasspreproc_close();
}
