//
// Created by cain on 2017/6/4.
//

#include "../util/EGLUtil.h"

typedef struct {
    GLuint program;
    GLint  samplerLoc;
    GLuint textureId;
} UserData;


GLuint createSimplerTexture2D() {
    GLuint textureId;
    GLubyte pixels[4 * 3] = {
            255, 0, 0,
            0, 255, 0,
            0, 0, 255,
            255, 255, 0,
    };

    // 0、设置解包对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 1、创建一个纹理对象
    glGenTextures(1, &textureId);

    // 2、绑定纹理对象
    glBindTexture(GL_TEXTURE_2D, textureId);

    // 3、加载图像数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // 4、将放大缩小过滤模式设置为GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureId;
}


int EGLInit(ESContext *esContext) {

    UserData *userData = (UserData *) esContext->userData;

    ANativeActivity* activity = esContext->activity;
    // 从asset文件夹加载shader
    char* vertex = readAssetFile("vertex_texture.glsl", activity->assetManager);
    char* fragment = readAssetFile("fragment_texture.glsl", activity->assetManager);
    // 加载program
    userData->program = loadProgram(vertex, fragment);
    if (userData->program == 0) {
        return FALSE;
    }

    // 获取sampler 的location
    userData->samplerLoc = glGetUniformLocation(userData->program, "s_texture");

    // 加载texture
    userData->textureId = createSimplerTexture2D();

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return TRUE;
}


void onDraw(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    GLfloat vertices[] = {
            -0.5f,  0.5f, 0.0f,  // Position 0
            0.0f,  0.0f,        // TexCoord 0
            -0.5f, -0.5f, 0.0f,  // Position 1
            0.0f,  1.0f,        // TexCoord 1
            0.5f, -0.5f, 0.0f,  // Position 2
            1.0f,  1.0f,        // TexCoord 2
            0.5f,  0.5f, 0.0f,  // Position 3
            1.0f,  0.0f         // TexCoord 3
    };

    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    // 设置viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // 清空缓冲区
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用之前创建的program
    glUseProgram(userData->program);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), vertices);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), &vertices[3]);

    // 使能对应位置的vertex属性值
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // 绑定texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId);

    // 获取统一变量
    glUniform1i(userData->samplerLoc, 0);

    // 绘制元素
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

/**
 * 关闭窗口
 * @param esContext
 */
void windowShutdown(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    // 删除texture
    glDeleteTextures(1, &userData->textureId);
    // 删除program
    glDeleteProgram ( userData->program );
}


/**
 * OpenGLES应用程序入口
 * @param esContext
 * @return
 */
int onCreate(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    createWindow(esContext, "Hello Texture2D", 320, 240, ES_WINDOW_RGB);
    if (!EGLInit(esContext)) {
        return GL_FALSE;
    }
    // 注册应用程序关闭函数
    registerShutdownFunc(esContext, windowShutdown);
    // 注册应用程序绘制函数
    registerDrawFunc(esContext, onDraw);

    return GL_TRUE;
}
