#ifndef WSELECTOR_H
#define WSELECTOR_H
#include <QWidget>

enum T_Selection
{
    SEL_CANCEL,
    SEL_ROI,
    SEL_CUT_X,
    SEL_CUT_Y
};

class WSelector : public QWidget
{
    Q_OBJECT
public:
    explicit WSelector(QWidget *parent = nullptr);
    void redraw(QPainter* p);
    void setZoomFactor(double x);
    void setSelection(T_Selection s);

signals:
    void selection(int tipo, const QRect& rct);

private:
    bool eventFilter(QObject* o, QEvent* e);
    QPoint m_ptStart;
    double m_xZoom;
    T_Selection m_selection;
};

#endif // WSELECTOR_H
