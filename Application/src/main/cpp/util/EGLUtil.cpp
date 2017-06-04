//
// Created by cain on 2017/5/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "EGLUtil.h"

#define PI 3.1415926535897932384626433832795f

typedef struct {
    unsigned char  IdSize,
            MapType,
            ImageType;
    unsigned short PaletteStart,
            PaletteSize;
    unsigned char  PaletteEntryDepth;
    unsigned short X,
            Y,
            Width,
            Height;
    unsigned char  ColorDepth,
            Descriptor;

} TGA_HEADER;

/**
 * 查询可渲染版本类型
 * @param eglDisplay
 * @return
 */
GLint getContextRenderableType(EGLDisplay eglDisplay) {
#ifdef EGL_KHR_create_context
    const char *extensions = eglQueryString(eglDisplay, EGL_EXTENSIONS);
    // 查询字符串并判断是否包含在extension 字符串中
    if (extensions != nullptr && strstr(extensions, "EGL_KHR_create_context")) {
        return EGL_OPENGL_ES3_BIT_KHR;
    }
#endif
    return EGL_OPENGL_ES2_BIT;
}

/**
 * 创建窗口
 * @param esContext 上下文
 * @param title 标题
 * @param width 宽
 * @param height 高
 * @param flags 指定窗口缓冲区特性：
 * EGL_WINDOW_RGB, 基于RGB的颜色缓冲区
 * EGL_WINDOW_ALPHA, 分配目标alpha缓冲区
 * EGL_WINDOW_DEPTH, 分配深度缓冲区
 * EGGL_WINDOW_STENCIL, 分配模板缓冲区
 * EGL_WINDOW_MULTISAMPLE, 分配多重采样缓冲区
 *
 * @return 创建成功与否的标志
 */
GLboolean createWindow(ESContext* esContext,
                       const char* title,
                       GLint width,
                       GLint height,
                       GLuint flags) {
    // 1、初始化EGL
    EGLConfig config;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            3,
            EGL_NONE
    };

    // 获取宽和高
    esContext->width = ANativeWindow_getWidth(esContext->eglNativeWindow);
    esContext->height = ANativeWindow_getHeight(esContext->eglNativeWindow);

    // 打开与EGL显示服务器的连接，默认为EGL_DEFAULT_DISPLAY
    esContext->eglDisplay = eglGetDisplay(esContext->eglNativeDisplay);

    // 如果显示连接不可用，则返回EGL_NO_DISPLAY标志，此时无法创建窗口
    if (esContext->eglDisplay == EGL_NO_DISPLAY) {
        return GL_FALSE;
    }

    // 初始化egl并回传版本号
    if (!eglInitialize(esContext->eglDisplay, &majorVersion, &minorVersion)) {
        return GL_FALSE;
    }

    // 2.确定可用表面配置
    EGLint numConfigs = 0;
    EGLint attribList[] = {
            EGL_RED_SIZE, 5,
            EGL_GREEN_SIZE, 6,
            EGL_BLUE_SIZE, 5,
            EGL_ALPHA_SIZE, (flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
            EGL_DEPTH_SIZE, (flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
            EGL_STENCIL_SIZE, (flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
            EGL_SAMPLE_BUFFERS, (flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
            EGL_RENDERABLE_TYPE, getContextRenderableType(esContext->eglDisplay),
            EGL_NONE
    };

    // 选择Config
    if (!eglChooseConfig(esContext->eglDisplay, attribList, &config, 1, &numConfigs)) {
        return GL_FALSE;
    }
    if (numConfigs < 1) {
        return GL_FALSE;
    }
    // 3.查询EGLConfig
    // Android需要获取EGL_NATIVE_VISUAL_ID的值并将其放如ANativeWindow_setBuffersGeometry函数中
    EGLint format = 0;
    eglGetConfigAttrib(esContext->eglDisplay, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(esContext->eglNativeWindow, 0, 0, format);

    // 4.创建屏幕上的渲染区域：EGL窗口
    // 创建Window surface
    esContext->eglSurface = eglCreateWindowSurface(esContext->eglDisplay, config,
                                                   esContext->eglNativeWindow, nullptr);
    // 判断是否创建成功
    if (esContext->eglSurface == EGL_NO_SURFACE) {
        return GL_FALSE;
    }

    // 5.创建上下文
    esContext->eglContext = eglCreateContext(esContext->eglDisplay, config,
                                             EGL_NO_CONTEXT, contextAttribs);

    // 判断上下文是否创建成功
    if (esContext->eglContext == EGL_NO_CONTEXT) {
        return GL_FALSE;
    }

    // 指定某个EGLContext为当前上下文
    if (!eglMakeCurrent(esContext->eglDisplay, esContext->eglSurface,
                        esContext->eglSurface, esContext->eglContext)) {
        return GL_FALSE;
    }

    return GL_TRUE;
}

void registerDrawFunc(ESContext *esContext, void (drawFunc)(ESContext *)) {
    esContext->drawFunc = drawFunc;
}

void registerShutdownFunc(ESContext *esContext, void (*shutdownFunc)(ESContext *)) {
    esContext->shutdownFunc = shutdownFunc;
}

void registerUpdateFunc(ESContext *esContext, void(*updateFunc)(ESContext *, float)) {
    esContext->updateFunc = updateFunc;
}



/**
 * 读取assets文件夹中的文件
 * @param file assets目录下的文件名(包含路径)
 * @param manager AssetManager管理器
 * @return 返回字符串
 */
char* readAssetFile(const char* file, AAssetManager* manager) {
    AAsset* asset = nullptr;
    char* buffer = nullptr;
    off_t size = -1;
    if (!manager) {
        ALOGE("AssetManager is null!");
        return nullptr;
    }
    // 使用asset文件管理器打开文件
    asset = AAssetManager_open(manager, file, AASSET_MODE_UNKNOWN);
    // 获取文件的长度
    size = AAsset_getLength(asset);
    // 如果文件长度小于1，则表示文件为空，直接返回空字符串
    if (size < 1) {
        ALOGE("file is null!");
        return nullptr;
    }

    // 分配内存，最后需要添加一个字符串结尾地址'\0', (C/C++中char数组与字符串的区别)
    buffer = (char *) malloc(size + 1);
    buffer[size] = '\0';

    // 从文件中读取内容，完成后需要关闭文件
    AAsset_read(asset, buffer, size);
    AAsset_close(asset);
    // 返回字符串
    return buffer;
}

/**
 * 打开文件
 * @param context   上下文
 * @param fileName 文件名
 * @return 返回AAsset
 */
static AAsset* fileOpen(void *context, const char *fileName) {
    AAsset* file = nullptr;
    if (context != nullptr) {
        AAssetManager *manager = (AAssetManager *)context;
        file = AAssetManager_open(manager, fileName, AASSET_MODE_BUFFER);
    }
    return file;
}

/**
 * 关闭文件
 * @param file
 */
static void fileClose(AAsset *file) {
    if (file != nullptr) {
        AAsset_close(file);
    }
}

/**
 * 读取文件
 * @param file
 * @param bytesToRead
 * @param buffer
 * @return
 */
static int fileRead(AAsset *file, int bytesToRead, void *buffer) {
    int bytes = 0;
    if (file != nullptr) {
        bytes = AAsset_read(file, buffer, bytesToRead);
    }
    return bytes;
}

/**
 * 加载一个 8bit，24bit或32bit TGA图片
 * @param context
 * @param fileName
 * @param width
 * @param height
 * @return
 */
char* loadTGA(void* context, const char *fileName, int *width, int *height) {
    char* buffer;
    AAsset* file;
    TGA_HEADER header;
    int bytes;
    file = fileOpen(context, fileName);
    if (file == nullptr) {
        ALOGE("fialed to load: {%s}\n", fileName);
        return nullptr;
    }

    bytes = fileRead(file, sizeof(TGA_HEADER), &header);
    *width = header.Width;
    *height = header.Height;
    if (header.ColorDepth == 8 || header.ColorDepth == 24 || header.ColorDepth == 32) {
        int bytesToRead = sizeof(char) * (*width) * (*height) * header.ColorDepth / 8;

        buffer = (char *)malloc(bytesToRead);
        if (buffer) {
            bytes = fileRead(file, bytesToRead, buffer);
            fileClose(file);
            return buffer;
        }
    }

    return nullptr;
}

GLuint loadShader(GLenum type, const char* shaderSrc) {
    GLuint shader;
    GLint compiled;
    // 创建shader
    shader = glCreateShader(type);
    if (shader == 0) {
        return 0;
    }
    // 加载着色器的源码
    glShaderSource(shader, 1, &shaderSrc, nullptr);

    // 编译源码
    glCompileShader(shader);

    // 检查编译状态
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint  infoLen = 0;
        // 查询日志的长度判断是否有日志产生
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            // 分配一个足以存储日志信息的字符串
            char* infoLog = (char *) malloc(sizeof(char) * infoLen);
            // 检索日志信息
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            ALOGE("Error compiling shader:\n%s\n", infoLog);
            // 使用完成后需要释放字符串分配的内存
            free(infoLog);
        }
        // 删除编译出错的着色器释放内存
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint loadProgram(const char* vertexShader, const char* fragShader) {
    GLuint vertex;
    GLuint fragment;
    GLuint program;
    GLint linked;

    //加载顶点shader
    vertex = loadShader(GL_VERTEX_SHADER, vertexShader);
    if (vertex == 0) {
        return 0;
    }
    // 加载片元着色器
    fragment = loadShader(GL_FRAGMENT_SHADER, fragShader);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return 0;
    }
    // 创建program
    program = glCreateProgram();
    if (program == 0) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return 0;
    }
    // 绑定shader
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    // 链接program程序
    glLinkProgram(program);
    // 检查链接状态
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        // 检查日志信息长度
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            // 分配一个足以存储日志信息的字符串
            char* infoLog = (char *) malloc(sizeof(char) * infoLen);
            // 检索日志信息
            glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
            ALOGE("Error linking program:\n%s\n", infoLog);
            // 使用完成后需要释放字符串分配的内存
            free(infoLog);
        }
        // 删除着色器释放内存
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(program);
        return 0;
    }
    // 删除着色器释放内存
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}


/**
 * 查询活动的统一变量uniform
 * @param program
 */
void checkActiveUniform(const GLuint program) {
    GLint maxLen;
    GLint numUniforms;
    char* uniformName;

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen);

    uniformName = (char *)malloc(sizeof(char) * maxLen);

    for (int i = 0; i < numUniforms; ++i) {
        GLint size;
        GLenum type;
        GLint location;

        glGetActiveUniform(program, i, maxLen, nullptr, &size, &type, uniformName);

        location = glGetUniformLocation(program, uniformName);

        ALOGD("location:", location);

        switch (type) {
            case GL_FLOAT:
                ALOGD("type : GL_FLOAT");
                break;
            case GL_FLOAT_VEC2:
                ALOGD("type : GL_FLOAT_VEC2");
                break;
            case GL_FLOAT_VEC3:
                ALOGD("type : GL_FLOAT_VEC3");
                break;
            case GL_FLOAT_VEC4:
                ALOGD("type : GL_FLOAT_VEC4");
                break;
            case GL_INT:
                ALOGD("type : GL_INT");
                break;
        }
    }
}

/**
 * 缩放
 * @param result
 * @param sx
 * @param sy
 * @param sz
 */
void scale(ESMatrix* result, GLfloat sx, GLfloat sy, GLfloat sz) {
    result->m[0][0] *= sx;
    result->m[0][1] *= sx;
    result->m[0][2] *= sx;
    result->m[0][3] *= sx;

    result->m[1][0] *= sy;
    result->m[1][1] *= sy;
    result->m[1][2] *= sy;
    result->m[1][3] *= sy;

    result->m[2][0] *= sz;
    result->m[2][1] *= sz;
    result->m[2][2] *= sz;
    result->m[2][3] *= sz;
}

/**
 * 平移
 * @param result
 * @param x
 * @param y
 * @param z
 */
void translate(ESMatrix* result, GLfloat x, GLfloat y, GLfloat z) {
    result->m[3][0] += (result->m[0][0] * x + result->m[1][0] * y + result->m[2][0] * z);
    result->m[3][1] += (result->m[0][1] * x + result->m[1][1] * y + result->m[2][1] * z);
    result->m[3][2] += (result->m[0][2] * x + result->m[1][2] * y + result->m[2][2] * z);
    result->m[3][3] += (result->m[0][3] * x + result->m[1][3] * y + result->m[2][3] * z);
}


/**
 * 旋转
 * @param result
 * @param angle
 * @param x
 * @param y
 * @param z
 */
void rotate(ESMatrix* result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat sinAngle, cosAngle;
    GLfloat mag = sqrtf ( x * x + y * y + z * z );

    sinAngle = sinf ( angle * PI / 180.0f );
    cosAngle = cosf ( angle * PI / 180.0f );

    if ( mag > 0.0f )
    {
        GLfloat xx, yy, zz, xy, yz, xz, xs, ys, zs;
        GLfloat oneMinusCos;
        ESMatrix rotMat;

        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        xz = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        oneMinusCos = 1.0f - cosAngle;

        rotMat.m[0][0] = ( oneMinusCos * xx ) + cosAngle;
        rotMat.m[0][1] = ( oneMinusCos * xy ) - zs;
        rotMat.m[0][2] = ( oneMinusCos * xz ) + ys;
        rotMat.m[0][3] = 0.0F;

        rotMat.m[1][0] = ( oneMinusCos * xy ) + zs;
        rotMat.m[1][1] = ( oneMinusCos * yy ) + cosAngle;
        rotMat.m[1][2] = ( oneMinusCos * yz ) - xs;
        rotMat.m[1][3] = 0.0F;

        rotMat.m[2][0] = ( oneMinusCos * xz ) - ys;
        rotMat.m[2][1] = ( oneMinusCos * yz ) + xs;
        rotMat.m[2][2] = ( oneMinusCos * zz ) + cosAngle;
        rotMat.m[2][3] = 0.0F;

        rotMat.m[3][0] = 0.0F;
        rotMat.m[3][1] = 0.0F;
        rotMat.m[3][2] = 0.0F;
        rotMat.m[3][3] = 1.0F;

        matrixMultiply(result, &rotMat, result);
    }
}


/**
 * 视锥体
 * @param result
 * @param left
 * @param top
 * @param right
 * @param bottom
 * @param nearz
 * @param farz
 */
void frustum(ESMatrix* result, float left, float top,
             float right, float bottom, float nearz, float farz) {
    float deltaX = right - left;
    float deltaY = top - bottom;
    float deltaZ = farz - nearz;
    ESMatrix frust;

    if ((nearz <= 0.0f) || (farz <= 0.0f) || (deltaX <= 0.0f)
        || (deltaY <= 0.0f) || (deltaZ <= 0.0f)) {
        return;
    }

    frust.m[0][0] = 2.0f * nearz / deltaX;
    frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

    frust.m[1][1] = 2.0f * nearz / deltaY;
    frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

    frust.m[2][0] = ( right + left ) / deltaX;
    frust.m[2][1] = ( top + bottom ) / deltaY;
    frust.m[2][2] = - ( nearz + farz ) / deltaZ;
    frust.m[2][3] = -1.0f;

    frust.m[3][2] = -2.0f * nearz * farz / deltaZ;
    frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

    matrixMultiply(result, &frust, result);
}

/**
 * 透视
 * @param result
 * @param fovy
 * @param aspect
 * @param nearz
 * @param farz
 */
void perspective(ESMatrix* result, float fovy, float aspect, float nearz, float farz) {
    GLfloat frustW, frustH;
    frustH = tanf ( fovy / 360.0f * PI ) * nearz;
    frustW = frustH * aspect;

    frustum(result, -frustW, frustW, -frustH, frustH, nearz, farz);

}


/**
 * 矩阵相乘
 * @param result
 * @param srcA
 * @param srcB
 */
void matrixMultiply(ESMatrix* result, ESMatrix* srcA, ESMatrix* srcB) {
    ESMatrix tmp;

    for (int i = 0; i < 4; ++i) {
        tmp.m[i][0] =   srcA->m[i][0] * srcB->m[0][0]
                      + srcA->m[i][1] * srcB->m[1][0]
                      + srcA->m[i][2] * srcB->m[2][0]
                      + srcA->m[i][3] * srcB->m[3][0];

        tmp.m[i][1] =   srcA->m[i][0] * srcB->m[0][1]
                      + srcA->m[i][1] * srcB->m[1][1]
                      + srcA->m[i][2] * srcB->m[2][1]
                      + srcA->m[i][3] * srcB->m[3][1];

        tmp.m[i][2] =   srcA->m[i][0] * srcB->m[0][2]
                      + srcA->m[i][1] * srcB->m[1][2]
                      + srcA->m[i][2] * srcB->m[2][2]
                      + srcA->m[i][3] * srcB->m[3][2];

        tmp.m[i][3] =   srcA->m[i][0] * srcB->m[0][3]
                      + srcA->m[i][1] * srcB->m[1][3]
                      + srcA->m[i][2] * srcB->m[2][3]
                      + srcA->m[i][3] * srcB->m[3][3];
    }

    memcpy(result, &tmp, sizeof(ESMatrix));
}

/**
 * 产生一个单位矩阵
 * @param result
 */
void matrixLoadIdentity(ESMatrix* result) {
    memset(result, 0x0, sizeof(ESMatrix));

    result->m[0][0] = 1.0f;
    result->m[1][1] = 1.0f;
    result->m[2][2] = 1.0f;
    result->m[3][3] = 1.0f;
}


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
           GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLuint **indices) {

    int i;
    int j;
    int numParallels = numSlices / 2;
    int numVertices = (numParallels + 1) * (numSlices + 1);
    int numIndices = numParallels * numSlices * 6;
    float angleStep = (2.0f * ES_PI) / ((float) numSlices);

    if ( vertices != NULL ) {
        *vertices = (GLfloat *)malloc(sizeof(GLfloat) * 3 * numVertices);
    }

    if ( normals != NULL ) {
        *normals = (GLfloat *)malloc( sizeof(GLfloat) * 3 * numVertices);
    }

    if ( texCoords != NULL ) {
        *texCoords = (GLfloat *)malloc(sizeof(GLfloat) * 2 * numVertices);
    }

    if ( indices != NULL) {
        *indices = (GLuint *)malloc (sizeof(GLuint) * numIndices);
    }

    for (i = 0; i < numParallels + 1; i++) {
        for (j = 0; j < numSlices + 1; j++) {
            int vertex = (i * (numSlices + 1) + j) * 3;

            if (vertices) {
                (*vertices) [vertex + 0] = radius * sinf(angleStep * (float) i) *
                                             sinf (angleStep * (float) j);
                (*vertices) [vertex + 1] = radius * cosf(angleStep * (float) i);
                (*vertices) [vertex + 2] = radius * sinf(angleStep * (float) i) *
                                             cosf(angleStep * (float)j);
            }

            if (normals) {
                (*normals) [vertex + 0] = (*vertices) [vertex + 0] / radius;
                (*normals) [vertex + 1] = (*vertices) [vertex + 1] / radius;
                (*normals) [vertex + 2] = (*vertices) [vertex + 2] / radius;
            }

            if (texCoords) {
                int texIndex = (i * (numSlices + 1) + j) * 2;
                (*texCoords) [texIndex + 0] = (float) j / (float) numSlices;
                (*texCoords) [texIndex + 1] = (1.0f - (float) i) / (float) (numParallels - 1);
            }
        }
    }

    if (indices != NULL) {
        GLuint *indexBuf = *indices;

        for (i = 0; i < numParallels; i++) {
            for (j = 0; j < numSlices; j++) {
                *indexBuf++  = (GLuint)(i * (numSlices + 1) + j);
                *indexBuf++ = (GLuint)((i + 1) * (numSlices + 1) + j);
                *indexBuf++ = (GLuint)((i + 1) * (numSlices + 1) + (j + 1));

                *indexBuf++ = (GLuint)(i * (numSlices + 1) + j);
                *indexBuf++ = (GLuint)((i + 1) * ( numSlices + 1) + (j + 1));
                *indexBuf++ = (GLuint)(i * (numSlices + 1) + (j + 1));
            }
        }
    }

    return numIndices;
}