#include <imgdebug.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "fass_preproc.h"

#define RAD_EXT   PP_PUP_EXTRADIO
#define RAD_INT   PP_PUP_INTRADIO
#define AREA_MAX    (2*RAD_EXT*2*RAD_EXT)
static int  m_cntpix;
static int* m_posx=NULL;
static int* m_posy=NULL;
static int  m_ccdwidth;
static void regenerar(int xcenter, int ycenter, int ccd_w, int ccd_h);



void imgdbg_init(int ccd_width, int ccd_height)
{
    if(m_posx!=NULL) free(m_posx);
    if(m_posy!=NULL) free(m_posy);
    m_posx=(int*)malloc(sizeof(int)*AREA_MAX);
    m_posy=(int*)malloc(sizeof(int)*AREA_MAX);
    m_ccdwidth=ccd_width;
    regenerar(200, 800, ccd_width, ccd_height);
}


void imgdbg_overlay(unsigned short* d)
{
    for(int i=0; i<m_cntpix; i++)
    {
        int px = m_posx[i];
        int py = m_posy[i];
        d[py*m_ccdwidth + px] = 100;
    }
}


static void regenerar(int xcenter, int ycenter, int ccd_w, int ccd_h)
{
    m_cntpix=0;
    int y0 = ycenter-RAD_EXT;
    int diam = 2*RAD_EXT;
    for(int i=0; i<diam; i++, y0++)
    {
        if(y0<0 || y0>=ccd_h) continue;

        int x0 = xcenter-RAD_EXT;
        for(int j=0; j<diam; j++, x0++)
        {
            if(x0<0 || x0>=ccd_w) continue;
            float xmag = x0-xcenter;
            float ymag = y0-ycenter;
            float rad = sqrt(xmag*xmag + ymag*ymag);
            if(rad>RAD_EXT) continue;
            if(rad<RAD_INT) continue;
            m_posx[m_cntpix] = x0;
            m_posy[m_cntpix] = y0;
            m_cntpix++;
        }
    }
}


