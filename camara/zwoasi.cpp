#include "zwoasi.h"
#include "ASICamera2.h"
#include "fass_file.h"
#include "fass_preproc.h"
#include <QFile>


ZwoASI::ZwoASI(QObject *parent) :
    QThread(parent)
{
    m_status=CAMSTAT_NC;
    m_run=false;
    m_tpo_fps.start();
    m_frames=0;
    m_integra=false;
    m_longExpCnt=0;
    m_export = (ushort*)malloc(sizeof(ushort)*CAM_WIDTH*CAM_HEIGHT);
    m_longExpImage = (int*)malloc(sizeof(int)*CAM_WIDTH*CAM_HEIGHT);
    memset(&m_camara,0,sizeof(m_camara));
    m_roi.x0=1;
    m_roi.x1=2;
    m_roi.y0=1;
    m_roi.y1=2;
    m_bin=5;
}

bool ZwoASI::open(int id, double exposure)
{
    qDebug("open:%d", id);
    m_status=CAMSTAT_NC;
    if(m_camara.open) close();
    ASI_CAMERA_INFO cam_info;
    ASIGetCameraProperty(&cam_info, id);
    if(cam_info.MaxWidth<100 || cam_info.MaxWidth>2048 ||
            cam_info.MaxHeight<100 || cam_info.MaxHeight>2048)
    {
        return false;
    }

    int r = ASIOpenCamera(id);
    if(r!=ASI_SUCCESS)
    {
        return false;
    }

    r = ASIInitCamera(id);

    int nctrl=0;
    ASIGetNumOfControls(id, &nctrl);
    qDebug("max_size: [%d %d]", cam_info.MaxWidth, cam_info.MaxHeight);
    for(int i=0; i<nctrl; i++)
    {
        ASI_CONTROL_CAPS caps;
        ASIGetControlCaps(id, i, &caps);
        qDebug("cap[%d] %s [%s][%d %d]", i, caps.Name, caps.Description,
               caps.MinValue, caps.MaxValue);
    }

    m_camara.existe=true;
    m_camara.open=true;
    m_camara.id=cam_info.CameraID;
    m_camara.w=cam_info.MaxWidth;
    m_camara.h=cam_info.MaxHeight;
    m_camara.exposure=exposure;
    memcpy(m_camara.name, cam_info.Name, 16);
    m_camara.name[16]=0;

    ASISetControlValue(id,ASI_EXPOSURE,m_camara.exposure*1000, ASI_FALSE);
    m_roi.x0=1;
    m_roi.y0=1;
    m_roi.x1=1096;
    m_roi.y1=1096;
    ASISetROIFormat(id, m_roi.width(), m_roi.heigth(), 1, ASI_IMG_RAW16);
    ASISetStartPos(id,m_roi.x0,m_roi.y0);
    ASISetCameraMode(id, ASI_MODE_NORMAL);
    m_status=CAMSTAT_READY;

    return true;
}


void ZwoASI::close()
{
    m_run=false;
    wait();

    if(m_camara.open)
    {
        ASICloseCamera(m_camara.id);
        m_camara.open=false;
        if(m_export)free(m_export);
        m_export=NULL;
        memset(&m_camara,0,sizeof(m_camara));
        m_status=CAMSTAT_NC;
    }
}


QStringList ZwoASI::availables()
{
    T_ZwoCamara cams[MAX_CAMS];
    ASI_CAMERA_INFO cam;
    int ncams = ASIGetNumOfConnectedCameras();

    QStringList ret;
    for(int i=0; i<ncams; i++)
    {
        if(i>=MAX_CAMS) break;
        ASIGetCameraProperty(&cam, i);
        cams[i].existe=true;
        cams[i].open=false;
        cams[i].id=cam.CameraID;
        cams[i].w=cam.MaxWidth;
        cams[i].h=cam.MaxHeight;
        cams[i].exposure=4;
        memcpy(cams[i].name, cam.Name, 16);
        cams[i].name[16]=0;

        ret << QString().sprintf("%s [%d]", cams[i].name, cams[i].id);
    }

    return ret;
}


void ZwoASI::setExposure(double exp_ms)
{
    if(!m_camara.open) return;
    m_camara.exposure=exp_ms;
    ASISetControlValue(m_camara.id,ASI_EXPOSURE,m_camara.exposure*1000, ASI_FALSE);
}


void ZwoASI::setRoi(const T_CsROI &roi)
{

}


void ZwoASI::capturar_continuo()
{
    if(m_run)  detener();
    if(m_camara.open)
    {
        m_status=CAMSTAT_PREVIEW;
        start();
    }
}


void ZwoASI::detener()
{
    m_run=false;
    wait();
}


float ZwoASI::fps()
{
    static float val=0;
    if(m_tpo_fps.elapsed()<1000)
        return val;
    double t = m_tpo_fps.elapsed()/1000.;
    val = m_frames/t;

    m_frames=0;
    m_tpo_fps.restart();
    return val;
}


void ZwoASI::copy_last(char *d, int negro, float xmul)
{
    if(m_integra && m_longExpCnt>0)
    {
        m_mutex.lock();
        ushort* dst=(ushort*)d;
        for(int i=0; i<m_camara.h; i++)
        {
            int dst_l0 = (m_roi.y0+i-1)*CAM_WIDTH+m_roi.x0-1;
            int src_l0 = i*m_roi.width();

            for(int k=0; k<m_roi.width(); k++)
                dst[dst_l0+k] = int((m_longExpImage[src_l0+k]/m_longExpCnt-negro)*xmul);
        }
        m_mutex.unlock();
    }

    else
    {
        m_mutex.lock();
        ushort* dst=(ushort*)d;
        for(int i=0; i<m_camara.h; i++)
        {
            int dst_l0 = (m_roi.y0+i-1)*CAM_WIDTH+m_roi.x0-1;
            int src_l0 = i*m_roi.width();

            for(int k=0; k<m_roi.width(); k++)
                dst[dst_l0+k] = int((m_export[src_l0+k]-negro)*xmul);
        }
        m_mutex.unlock();
    }
}


void ZwoASI::copy_last_processed(char *d, int negro, float xmul)
{
    ushort src[IM_PROC_SZ*IM_PROC_SZ];
    fasspreproc_get_image(src);

    ushort* dst=(ushort*)d;
    for(int i=0; i<IM_PROC_SZ; i++)
    {
        for(int k=0; k<IM_PROC_SZ; k++)
            dst[i*IM_PROC_SZ+k] = (int(src[i*IM_PROC_SZ+k])-negro) *xmul;
    }
}


void ZwoASI::setIntegration(bool enable)
{
    m_mutex.lock();
    m_integra=enable;
    m_longExpCnt=0;
    memset(m_longExpImage,0,sizeof(int)*CAM_HEIGHT*CAM_WIDTH);
    m_mutex.unlock();
}

int ZwoASI::record_start(const char *filename, const char *comentario)
{
    detener();

    T_FileHeader h;
    if(!m_camara.open) return 0;
    if(m_run)  detener();
    int clen = strlen(comentario);
    if(clen>=(sizeof(h.comment)-1)) clen = (sizeof(h.comment)-1);

    h.img_h=IM_PROC_SZ;
    h.img_w=IM_PROC_SZ;
    memcpy(h.comment, comentario, clen);
    h.comment[clen] = 0;

    int d = fass_alloc(1024*2, h, filename);
    m_status=CAMSTAT_SAVING;
    start();
    return d;
}


bool ZwoASI::record_end()
{
    detener();
    return fass_write2Disc();
}


int ZwoASI::record_frame()
{
    return fass_frameCurrent();
}


void ZwoASI::setGain(int val)
{
    if(!m_camara.open) return;
    ASISetControlValue(m_camara.id,ASI_GAIN,val, ASI_FALSE);
}

void ZwoASI::run()
{
    if(!m_camara.open) return;

    qDebug("run()");
    QElapsedTimer tpo_export;
    tpo_export.start();
    m_run=true;


    u16* d1 =(u16*)malloc(sizeof(u16)*m_camara.w*m_camara.h);
    u16* d2 =(u16*)malloc(sizeof(u16)*m_camara.w*m_camara.h);
    int buffer=0;

    fasspreproc_init(CAM_SOFT_BIN,CAM_SZ_PIX,m_roi.width(),m_roi.heigth(), fass_push);
    ASIStartVideoCapture(m_camara.id);
    while(m_run)
    {

        u16* dst = buffer==0? d1:d2;
        buffer = buffer==0? 1:0;
        ASIGetVideoData(m_camara.id, (unsigned char*)dst,
                        sizeof(ushort)*m_roi.width()*m_roi.heigth(), 1000);
        m_frames++;

        fasspreproc_mutex(true);
        fasspreproc_push(dst);
        fasspreproc_mutex(false);

        if(m_status==CAMSTAT_SAVING)
        {
            //fass_push((const char*)m_camara.data);
        }
        else if(m_status==CAMSTAT_PREVIEW)
        {
            if(m_integra)
            {
                ushort* src = (ushort*)dst;
                if(m_longExpCnt<10000)
                {
                    m_mutex.lock();
                    for(int i=0; i<m_roi.heigth(); i++)
                        for(int k=0; k<m_roi.width(); k++)
                            m_longExpImage[i*m_roi.width()+k] += src[i*m_roi.width()+k];
                    m_longExpCnt++;
                    m_mutex.unlock();
                }
            }
        }

        if(tpo_export.elapsed()>100)
        {
            tpo_export.restart();
            m_mutex.lock();
            memcpy(m_export,dst,sizeof(ushort)*m_camara.w*m_camara.h);
            m_mutex.unlock();
        }
    }

    ASIStopVideoCapture(m_camara.id);
    fasspreproc_close();
    usleep(200000);
    m_status=CAMSTAT_READY;
    free(d1);
    free(d2);

    qDebug("run() saliendo...");
}
