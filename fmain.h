#ifndef FMAIN_H
#define FMAIN_H
#include "ui_fmain.h"
#include "widgets/wimagen.h"


class FMain : public QDialog, public Ui::FMain
{
    Q_OBJECT

public:
    FMain(QWidget *parent = nullptr);
    ~FMain();

private slots:
    void on_btnAbrir_clicked();
    void on_btnPlay_clicked();
    void on_btnStop_clicked();
    void on_btnRec_clicked();
    void on_edtExposure_valueChanged(double arg1);
    void slot_timer_100ms();
    void on_btnZoomMas_clicked();
    void on_btnZoomMenos_clicked();
    void slot_selection(int tipo, const QRect& rct);
    void on_btnBlancoAuto_clicked();
    void on_btnCutX_clicked();
    void on_btnCutY_clicked();
    void on_btnOverlayCfg_clicked();
    void slot_fcut_finish(int);
    void on_btnCerrar_clicked();
    void on_chkIntegra_toggled(bool checked);
    void on_edtGain_valueChanged(int arg1);
    void on_btnImgProc_clicked();

private:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent*);
    void update_ui();
    void exec_zoom(T_Zoom zoom);
    QString m_cam_selected;
    int m_frames2Rec;
    class ZwoASI* m_camara;
    class WImagen* m_widImagen;
    class FCut* m_fCut;
    class MeadeCommander* m_meade;
    class Overlay1* m_overlay1;
    class DlgImgProcesada* m_imgProc;
};
#endif // FMAIN_H
