#include <cplaywidget.h>
#include <mainwindow.h>


#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4
#define BHANU
#define WORKING_420P
#define YUY2_test

extern int g_cs_Index;
#ifdef BHANU
QOpenGLTexture* texName;
GLuint texture;
#endif

#ifdef YUY2_test
GLubyte *Ytex,*UVtex;
GLhandleARB FSHandle,PHandle;
QOpenGLExtension_ARB_shader_objects* myShader = new QOpenGLExtension_ARB_shader_objects();

// fy   = /*1080.0-*/gl_TexCoord[0].y;
const char *FProgram=
      "uniform sampler2DRect Ytex;\n"
      "uniform sampler2DRect UVtex;\n"
      "void main(void) {\n"
      "  float fx, fy, y, u, v, r, g, b;\n"

      "  fx   = gl_TexCoord[0].x;\n"
      "  fy   = 1080.0-gl_TexCoord[0].y;\n"

      "  y = texture2DRect(Ytex,vec2(fx,fy)).a;\n"
      "  u = texture2DRect(UVtex,vec2(fx/2.0,fy)).b;\n"
      "  v = texture2DRect(UVtex,vec2(fx/2.0,fy)).r;\n"

      "  y=1.164*(y-0.0627);\n"
      "  u=u-0.5;\n"
      "  v=v-0.5;\n"
      "  r = y+1.5958*v;\n"
      "  g = y-0.39173*u-0.81290*v;\n"
      "  b = y+2.017*u;\n"
      "  gl_FragColor=vec4(r, g, b, 1.0);\n"

      "}\n";
#endif
extern camera_t* camera;


CPlayWidget::CPlayWidget(QWidget *parent):QOpenGLWidget(parent)
{
    textureUniformY = 0;
    textureUniformU = 0;
    textureUniformV = 0;

    id_y = 0;
    id_u = 0;
    id_v = 0;

    m_pBufYuv420p = NULL;
    m_pVSHader = NULL;
    m_pFSHader = NULL;
    m_pShaderProgram = NULL;
    m_pTextureY = NULL;
    m_pTextureU = NULL;
    m_pTextureV = NULL;
    m_pYuvFile = NULL;
    m_nVideoH = 0;
    m_nVideoW = 0;
}

CPlayWidget::~CPlayWidget()
{

}

void CPlayWidget::initializeGL()
{

    initializeOpenGLFunctions();

#ifdef YUY2_test
    int i;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,1920,0,1080,-1,1);
    glViewport(0,0,1920,1080);
    glClearColor(0,0,0,0);
    glColor3f(1.0,0.84,0.0);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

     myShader->initializeOpenGLFunctions();

    PHandle = myShader->glCreateProgramObjectARB();
    FSHandle= myShader->glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    myShader->glShaderSourceARB(FSHandle,1,&FProgram,NULL);
    myShader->glCompileShaderARB(FSHandle);

    myShader->glGetObjectParameterivARB(FSHandle,GL_OBJECT_COMPILE_STATUS_ARB,&i);

    myShader->glAttachObjectARB(PHandle,FSHandle);
    myShader->glLinkProgramARB(PHandle);

    myShader->glUseProgramObjectARB(PHandle);

#endif
}

void CPlayWidget::resizeGL(int w, int h)
{
    if(h == 0)
    {
        h = 1;
    }
    glViewport(0,0, w,h);
}

 void CPlayWidget::paintGL()
 {
 #ifdef YUY2_test
     int i;
     glClear(0);

     glActiveTexture(GL_TEXTURE1);
     i=myShader->glGetUniformLocationARB(PHandle,"UVtex");
     myShader->glUniform1iARB(i,1);  /* Bind Utex to texture unit 1 -YUY2 working*/
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, 1);
     glEnable(GL_TEXTURE_RECTANGLE_NV);

     glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
     glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV,0,GL_RGBA8, 1920/2/*(camera->width/2)*/, 1080/*(camera->height)*/,0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, UVtex); //UV

     glActiveTexture(GL_TEXTURE0);
     i=myShader->glGetUniformLocationARB(PHandle,"Ytex");
     myShader->glUniform1iARB(i,0);  /* Bind Ytex to texture unit 0 -YUY2 Working*/
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, 2);//id_y);
     glEnable(GL_TEXTURE_RECTANGLE_NV);

     glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
     glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE_ALPHA, 1920, 1080, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,Ytex); //Y


     glBegin(GL_QUADS);
         glTexCoord2i(0,0);
         glVertex2i(0,0);
         glTexCoord2i(1920,0);
         glVertex2i(1920,0);
         glTexCoord2i(1920,1080);
         glVertex2i(1920,1080);
         glTexCoord2i(0,1080);
         glVertex2i(0,1080);
     glEnd();

     glDrawArrays(GL_QUADS, 0, 4);

#endif
    return;
 }

void CPlayWidget::PlayOneFrame(unsigned char *l_ui_arr)

{

    m_nVideoW = camera->width;
    m_nVideoH = camera->height;
    int nLen;

    switch (g_cs_Index)
    {
    case I420:
    case I422:
    case YV12:
         nLen = m_nVideoW*m_nVideoH*3/2;
        break;
    case RGB24:
         nLen = m_nVideoW*m_nVideoH*3;
        break;
    case YUY2:
    case UYVY:
        nLen = m_nVideoW*m_nVideoH*2;
       break;
    case RGB32:
         nLen = m_nVideoW*m_nVideoH*4;
        break;
    default:
         qDebug() << "Unsupported format\n";
    }

    if(NULL == m_pBufYuv420p)
    {
        m_pBufYuv420p = new unsigned char[nLen];
        Ytex=(GLubyte *)malloc(nLen);
        UVtex = (GLubyte *)malloc(nLen);
        qDebug("CPlayWidget::PlayOneFrame new data memory. Len=%d width=%d height=%d\n",
               nLen, m_nVideoW, m_nVideoH);
    }
    m_pBufYuv420p = l_ui_arr;

    Ytex =UVtex = (GLubyte *)m_pBufYuv420p;
    update();
    return;
}
