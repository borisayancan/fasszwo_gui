#include "fass_file.h"
#include <stdlib.h>
#include <QtGlobal>
#include <QFile>
static char* m_data=nullptr;
static qint64 m_szImage=0;
static int m_maxImages=0;
static int m_numImages=0;
static QString m_fileName;



int fass_alloc(int mb, T_FileHeader header, const char* file_name)
{
    m_fileName=file_name;
    qint64 mbytes = mb;
    qint64 len = mbytes*1024*1024-sizeof(header);
    m_szImage = header.img_w*header.img_h*2;
    if(m_data!=nullptr) free(m_data);
    m_data = (char*)malloc(mbytes*1024*1024);

    m_numImages=0;
    if(m_data==nullptr)
    {
        m_maxImages=0;
        m_szImage=1;
        return 0;
    }

    header.comment[1023]=0;
    memcpy(m_data,&header,sizeof(header));
    m_maxImages = len/m_szImage;
    return m_maxImages;
}


bool fass_push(const char *image)
{
    if(m_numImages>=m_maxImages ||
            m_data==nullptr) return false;

    qint64 ptr = sizeof(T_FileHeader)+m_numImages*m_szImage;
    memcpy(&m_data[ptr], image, m_szImage);
    m_numImages++;
    return true;
}


bool fass_write2Disc()
{
    bool res = false;
    QFile f(m_fileName);
    if(m_data!=nullptr && f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qint64 nwr = sizeof(T_FileHeader)+m_numImages*m_szImage;
        if(f.write(m_data,nwr)==nwr) res=true;
        f.close();
    }

    free(m_data);
    m_data=nullptr;
    m_numImages=0;
    return res;
}

int fass_frameCurrent()
{
    return m_numImages;
}
