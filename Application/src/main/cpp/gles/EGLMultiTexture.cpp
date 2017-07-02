#include "../util/EGLUtil.h"

typedef struct {
    GLuint programObject;

    GLint baseMapLoc;
    GLint lightMapLoc;

    GLuint baseMapTexId;
    GLuint lightMapTexId;

} UserData;


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


int init(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    char *vertexStr = readAssetFile("vertex_multitexture.glsl",
                                    esContext->activity->assetManager);
    char *fragmentStr = readAssetFile("fragment_multitexture.glsl",
                                      esContext->activity->assetManager);

    userData->programObject = loadProgram(vertexStr, fragmentStr);

    userData->baseMapLoc = glGetUniformLocation(userData->programObject, "s_baseMap");
    userData->lightMapLoc = glGetUniformLocation(userData->programObject, "s_lightMap");

    userData->baseMapTexId = loadTexture(esContext->activity->assetManager, "basemap.tga");
    userData->lightMapTexId = loadTexture(esContext->activity->assetManager, "lightmap.tga");

    if (userData->baseMapTexId == 0 || userData->lightMapTexId == 0) {
        return FALSE;
    }

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return TRUE;
}

void onDraw(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    // 顶点和texture
    GLfloat vertices[] = {
            -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f
    };
    // 索引
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glViewport(0, 0, esContext->width, esContext->height);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(userData->programObject);
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), vertices);
    glVertexAttribPointer(1, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vertices[3]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->baseMapTexId);
    glUniform1i(userData->baseMapLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, userData->lightMapTexId);
    glUniform1i(userData->lightMapLoc, 1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void shutDown(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    glDeleteTextures(1, &userData->baseMapTexId);
    glDeleteTextures(1, &userData->lightMapTexId);

    glDeleteProgram(userData->programObject);
}

int onCreate(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    createWindow(esContext, "MultiTexture", 320, 240, ES_WINDOW_RGB);
    if (!init(esContext)) {
        ALOGE("Cannot init!");
        return GL_FALSE;
    }
    registerDrawFunc(esContext, onDraw);
    registerShutdownFunc(esContext, shutDown);
    return GL_TRUE;
}
