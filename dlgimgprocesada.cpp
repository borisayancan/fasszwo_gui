#include "dlgimgprocesada.h"
#include "fass_preproc.h"
#include <QEvent>
#include <QPainter>
#include <math.h>


DlgImgProcesada::DlgImgProcesada(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    m_image = new QImage(PP_IMGOUT,PP_IMGOUT,QImage::Format_Grayscale16);
    frmImg->installEventFilter(this);
    frmImg->setMinimumWidth (PP_IMGOUT*2);
    frmImg->setMinimumHeight(PP_IMGOUT*2);
    adjustSize();
    frmImg->resize(PP_IMGOUT*2,PP_IMGOUT*2);
}


bool DlgImgProcesada::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj==frmImg && ev->type()==QEvent::Paint)
    {
        QPainter p(frmImg);
        int w=frmImg->width();
        int h=frmImg->height();
        int sz = w>h? h:w;
        sz -= 4;
        p.drawImage(QRect(2,2,sz,sz),*m_image);

        if(chkTarget->isChecked())
        {
            int center = sz/2;
            p.setPen(Qt::green);
            p.drawLine(center-6,center,center+6,center);
            p.drawLine(center,center-6,center,center+6);

            float ratio =  float(sz)/float(PP_IMGOUT);
            int r = (PP_PUP_EXTRADIO/PP_BIN) * ratio + 0.5;
            int px = r*cos(0*M_PI/180);
            int py = r*sin(0*M_PI/180);
            p.translate(center+1,center+1);
            for(int i=5; i<=360; i+= 5)
            {
                int pxx = r*cos(i*M_PI/180);
                int pyy = r*sin(i*M_PI/180);
                p.drawLine(px,py,pxx,pyy);
                px=pxx;
                py=pyy;
            }
        }

    }
    return false;
}


