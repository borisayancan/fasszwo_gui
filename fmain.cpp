#include "fmain.h"
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QWidgetList>
#include <QDebug>
#include <QScrollBar>
#include <QSemaphore>
#include <QDateTime>
#include "widgets/wselector.h"
#include <QKeyEvent>
#include "fcut.h"
#include "meadecommander.h"
#include "overlay1.h"
#include "camara/zwoasi.h"
#include "dlgimgprocesada.h"
#include "dlgaskinfo.h"
#include "fass_file.h"
#include "os_includes.h"


// 400 mt -> 1.8468mm (637px -> 127px)

/********************************************************************************
 *
 *  Formulario principal del programa administrador de camara
 *  para el sistema FASS
 *  Boris Ayanca, Octubre 2019
 *
 * *****************************************************************************/
FMain::FMain(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    setWindowFlag(Qt::WindowMaximizeButtonHint, true);
    setWindowFlag(Qt::WindowMinimizeButtonHint, true);
    m_frames2Rec=-1;

    // Crea interfaz a camara PCO
    m_camara = new ZwoASI(this);
    update_ui();

    // Overlay
    m_overlay1 = new Overlay1(this);
    m_overlay1->hide();

    // Un widget para la imagen
    scrArea->widget()->resize(CAM_WIDTH, CAM_HEIGHT);
    m_widImagen = new WImagen(CAM_WIDTH, CAM_HEIGHT, m_overlay1, scrArea->widget());
    m_widImagen->show();
    connect(m_widImagen->m_selector, &WSelector::selection, this, &FMain::slot_selection);

    // Formulario para mostrar cortes
    m_fCut = new FCut(CAM_WIDTH, CAM_HEIGHT, this);
    m_fCut->hide();
    connect(m_fCut, &FCut::finished, this, &FMain::slot_fcut_finish);

    // Comandos al telescopio MEADE
    m_meade = new MeadeCommander(this);
    m_meade->hide();
    connect(btnMeade,&QPushButton::clicked,m_meade,&MeadeCommander::show);

    // Ventanita para imagen procesada
    m_imgProc = new DlgImgProcesada(this);
    m_imgProc->show();

    // Timer de tareas
    QTimer* t = new QTimer(this);
    connect(t,&QTimer::timeout, this, &FMain::slot_timer_100ms);
    t->start(100);

    // Zoom inicial
    exec_zoom(ZOOM_OUT);
}


/********************************************************************************
 *
 *  Destructor de clase
 *
 * *****************************************************************************/
FMain::~FMain()
{
    m_camara->detener();
    m_camara->wait();
    m_camara->close();
}


/********************************************************************************
 *
 *  Busca camaras conectadas, pregunta cual desea conectar y conecta
 *
 * *****************************************************************************/
void FMain::on_btnAbrir_clicked()
{
    // Pregunta camaras conectadas e inicia menu de seleccion
    QStringList camaras = m_camara->availables();
    QFont negrita = font(); negrita.setBold(true);
    QMenu menu("Camaras disponibles");
    menu.addAction("Refrescar")->setFont(negrita);

    // Agrega items a menu de seleccion
    if(camaras.isEmpty()) menu.addAction("No hay camaras")->setData(QVariant(-1));
    for(int i=0; i<camaras.count(); i++) menu.addAction(camaras.at(i))->setData(QVariant(int(i)));

    // Selecciona camara
    QPoint btn_pos = btnAbrir->parentWidget()->mapToGlobal(btnAbrir->pos());
    QAction* select = menu.exec(QPoint(btn_pos.x()+btnAbrir->width(), btn_pos.y()));
    bool sel_ok=false;
    int sel_d = -1;
    if(select==nullptr) return;
    sel_d = select->data().toInt(&sel_ok);

    // Selecciona refrescar
    if(select->text()=="Refrescar")
    {
        QTimer::singleShot(0,this,&FMain::on_btnAbrir_clicked);
        return;
    }

    // Selecciona camara valida
    if(sel_ok && sel_d>=0)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if(!m_camara->open(sel_d, edtExposure->value()))
            QMessageBox::critical(this,"Error", "No se pudo iniciar la camara seleccionada    ");
        else
        {
            m_cam_selected = select->text();
            on_btnPlay_clicked();
            on_edtGain_valueChanged(edtGain->value());
            sleep_ms(250);
            on_btnBlancoAuto_clicked();
            edtExposure->setFocus();

        }
        QApplication::restoreOverrideCursor();
    }

    update_ui();
}


/********************************************************************************
 *
 *  Actualiza la UI de acuerdo al estado del software
 *
 * *****************************************************************************/
void FMain::update_ui()
{
    T_CsStatus cs = m_camara->status();
    btnAbrir->setEnabled(cs==CAMSTAT_NC);
    btnCerrar->setEnabled(cs!=CAMSTAT_NC);
    btnPlay->setEnabled(cs==CAMSTAT_READY);
    btnStop->setEnabled(cs==CAMSTAT_PREVIEW || cs==CAMSTAT_SAVING);
    btnRec->setEnabled(cs==CAMSTAT_READY);
    cmbBin->setEnabled(cs==CAMSTAT_NC);
    chkIntegra->setEnabled(cs==CAMSTAT_PREVIEW);
}


/********************************************************************************
 *
 *  Ejecuta un zoom hacia adentro o hacia afuera
 *
 * *****************************************************************************/
void FMain::exec_zoom(T_Zoom zoom)
{
    double sx=0.5;
    double sy=0.5;

    // Guardar proporicion actual
    if(scrArea->horizontalScrollBar()->maximum()>0)
        sx = double(scrArea->horizontalScrollBar()->value())/scrArea->horizontalScrollBar()->maximum();
    if(scrArea->verticalScrollBar()->maximum()>0)
        sy = double(scrArea->verticalScrollBar()->value())/scrArea->verticalScrollBar()->maximum();

    m_widImagen->setZoom(zoom);

    // Reestabecer proporcion
    if(scrArea->horizontalScrollBar()->maximum()>0)
         scrArea->horizontalScrollBar()->setValue(scrArea->horizontalScrollBar()->maximum()*sx);
    if(scrArea->verticalScrollBar()->maximum()>0)
        scrArea->verticalScrollBar()->setValue(scrArea->verticalScrollBar()->maximum()*sy);
}


/********************************************************************************
 *
 *  Play: comienza una previsualizacion
 *
 * *****************************************************************************/
void FMain::on_btnPlay_clicked()
{
    m_camara->capturar_continuo();
    memset(m_widImagen->data(),0x0,sizeof(ushort)*CAM_WIDTH*CAM_HEIGHT);
    update_ui();
}


/********************************************************************************
 *
 *  Stop: Detiene actual preview o grabacion
 *
 * *****************************************************************************/
void FMain::on_btnStop_clicked()
{
    m_camara->detener();
    if(m_frames2Rec>=0)
    {
        m_camara->record_end();
        m_frames2Rec=-1;
    }
    update_ui();
}


/********************************************************************************
 *
 *  Rec: comienza un nuevo registro de frames
 *
 * *****************************************************************************/
void FMain::on_btnRec_clicked()
{
    DlgAskInfo info(edtExposure->value(), edtGain->value(),
                    m_cam_selected.toUtf8().constData(), this);
    if(!info.exec())
        return;

    T_FileHeader h;
    QString estrella;
    QString coment = info.getInfo(sizeof(h.comment)-1, &estrella);
    if(estrella.length()<1) estrella = "no_estrella";
    QString fecha_file = estrella + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    fecha_file += ".fass";

    m_frames2Rec = m_camara->record_start(fecha_file.toUtf8().constData(), coment.toUtf8().constData());
}


/********************************************************************************
 *
 *  Llamado cuando se ha modificado el tiempo de exposicion
 *
 * *****************************************************************************/
void FMain::on_edtExposure_valueChanged(double arg1)
{
    m_camara->setExposure(arg1);
}


/********************************************************************************
 *
 *  Timer de refresco a 100ms
 *
 * *****************************************************************************/
void FMain::slot_timer_100ms()
{
    char info[256];
    sprintf(info,"FPS:%.1f", m_camara->fps());
    txtFPS->setText(info);

    //extern int mean;
    //qDebug("mean:%d", mean);

    info[0]=0;
    int x, y;
    m_widImagen->get_cursor(&x, &y);
    if(x>=0 && y>=0) sprintf(info,"CUR: %d %d", x, y);
    txtCursor->setText(info);

    float yrange = edtBlanco->value()-edtNegro->value();
    float xwhite = 65536./yrange;
    m_camara->copy_last((char*)m_widImagen->data(), edtNegro->value(), xwhite);
    m_fCut->setData(m_widImagen->data(), edtNegro->value(), edtBlanco->value());
    m_camara->copy_last_processed((char*)m_imgProc->m_image->bits(), edtNegro->value(), xwhite);
    m_widImagen->update();
    m_imgProc->update();

    m_widImagen->setRecordState(m_camara->record_frame(), m_frames2Rec);
    update_ui();
}


/********************************************************************************
 *
 *  Llamado cuando se cambia de tamaño el formulario prinicipal
 *
 * *****************************************************************************/
void FMain::resizeEvent(QResizeEvent *)
{
    m_widImagen->setGeometry(0,0,scrArea->widget()->width(),scrArea->widget()->height());
}


/********************************************************************************
 *
 *  Llamado cuando se presiona un tecla en el formulario principal
 *
 * *****************************************************************************/
void FMain::keyPressEvent(QKeyEvent *k)
{
    if(k->key()==Qt::Key_Escape)
    {
        scrArea->setCursor(Qt::ArrowCursor);
        m_widImagen->m_selector->setSelection(SEL_CANCEL);
    }
    else if(k->key()==Qt::Key_F1)
    {
    }
}


/********************************************************************************
 *
 *  Ejecuta zoo para acercar
 *
 * *****************************************************************************/
void FMain::on_btnZoomMas_clicked()
{
    exec_zoom(ZOOM_IN);
}


/********************************************************************************
 *
 *  Ejecuta zoo para alejar
 *
 * *****************************************************************************/
void FMain::on_btnZoomMenos_clicked()
{
    exec_zoom(ZOOM_OUT);
}


/********************************************************************************
 *
 *  Llamado cuando ha finalizado una seleccion de ROI o una seleccion
 *  de area de corte
 *
 * *****************************************************************************/
void FMain::slot_selection(int tipo, const QRect &r)
{
    if(tipo==SEL_ROI)
    {

    }
    else if(tipo==SEL_CUT_X )
    {
        QPoint p1(r.x(),r.y());
        QPoint p2(r.x()+r.width(),r.y());
        QPoint ptmp = p1;
        if(p2.x()<p1.x())
        {
            p1=p2;
            p2=ptmp;
        }

        m_widImagen->setCut(p1,p2);
        m_fCut->setVista(true,true,p1,r.width());
        m_fCut->show();
    }
    else if(tipo==SEL_CUT_Y)
    {
        QPoint p1(r.x(),r.y());
        QPoint p2(r.x(),r.y()+r.height());
        QPoint ptmp = p1;
        if(p2.y()<p1.y())
        {
            p1=p2;
            p2=ptmp;
        }
        m_widImagen->setCut(p1,p2);
        m_fCut->setVista(true,false,p1,r.height());
        m_fCut->show();
    }

    scrArea->setCursor(Qt::ArrowCursor);
    m_widImagen->m_selector->setSelection(SEL_CANCEL);
}


/********************************************************************************
 *
 *  Calcula los niveles de negro y blanco en base a la captura actual
 *
 * *****************************************************************************/
void FMain::on_btnBlancoAuto_clicked()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);
    ushort* dtmp = (ushort*)malloc(sizeof(ushort)*CAM_WIDTH*CAM_HEIGHT);
    memset(dtmp,0,sizeof(ushort)*CAM_WIDTH*CAM_HEIGHT);
    m_camara->copy_last((char*)dtmp,0,1);

    // Ordenar pixeles validos
    int npix = 0;
    ushort* d = (ushort*)malloc(sizeof(ushort)*CAM_WIDTH*CAM_HEIGHT);
    for(int i=0; i<CAM_WIDTH*CAM_HEIGHT; i++)
    {
        if(dtmp[i]==0) continue;
        d[npix++] = dtmp[i];
    }
    std::sort(d, &d[npix]);

    // los 100 mas brillantes
    int white = 15000;
    if(npix>70)
    {
        white=0;
        for(int i=(npix-51); i<(npix-1); i++)
            white += d[i];
    }
    white /= 50;

    // los 50 mas negros
    int negro=0;
    if(npix>70)
    {
        for(int i=0; i<50; i++)
            negro+=d[i];
    }
    negro /= 50;

    if(negro>white)
    {
        negro=0;
        white=65535;
    }
    edtBlanco->setValue(white);
    edtNegro->setValue(negro);
    free(d);
    free(dtmp);
    QApplication::restoreOverrideCursor();
}


void FMain::on_btnCutX_clicked()
{
    scrArea->setCursor(Qt::CrossCursor);
    m_widImagen->m_selector->setSelection(SEL_CUT_X);
}


void FMain::on_btnCutY_clicked()
{
    scrArea->setCursor(Qt::CrossCursor);
    m_widImagen->m_selector->setSelection(SEL_CUT_Y);
}

void FMain::on_btnOverlayCfg_clicked()
{
    m_overlay1->show();
}


void FMain::slot_fcut_finish(int)
{
    m_widImagen->setCut(QPoint(0,0), QPoint(0,0));
}



void FMain::on_btnCerrar_clicked()
{
    m_camara->close();
    //memset(m_widImagen->data(),0xff,sizeof(ushort)*CAM_WIDTH*CAM_HEIGHT);
}


void FMain::on_chkIntegra_toggled(bool checked)
{
    m_camara->setIntegration(checked);
}

void FMain::on_edtGain_valueChanged(int arg1)
{
    m_camara->setGain(arg1);
}

void FMain::on_btnImgProc_clicked()
{
    m_imgProc->show();
}
