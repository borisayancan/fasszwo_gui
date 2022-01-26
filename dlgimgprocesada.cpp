#include "dlgimgprocesada.h"
#include "fass_preproc.h"
#include <QPainter>


DlgImgProcesada::DlgImgProcesada(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    m_image = new QImage(IM_PROC_SZ,IM_PROC_SZ,QImage::Format_Grayscale16);
}


void DlgImgProcesada::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int w=width();
    int h=height();
    int sz = w>h? h:w;

    p.drawImage(QRect(0,0,sz,sz),*m_image);
}
