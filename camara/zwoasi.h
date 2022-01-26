#ifndef ZWOASI_H
#define ZWOASI_H
#include <QElapsedTimer>
#include <QThread>
#include <QMutex>


// Estado de este modulo
enum T_CsStatus
{
    CAMSTAT_NC,          // No conectado
    CAMSTAT_READY,       // Listo para play o rec
    CAMSTAT_PREVIEW,     // Play running
    CAMSTAT_INTEGRATE,
    CAMSTAT_SAVING,      // Rec running
};


#define CAM_WIDTH       1936
#define CAM_HEIGHT      1096
#define MAX_CAMS        10
#define CAM_NAME        64
#define CAM_SOFT_BIN    5
#define CAM_SZ_PIX      2.9e-6
struct T_ZwoCamara
{
    bool existe;
    bool open;
    int  id;
    int  w;
    int  h;
    float exposure;         // en msec
    char name[CAM_NAME];
};


// Definicion de un ROI
struct T_CsROI
{
    int x0;
    int y0;
    int x1;
    int y1;
    int width()  { return (this->x1-this->x0+1); }
    int heigth() { return (this->y1-this->y0+1); }
};


class ZwoASI : public QThread
{
    Q_OBJECT

public:
    ZwoASI(QObject* parent);
    bool open(int id, double exposure);
    void close();
    QStringList availables();
    void setExposure(double exp_ms);
    void setRoi(const T_CsROI& roi);
    void capturar_continuo();
    void detener();
    float fps();
    void copy_last(char *d, int negro, float xmul);
    void copy_last_processed(char *d, int negro, float xmul);
    void setIntegration(bool enable);
    int  record_start(const char* filename, const char *comentario);
    bool record_end();
    int record_frame();
    void setGain(int val);

private:
    QElapsedTimer m_tpo_fps;
    int m_frames;
    bool m_run;
    bool m_integra;
    int  m_longExpCnt;
    int  m_bin;
    int* m_longExpImage;
    T_ZwoCamara m_camara;
    T_CsStatus m_status;
    T_CsROI m_roi;
    ushort* m_export;
    QMutex m_mutex;
    void run();

public:
    T_CsStatus status()         { return m_status; }
    bool isOpen()               { return m_camara.open; }
};

#endif // ZWOASI_H
