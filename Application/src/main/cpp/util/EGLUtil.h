//
// Created by cain on 2017/5/23.
//

#ifndef GLESNATIVESAMPLE_EGLUTIL_H
#define GLESNATIVESAMPLE_EGLUTIL_H

#include <jni.h>
#include <cstdlib>
#include <math.h>
#include <time.h>

#include <android/log.h>
#include <android/native_window.h>
#include <android/native_activity.h>
#include <android/configuration.h>
#include <android/asset_manager.h>
#include <android/sensor.h>

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

using namespace std;

#ifndef LOG_TAG
#define LOG_TAG "NDK-LIB"
#endif

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

#define ES_PI  (3.14159265f)

#ifdef __cplusplus

extern "C" {
#endif

#define ES_WINDOW_RGB 0
#define ES_WINDOW_ALPHA 1
#define ES_WINDOW_DEPTH 2
#define ES_WINDOW_STENCIL 4
#define ES_WINDOW_MULTISAMPLE 8

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

typedef struct ESContext ESContext;

/**
 * OpenGLES绘制程序
 * @param esContext
 * @return
 */
extern int onCreate(ESContext *esContext);


typedef struct {
    GLfloat m[4][4];
} ESMatrix;


// 封装的上下文结构体
struct ESContext {
    // 用于存放android_app对象
    ANativeActivity* activity;
    // 暂存的数据，类似onRestoreInstanceState 返回的值
    void* savedState;
    // 用户配置
    AConfiguration* config;
    // 用户数据
    void *userData;
    // 宽
    GLint width;
    // 高
    GLint height;
    // display handle
    EGLNativeDisplayType  eglNativeDisplay;
    // window handle
    EGLNativeWindowType  eglNativeWindow;

    EGLDisplay  eglDisplay;

    EGLContext eglContext;

    EGLSurface eglSurface;

    /// 动画
    // 是否允许动画
    volatile bool animate;

    /// 传感器
    // 传感器管理器
    ASensorManager* sensorManager;
    // 传感器事件队列
    ASensorEventQueue* sensorEventQueue;
    // 加速度传感器
    const ASensor* accelerometerSensor;

    /// 各个生命周期回调方法
    // 绘制回调
    void (*drawFunc)(ESContext *esContext);
    // 关闭回调
    void (*shutdownFunc)(ESContext *esContext);
    // 更新回调
    void (*updateFunc)(ESContext *esContext, float deltaTime);
    // 窗口发生改变时的回调
    void (*onWindowResize)(ESContext *esContext, int width, int height);
    // 窗口需要重绘制时回调
    void (*onRedrawIfNeeded)(ESContext* esContext);
    // 窗口内容发生改变时回调
    void (*onWindowContentChanged)(ESContext* esContext);
    // 焦点发生变化时回调，其中focus表示获取焦点还是失去焦点
    void (*onWindowFocusChanged)(ESContext* esContext, bool focus);
    // 配置发生变化时回调
    void (*onConfigurationChanged)(ESContext* esContext);
    // 低内存时回调
    void (*onLowMemory)(ESContext* esContext);
    // 各个生命周期回调
    void (*onStart)(ESContext* esContext);
    void (*onResume)(ESContext* esContext);
    void (*onSavedInstance)(ESContext* esContext, struct android_app* app);
    void (*onPause)(ESContext* esContext);
    void (*onStop)(ESContext* esContext);
    void (*onDestroy)(ESContext* esContext);
    // 事件回调
    int32_t (*onTouchEvent)(ESContext* esContext, AInputEvent* event);
    int32_t (*onKeyEvent)(ESContext* esContext, AInputEvent* event);
    // 动画回调
    void (*onAnimate)(ESContext* esContext);
};


/// EGL init
GLboolean createWindow(ESContext* esContext, const char* title,
                       GLint width, GLint height, GLuint flags);
/// 注册回调函数
void registerDrawFunc(ESContext *esContext, void (drawFunc)(ESContext *));

void registerShutdownFunc(ESContext *esContext, void (*shutdownFunc)(ESContext *));

void registerUpdateFunc(ESContext *esContext, void(*updateFunc)(ESContext *, float));

void registerWindowChange(ESContext *esContext, void(*onWindowResize)(ESContext*, int, int));

/**
 * 加载TGA图片
 * @param context
 * @param fileName
 * @param width
 * @param height
 * @return
 */
char* loadTGA(void* context, const char *fileName, int *width, int *height);

/**
 * 读取assets文件夹中的文件
 * @param file assets目录下的文件名(包含路径)
 * @param manager AssetManager管理器
 * @return 返回字符串
 */
char* readAssetFile(const char* file, AAssetManager* manager);

/**
 * 打开文件
 * @param context   上下文
 * @param fileName 文件名
 * @return 返回AAsset
 */
static AAsset* fileOpen(void *context, const char *fileName);


/**
 * 关闭文件
 * @param file
 */
static void fileClose(AAsset *file);

/**
 * 读取文件
 * @param file
 * @param bytesToRead
 * @param buffer
 * @return
 */
static int fileRead(AAsset *file, int bytesToRead, void *buffer);

/// shader
GLuint loadShader(GLenum type, const char* shaderSrc);

GLuint loadProgram(const char* vertexShader, const char* fragShader);

/**
 * 查询活动的统一变量uniform
 * @param program
 */
void checkActiveUniform(GLuint program);

/// transform
/**
 * 缩放
 * @param result
 * @param sx
 * @param sy
 * @param sz
 */
void scale(ESMatrix* result, GLfloat sx, GLfloat sy, GLfloat sz);

/**
 * 平移
 * @param result
 * @param x
 * @param y
 * @param z
 */
void translate(ESMatrix* result, GLfloat x, GLfloat y, GLfloat z);

/**
 * 旋转
 * @param result
 * @param angle
 * @param x
 * @param y
 * @param z
 */
void rotate(ESMatrix* result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

/**
 * 视锥体
 * @param result
 * @param left
 * @param top
 * @param right
 * @param bottom
 * @param nearz
 * @param z
 */
void frustum(ESMatrix* result, float left, float top,
             float right, float bottom, float nearz, float farz);

/**
 * 透视
 * @param result
 * @param fovy
 * @param aspect
 * @param nearz
 * @param farz
 */
void perspective(ESMatrix* result, float fovy, float aspect, float nearz, float farz);


/**
 * 矩阵相乘
 * @param result
 * @param srcA
 * @param srcB
 */
void matrixMultiply(ESMatrix* result, ESMatrix* srcA, ESMatrix* srcB);

/**
 * 产生一个单位矩阵
 * @param result
 */
void matrixLoadIdentity(ESMatrix* result);

/**
 * 创建球面
 * @param numSlices
 * @param radius
 * @param vertices
 * @param normals
 * @param texCoords
 * @param indices
 * @return
 */
int sphere(int numSlices, float radius,
           GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLuint **indices);




#ifdef __cplusplus
}
#endif

#endif //GLESNATIVESAMPLE_EGLUTIL_H
