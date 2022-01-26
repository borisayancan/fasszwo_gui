#ifndef WIMAGEN_H
#define WIMAGEN_H
#include <QWidget>
class Overlay1;


enum T_Zoom
{
    ZOOM_IN,
    ZOOM_OUT
};


class WImagen : public QWidget
{
    Q_OBJECT
public:
    explicit WImagen(int width, int height, Overlay1 *ol,
                     QWidget *parent = nullptr);
    void setZoom(T_Zoom zoom);
    class WSelector* m_selector;
    void setROI(const QRect& rct);
    void setCut(const QPoint& p0, const QPoint& p1);
    void setRecordState(int current, int total);
    void get_cursor(int* x, int* y);

private:
    class QImage* m_img;
    Overlay1* m_overlay1;
    QRect m_rctDraw, m_roi;
    QPoint m_pt1, m_pt2;
    ushort* m_d_img;
    QList<double> m_xZoom;
    QFont m_font1;
    int m_selZoom;
    int m_recCurrent;
    int m_recTotal;
    void paintEvent(QPaintEvent *event);

public:
    ushort* data() { return m_d_img; }
};

#endif // WIMAGEN_H
