//
// Created by cain on 2017/7/2.
//
#include "../util/EGLUtil.h"

typedef struct {
    GLuint programObject;

    GLint uMVPLoc;
    GLint uMVLoc;
    GLint uEyePosLoc;

    GLint uFogColorLoc;
    GLint uFogMaxDistLoc;
    GLint uFogMinDistLoc;
    GLint baseMapLoc;
    GLuint baseMapTexId;


} UserData;

// u_matViewProjection 的value值
GLfloat mvpMatrix[] = {
        1.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
};
// u_matView 的value值
GLfloat  mvMatrix[] = {
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f
};

// u_eyePos 的value值
GLfloat epVector[]= {
        0.5f, 0.5f, 0.5f, 1.0f
};

// u_fogColor 的value值
GLfloat fColorVector[] = {
      0.4f, 0.4f, 0.8f, 1.0f,
      0.2f, 0.2f, 0.2f, 1.0f,
      0.1f, 0.1f, 0.1f, 1.0f
};

//

// 顶点和texture
GLfloat vertices[] = {
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f
};
// 索引
GLushort indices[] = {0, 1, 2, 0, 2, 3};



/**
 * 加载Texture
 * @param ioContext
 * @param fileName
 * @return
 */
GLuint loadTexture(void *ioContext, char *fileName) {

    int width, height;
    char *buffer = loadTGA(ioContext, fileName, &width, &height);
    GLuint textureId;

    if (buffer == NULL) {
        ALOGE("Error loading (%s) image.\n", fileName);
        return 0;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    free(buffer);

    return textureId;
}

/**
 * 初始化
 * @param esContext
 * @return
 */
int init(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    char *vertexStr = readAssetFile("vertex_linear_fog.glsl",
                                    esContext->activity->assetManager);
    char *fragmentStr = readAssetFile("fragment_linear_fog.glsl",
                                      esContext->activity->assetManager);

    userData->programObject = loadProgram(vertexStr, fragmentStr);

    // vertex中的uniform
    userData->uMVPLoc = glGetUniformLocation(userData->programObject, "u_matViewProjection");
    userData->uMVLoc = glGetUniformLocation(userData->programObject, "u_matView");
    userData->uEyePosLoc = glGetUniformLocation(userData->programObject, "u_eyePos");

    // fragment中的uniform
    userData->uFogColorLoc = glGetUniformLocation(userData->programObject, "u_fogColor");
    userData->uFogMaxDistLoc = glGetUniformLocation(userData->programObject, "u_fogMaxDist");
    userData->uFogMinDistLoc = glGetUniformLocation(userData->programObject, "u_fogMinDist");
    userData->baseMapLoc = glGetUniformLocation(userData->programObject, "s_baseMap");

    // 加载texture
    userData->baseMapTexId = loadTexture(esContext->activity->assetManager, "lightmap.tga");
    if (userData->baseMapTexId == 0) {
        return FALSE;
    }

    // 初始化vertex 的uniform
    glUniformMatrix4fv(userData->uMVPLoc, 1, GL_FALSE, mvpMatrix);
    glUniformMatrix4fv(userData->uMVLoc, 1, GL_FALSE, mvMatrix);
    glUniform4fv(userData->uEyePosLoc, 1, epVector);

    // 初始化fragment的uniform
    glUniform4fv(userData->uFogColorLoc, 3, fColorVector);
    glUniform1f(userData->uFogMaxDistLoc, 5.0f);
    glUniform1f(userData->uFogMinDistLoc, 1.0f);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    return TRUE;
}

void onDraw(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    glViewport(0, 0, esContext->width, esContext->height);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(userData->programObject);

    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), vertices);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vertices[3]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->baseMapTexId);
    glUniform1i(userData->baseMapLoc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void shutDown(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    glDeleteTextures(1, &userData->baseMapTexId);
    glDeleteProgram(userData->programObject);
}

int onCreate(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    createWindow(esContext, "LinearFog", 320, 240, ES_WINDOW_RGB);
    if (!init(esContext)) {
        ALOGE("Cannot init!");
        return GL_FALSE;
    }
    registerDrawFunc(esContext, onDraw);
    registerShutdownFunc(esContext, shutDown);
    return GL_TRUE;
}