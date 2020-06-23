#ifndef FCUT_H
#define FCUT_H
#include "ui_fcut.h"


class FCut : public QDialog, private Ui::FCut
{
    Q_OBJECT

public:
    explicit FCut(int w, int h, QWidget *parent = nullptr);
    void setVista(bool enable, bool eje_x,
                  const QPoint &p0, int p_width);
    void setData(ushort* d, int ymin, int ymax);

private:
    void resizeEvent(QResizeEvent *);
    QSize m_imSz;
    class WCut* m_wcut;
    QPoint m_p0;
    int* m_buffer;
    bool m_enable;
    bool m_eje_x;
    int m_p_width;
};

#endif // FCUT_H
