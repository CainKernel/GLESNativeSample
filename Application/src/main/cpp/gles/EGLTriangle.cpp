
#include "../util/EGLUtil.h"

const GLint COORDS_PER_VERTEX = 3;
const GLint vertexStride = COORDS_PER_VERTEX * 4;

typedef struct {
    GLuint program;

} UserData;

int EGLInit(ESContext *esContext) {

    UserData *userData = (UserData *) esContext->userData;

    char vertexShader[] =
            "#version 300 es\n"
                    "layout(location = 0) in vec4 a_position;\n"
                    "layout(location = 1) in vec4 a_color;\n"
                    "out vec4 v_color;"
                    "void main()\n"
                    "{\n"
                    "   gl_Position = a_position;\n"
                    "   v_color = a_color;\n"
                    "}\n";

    char fragmentShader[] =
            "#version 300 es\n"
                    "precision mediump float;\n"
                    "in vec4 v_color;\n"
                    "out vec4 fragColor;\n"
                    "void main()\n"
                    "{\n"
//                    "   fragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
                    "   fragColor = v_color;\n"
                    "}\n";

    userData->program = loadProgram(vertexShader, fragmentShader);

//    // 使用assets文件夹下面的着色器程序创建program
//    ANativeActivity* activity = esContext->activity;
//    char* vertex = readAssetFile("triangleVertex.glsl", activity->assetManager);
//    char* fragment = readAssetFile("triangleFragment.glsl", activity->assetManager);
//    userData->program = loadProgram(vertex, fragment);
    if (userData->program == 0) {
        return FALSE;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    return TRUE;
}

/**
 * 绘制方法
 * @param esContext
 */
void onDraw(ESContext *esContext) {
    UserData *userData = (UserData *)esContext->userData;

    GLfloat vertices[] = {
            0.0f,  0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
    };

    GLfloat color[] = {
            1.0f, 0.0f, 0.0f, 1.0f
    };

    GLint vertexCount = sizeof(vertices) / (sizeof(vertices[0]) * COORDS_PER_VERTEX);

    glViewport(0, 0, esContext->width, esContext->height);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(userData->program);

    // 获取a_position位置句柄
    GLint positionHandle = glGetAttribLocation(userData->program, "a_position");
    glVertexAttribPointer(positionHandle, COORDS_PER_VERTEX, GL_FLOAT, GL_FALSE, vertexStride, vertices);
    glEnableVertexAttribArray(positionHandle);

//    // 获取颜色句柄
//    GLint colorHandle = glGetUniformLocation(userData->program, "a_color");
//    glUniform4fv(colorHandle, 1, color);

    // 使用glVertexAttrib4fv指定，不启用顶点属性数组，第一个参数指的是着色器语言中layout绑定的location的值
    glVertexAttrib4fv(1, color);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glDisableVertexAttribArray(positionHandle);
}

/**
 * 关闭窗口
 * @param esContext
 */
void windowShutdown(ESContext *esContext) {
   UserData *userData = (UserData *) esContext->userData;
   glDeleteProgram ( userData->program );
}

/**
 * OpenGLES应用程序入口
 * @param esContext
 * @return
 */
int onCreate(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    createWindow(esContext, "Hello Triangle", 320, 240, ES_WINDOW_RGB);
    if (!EGLInit(esContext)) {
        return GL_FALSE;
    }
    // 注册应用程序关闭函数
    registerShutdownFunc(esContext, windowShutdown);
    // 注册应用程序绘制函数
    registerDrawFunc(esContext, onDraw);

    return GL_TRUE;
}
