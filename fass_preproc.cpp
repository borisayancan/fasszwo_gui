#include "fass_preproc.h"
#include <stdlib.h>
#include <algorithm>
#include <QSemaphore>
#include <QThread>
#include <QMutex>
#include <os_includes.h>


// Variables locales
static bool m_init=false;
static u16** m_img_binned=NULL;             // Imagen pasada por el binning
static int** m_img_integra=NULL;            // Imagen integrada para calculo de centro
static u16** m_img_centered=NULL;           // Imagen centrada de tamaño IM_PROC_SZ
static int*  m_img_lineal=NULL;
static u16*  m_data_raw;
static int m_cnt_integra=0;                 // Contador de integraciones
static int m_raw_w=0, m_raw_h=0, m_bin=1;   // Parametros para la imagen RAW
static double m_sz_pixel=6.5;               // Parametros para la imagen RAW
static int m_width=0, m_height=0;           // Parametros para la imagen binneada
static int m_centro_x=0, m_centro_y;        // Centro actual en la imagen reducida
static int m_bin_npix=0;                    // Numero de pixeles de un bin
static int m_bin_nignora=0;                 // Numero de pixeles a ignorar en un bin. Se ignoran los m_bin_nignora mas y menos
                                            // significativos dentro del bin
static int m_cnt_pupil=0;                   // Numero de pixeles que agrupa la pupila
QMutex m_mutex;
QSemaphore m_semaph;
cb_image_save m_cb_push;
static bool m_calc_run=false;


// Prototipos locales
static void binning(u16* img_raw);
static void update_xy_centro();
static void calcula_img_centrada();
static int  aux_calcula_bin(u16* d);
static int  aux_calcula_bin_stdsort(u16* din);
static void aux_qsort(int *list, int low, int high);
static int  aux_count_pupil();
static void th_bin(void);


/**************************************************************************************
 *
 *  Inicia el modulo de preprocesamiento del sistema FASS
 *  Recibe la imagen RAW desde la camara, ejecuta binning y centra la imagen
 *  en relaciona a la pupila detectada
 *
 *  Potificia Universidad Catolica de Chile
 *  Andres Guesalaga, Boris Ayancan. Marzo 2020
 *
 * ***********************************************************************************/
void fasspreproc_init(int bin, double sz_pixel, int raw_w, int raw_h, cb_image_save fun_save)
{
    m_cb_push = fun_save;

    // Verifica datos validos
    m_init=false;
    if(bin<1 || bin>20 ||
       raw_w<10 || raw_w>2048 ||
       raw_h<10 || raw_h>2048)
        return;

    // m_bin_nignora debe ser menor a (m_bin_npix/2)
    m_bin_npix=bin*bin;
    m_bin_nignora=0;
    if(bin>=2) m_bin_nignora=1;
    if(bin>=5) m_bin_nignora=4;

    // Alocar y asignar parametros
    m_raw_w=raw_w;
    m_raw_h=raw_h;
    m_width = m_raw_w/bin;
    m_height= m_raw_h/bin;
    m_bin=bin;
    m_centro_x=0;
    m_centro_y=0;
    m_cnt_integra=0;
    m_sz_pixel=sz_pixel;
    m_img_binned=(u16**)alloc_2d(m_width, m_height,sizeof(u16));
    m_img_integra=(int**)alloc_2d(m_width, m_height,sizeof(int));
    m_img_centered=(u16**)alloc_2d(IM_PROC_SZ, IM_PROC_SZ,sizeof(u16));
    m_img_lineal=(int*)malloc(m_width * m_height * sizeof(int));
    m_cnt_pupil=aux_count_pupil();
    m_init=true;

    // Correr thread de bineo
    QThread* th = QThread::create(th_bin);
    th->start();
}


/**************************************************************************************
 *
 *  Llamado externamente con cada imagen nueva
 *
 * ***********************************************************************************/
int preproc_lost=0;
u16** fasspreproc_push(u16* data_raw)
{
    // En error no hace nada
    if(!m_init) return m_img_binned;

    // Binea imagen raw, devuelve en m_img_binned
    if(m_semaph.available()<1)
    {
        m_data_raw = data_raw;
        m_semaph.release();
    }
    else preproc_lost++;

    // Calcular centro
    // update_xy_centro();

    // Recentrar

    return m_img_binned;
}


/**************************************************************************************
 *
 *  Llamado externamente para finalizar el modulo y liberar memoria
 *
 * ***********************************************************************************/
void fasspreproc_close()
{
    m_calc_run=false;
    m_init=false;
    sleep_ms(150);
    if(m_img_binned)  free_2d((char*)m_img_binned);
    if(m_img_integra) free_2d((char*)m_img_integra);
    if(m_img_centered)free_2d((char*)m_img_centered);
    if(m_img_lineal)  free(m_img_lineal);
    m_img_binned   = NULL;
    m_img_integra  = NULL;
    m_img_centered = NULL;
    m_img_lineal = NULL;
}


/**************************************************************************************
 *
 *  Privada
 *  Ejecuta bining, descarta pixeles mas y menos significativos
 *
 * ***********************************************************************************/
static void binning(u16* img_raw)
{
    int w=m_width;
    int h=m_height;
    u16 buffer[512];

    // Loop x-y sobre la imagen resultado (binneada)
    for(int y=0; y<h; y++)
    {
        for (int x=0;x<w;x++)
        {
            // Loop sobre el cuadradio a binnear
            for (int i=0;i<m_bin;i++)
            {
              int y1=y*m_bin+i;
              int x1=x*m_bin;
              for(int j=0;j<m_bin;j++)
                  buffer[i*m_bin+j]=img_raw[y1*m_raw_w + x1+j];
            }
            //m_img_binned[y][x]=aux_calcula_bin(buffer);
            m_img_binned[y][x]=aux_calcula_bin_stdsort(buffer);
        }
    }
}


/**************************************************************************************
 *
 *  Privada
 *  Actualiza las coordenadas m_centro_x y m_centro_y que definen el centro
 *  de la pupila
 *
 * ***********************************************************************************/
static void update_xy_centro()
{
#ifdef NO_CENTRAR
    m_centro_x=m_width/2;
    m_centro_y=m_height/2;
    return;
#endif

    // Integrar
    for(int y=0; y<m_height; y++)
        for(int x=0; x<m_width; x++)
            m_img_integra[y][x] += m_img_binned[y][x];
    if(++m_cnt_integra<8) return;

    // Acumulacion ok, calcular
    double aux1=0.0, aux2=0.0;
    int centX[2048], centY[2048];
    memcpy(m_img_lineal,&m_img_integra[0][0],sizeof(int)*m_width*m_height);
    std::sort(m_img_lineal, &m_img_lineal[m_width*m_height], std::less<int>());

    int cut = (m_cnt_pupil*2);
    if(cut>=(m_width*m_height)) cut=m_width*m_height;
    int ref = m_img_lineal[30000];

    for (int j=0;j<m_width;j++)
    {
        centX[j]=0;
        for (int i=0;i<m_height;i++) centX[j]=centX[j]+m_img_integra[i][j];
        centX[j]=int(centX[j]/m_height);
        if (centX[j]>ref)   centX[j]=1;
        else                centX[j]=0;
    }
    for (int i=0;i<m_width;i++)  {
        aux1=aux1+double(centX[i]*i);
        aux2=aux2+double(centX[i]);
    }
    m_centro_x=u16(round(aux1/aux2));

    aux1=0.0; aux2=0.0;
    for (int i=0;i<m_height;i++) {
        centY[i]=0;
        for (int j=0;j<m_width;j++) centY[i]=centY[i]+m_img_integra[i][j];
        centY[i]=int(centY[i]/m_width);
        if (centY[i]>ref)   centY[i]=1;
        else                centY[i]=0;
    }
    for (int j=0;j<m_height;j++)  {
        aux1=aux1+centY[j]*j;
        aux2=aux2+centY[j];
    }
    m_centro_y=u16(round(aux1/aux2));

    // Reiniciar
    m_cnt_integra=0;    
    memset(&m_img_integra[0][0], 0, sizeof(int)*m_width*m_height);
}



static void calcula_img_centrada()
{
    memset(&m_img_centered[0][0],0,sizeof(u16)*IM_PROC_SZ*IM_PROC_SZ);
    int x0 = m_centro_x-(IM_PROC_SZ/2);
    int y0 = m_centro_y-(IM_PROC_SZ/2);
    int x1 = m_centro_x+(IM_PROC_SZ/2);
    int y1 = m_centro_y+(IM_PROC_SZ/2);
    if(x0<0) x0=0;
    if(y0<0) y0=0;
    if(x1>=m_width)  x1=m_width -1;
    if(y1>=m_height) y1=m_height-1;

    int ydst=0;
    for(int y=y0; y<=y1; y++, ydst++)
    {
        if(ydst>=IM_PROC_SZ)
            break;

        int xdst=0;
        for(int x=x0; x<=x1; x++, xdst++)
        {
            if(xdst>=IM_PROC_SZ)
                break;
            m_img_centered[ydst][xdst] = m_img_binned[y][x];
        }
    }
}


/**************************************************************************************
 *
 *  Privada
 *  Calcula el valor para un bin, ejecuta el descarte de mas y menos significativos
 *  cuando corresponde
 *
 * ***********************************************************************************/
static int aux_calcula_bin(u16* d)
{
    int i,j,j1;
    int a,suma;

    for (j=0;j<m_bin_nignora;j++){
        j1=m_bin_npix-j-1;
        for (i=0;i<j1;i++)
            if (d[i]>d[i+1]){
                a=d[i+1];
                d[i+1]=d[i];
                d[i]=a;
            }
    }

    for (j=0;j<m_bin_nignora;j++)
        for (i=m_bin_npix-1;i>0;i--)
            if (d[i]<d[i-1]){
                a=d[i];
                d[i]=d[i-1];
                d[i-1]=a;
            }

    suma=0;
    int cnt=0;
    for (j=m_bin_nignora;j<m_bin_npix-m_bin_nignora;j++)
    {
      suma+=d[j];
      cnt++;
    }

    return(suma/cnt);
}

void countSort(u16 arr[], int n, int exp)
{
    u16 output[512]; // output array
    int i, count[10] = {0};

    // Store count of occurrences in count[]
    for (i = 0; i < n; i++)
        count[ (arr[i]/exp)%10 ]++;

    // Change count[i] so that count[i] now contains actual
    //  position of this digit in output[]
    for (i = 1; i < 10; i++)
        count[i] += count[i - 1];

    // Build the output array
    for (i = n - 1; i >= 0; i--)
    {
        output[count[ (arr[i]/exp)%10 ] - 1] = arr[i];
        count[ (arr[i]/exp)%10 ]--;
    }

    memcpy(arr,output,sizeof(u16)*n);
}


static int aux_calcula_bin_stdsort(u16* din)
{
    int n = m_bin*m_bin;   // largo
    std::sort(din, &din[n], std::less<u16>());

    int suma=0;
    int cnt=0;
    for (int j=m_bin_nignora;j<m_bin_npix-m_bin_nignora;j++)
    {
      suma+=din[j];
      cnt++;
    }

    return(suma/cnt);
}


/**************************************************************************************
 *
 *  Privada
 *  Ordena la lista indicada
 *
 * ***********************************************************************************/
static void aux_qsort(int* list, int low, int high)
{
    int pivot, i, j, temp;

    if (low < m_cnt_pupil){
        if (low < high)
        {
            pivot = low;
            i = low;
            j = high;
            while (i < j)
            {
               if (low >= m_cnt_pupil) break;
               while (list[i] >= list[pivot] && i <= high) i++;
               while (list[j] < list[pivot] && j >= low) j--;

               if (i < j)
               {
                   temp = list[i];
                   list[i] = list[j];
                   list[j] = temp;
               }
            }
            temp = list[j];
            list[j] = list[pivot];
            list[pivot] = temp;
            aux_qsort(list, low, j - 1);
            aux_qsort(list, j + 1, high);
        }
    }
}


/**************************************************************************************
 *
 *  Privada
 *  Calcula el numero de pixeles que forman la pupila
 *
 * ***********************************************************************************/
static int aux_count_pupil()
{
    const double F=8.0;
    const double Zc=400.0;
    const double Rext=0.15;
    const double Rint=0.07;
    const double dpix=m_bin*m_sz_pixel;
    const double Focal=2*F*Rext;
    const double aux=Focal/(dpix*(Zc+Focal));
    const double rext=Rext*aux;
    const double rint=Rint*aux;

    int i,j,sumapupil=0;
    double r;
    for (i=0;i<m_height;i++)
        for (j=0;j<m_width;j++){
            r=sqrt((i-m_height/2)*(i-m_height/2)+(j-m_width/2)*(j-m_width/2));
            if ((r>=rint*1.1)&&(r<=rext/1.1))
                sumapupil++;
        }
    return(sumapupil);
}


/**************************************************************************************
 *
 *  Privada
 *  Aloca una array 2D
 *
 * ***********************************************************************************/
char* alloc_2d(int cols, int rows, int size_element)
{
    char** d = (char**)malloc(rows*sizeof(char*));          // Punteros de acceso
    char* matrix = (char*)malloc(rows*cols*size_element);   // Array 2D
    memset(matrix,0,rows*cols*size_element);

    for(int i=0; i<rows; i++)
        d[i] = &matrix[i*cols*size_element];

    return (char*)d;
}


/**************************************************************************************
 *
 *  Privada
 *  Libera memoria previamente reservada con alloc_2d()
 *
 * ***********************************************************************************/
void  free_2d(char* data)
{
    free(data);
}



void fasspreproc_mutex(bool lock)
{
    if(lock) m_mutex.lock();
    else     m_mutex.unlock();
}


void fasspreproc_get_image(u16 *img)
{
    if(!m_init)
    {
        memset(img,1,sizeof(u16)*IM_PROC_SZ*IM_PROC_SZ);
        return;
    }
    m_mutex.lock();
    memcpy(img,&m_img_centered[0][0],sizeof(u16)*IM_PROC_SZ*IM_PROC_SZ);
    m_mutex.unlock();
}


void th_bin(void)
{
    m_calc_run=true;
    while(m_calc_run)
    {
        if(!m_calc_run) break;
        if(!m_semaph.tryAcquire(1,100)) continue;
        m_mutex.lock();
        binning(m_data_raw);
        update_xy_centro();
        calcula_img_centrada();
        if(m_cb_push) m_cb_push((const char*)&m_img_centered[0][0]);
        m_mutex.unlock();
    }
}
