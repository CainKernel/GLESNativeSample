//
// Created by cain on 2017/6/4.
//

#include "../util/EGLUtil.h"


typedef struct {
    GLuint program;

    GLint  samplerLoc;

    GLuint textureId;

    // 顶点数据
    int numIndices;
    GLfloat  *vertices;
    GLfloat  *normals;
    GLuint  *indices;

} UserData;


GLuint createSimplerTextureCube() {

    GLuint textureId;
    GLubyte cubePixels[6][3] = {
            255, 0, 0,
            0, 255, 0,
            0, 0, 255,
            255, 255, 0,
            255, 0, 255,
            255, 255, 255,
    };


    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);


    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[1]);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[2]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[3]);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[4]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[5]);

    // 放大缩小过滤模式
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureId;
}



int EGLInit(ESContext *esContext) {

    UserData *userData = (UserData *) esContext->userData;

    ANativeActivity* activity = esContext->activity;
    // 从asset文件夹加载shader
    char* vertex = readAssetFile("vertex_texture_cube.glsl", activity->assetManager);
    char* fragment = readAssetFile("fragment_texture_cube.glsl", activity->assetManager);
    // 加载program
    userData->program = loadProgram(vertex, fragment);
    if (userData->program == 0) {
        return FALSE;
    }

    // 获取sampler 的location
    userData->samplerLoc = glGetUniformLocation(userData->program, "s_texture");

    // 加载texture
    userData->textureId = createSimplerTextureCube();

    // 加载vertex 数据
    userData->numIndices = sphere(20, 0.75f, &userData->vertices, &userData->normals,
                                  nullptr, &userData->indices);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return TRUE;
}


void onDraw(ESContext *esContext) {

    UserData *userData = (UserData *)esContext->userData;

    glViewport(0, 0, esContext->width, esContext->height);

    glClear(GL_COLOR_BUFFER_BIT);

    glCullFace(GL_BACK);

    glEnable(GL_CULL_FACE);

    glUseProgram(userData->program);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, userData->vertices);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, userData->normals);

    glEnableVertexAttribArray(0);

    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, userData->textureId);

    glUniform1i(userData->samplerLoc, 0);

    glDrawElements(GL_TRIANGLES, userData->numIndices, GL_UNSIGNED_INT, userData->indices);

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
    glDeleteProgram(userData->program);

    free(userData->vertices);
    free(userData->normals);
    free(userData->indices);
}



/**
 * OpenGLES应用程序入口
 * @param esContext
 * @return
 */
int onCreate(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    createWindow(esContext, "Hello TextureCube", 320, 240, ES_WINDOW_RGB);
    if (!EGLInit(esContext)) {
        return GL_FALSE;
    }
    // 注册应用程序关闭函数
    registerShutdownFunc(esContext, windowShutdown);
    // 注册应用程序绘制函数
    registerDrawFunc(esContext, onDraw);

    return GL_TRUE;
}