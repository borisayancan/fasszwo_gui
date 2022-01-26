#include "fcut.h"
#include "widgets/wcut.h"

FCut::FCut(int w, int h, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    setWindowFlag(Qt::WindowMaximizeButtonHint, true);
    m_imSz = QSize(w,h);
    m_enable = false;
    m_buffer = (int*)malloc(sizeof(int)* (w>h? w:h) );
    m_p0 = QPoint(0,0);
    m_p_width = 0;

    // Widget que muestra el cut
    m_wcut = new WCut(frmCut);
    m_wcut->show();
}


void FCut::setVista(bool enable, bool eje_x,
                    const QPoint& p0, int p_width)
{
    m_enable=enable;
    m_eje_x=eje_x;
    m_p0 = p0;
    m_p_width = p_width;

    if(eje_x)
    {
        if(m_p0.x()<0 || m_p0.x()>=m_imSz.width())
            m_enable = false;

        if((m_p0.x()+m_p_width)>=m_imSz.width() )
            m_p_width = m_imSz.width()-m_p0.x()-1;
    }
    else
    {
        if(m_p0.y()<0 || m_p0.y()>=m_imSz.height())
            m_enable = false;

        if((m_p0.y()+m_p_width)>=m_imSz.height() )
            m_p_width = m_imSz.height()-m_p0.y()-1;
    }
}


void FCut::setData(ushort *d, int ymin, int ymax)
{
    if(!m_enable) return;

    if(m_eje_x)
    {
        int start = m_imSz.width()*m_p0.y()+m_p0.x();
        if(start<0 || (start+m_p_width)>= (m_imSz.width()*m_imSz.height()))
            return;

        ushort* src = &d[start];
        for(int i=0; i<m_p_width; i++)
            m_buffer[i] = src[i];
    }
    else
    {
        int start = m_imSz.width()*m_p0.y()+m_p0.x();
        if(start<0 || (start+m_p_width*m_imSz.width())>= (m_imSz.width()*m_imSz.height()))
            return;

        ushort* src = &d[start];
        for(int i=0; i<m_p_width; i++)
            m_buffer[i] = src[i*m_imSz.width()];
    }

    m_wcut->setData(m_buffer, m_p_width, ymin, ymax);
}


void FCut::resizeEvent(QResizeEvent *)
{
    m_wcut->setGeometry(0,0,frmCut->width(),frmCut->height());
}
