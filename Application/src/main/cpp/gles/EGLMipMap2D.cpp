//
// Created by cain on 2017/6/4.
//

#include "../util/EGLUtil.h"

typedef struct {
    GLuint program;
    GLint samplerLoc;
    GLint offsetLoc;
    GLuint textureId;
} UserData;


GLboolean mipMap2D(GLubyte *src, GLubyte **dst,
                   int srcWidth, int srcHeight,
                   int *dstWidth, int *dstHeight) {
    int x, y;
    int texelSize = 3;
    *dstWidth = srcWidth / 2;
    if (*dstWidth <= 0) {
        *dstWidth = 1;
    }

    *dstHeight = srcHeight / 2;

    if (*dstHeight <= 0) {
        *dstHeight = 1;
    }

    *dst = (GLubyte *)malloc(sizeof(GLubyte) * (*dstWidth) * (*dstHeight));

    if (*dst == nullptr) {
        return GL_FALSE;
    }

    for (y = 0; y < *dstHeight; ++y) {
        for (x = 0; x < *dstWidth; ++x) {
            int srcIndex[4];
            float r = 0.0f, g = 0.0f, b = 0.0f;
            int sample;
            srcIndex[0] = (y * 2 * srcWidth + (x * 2)) * texelSize;
            srcIndex[1] = (y * 2 * srcWidth + (x * 2 + 1)) * texelSize;
            srcIndex[2] = ((y * 2 + 1) * srcWidth + x * 2) * texelSize;
            srcIndex[2] = ((y * 2 + 1) * srcWidth + x * 2 + 1) * texelSize;

            for (sample = 0; sample < 4; sample++) {
                r += src[srcIndex[sample]];
                g += src[srcIndex[sample] + 1];
                b += src[srcIndex[sample] + 2];
            }

            r /= 4.0;
            g /= 4.0;
            b /= 4.0;

            (*dst)[(y * (*dstWidth) + x) * texelSize] = (GLubyte) r;
            (*dst)[(y * (*dstWidth) + x) * texelSize + 1] = (GLubyte) g;
            (*dst)[(y * (*dstWidth) + x) * texelSize + 2] = (GLubyte) b;
        }
    }

    return GL_TRUE;
}

GLubyte *checkImage(int width, int height, int checkSize) {

    GLubyte *pixels = (GLubyte *)malloc((size_t) (width * height * 3));
    if (pixels == nullptr) {
        return nullptr;
    }
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            GLubyte rColor = 0;
            GLubyte bColor = 0;

            if ((x / checkSize) % 2 == 0) {
                rColor = 255 * ((y / checkSize) % 2);
                bColor = 255 * (1 - (y /checkSize) %2);
            } else {
                bColor = 255 * ((y / checkSize) % 2);
                rColor = 255 * (1 - (y / checkSize) % 2);
            }

            pixels[(y * width + x) * 3] = rColor;
            pixels[(y * width + x) * 3 + 1] = 0;
            pixels[(y * width + x) * 3 + 2] = bColor;
        }
    }

    return pixels;
}

GLuint createMipMappedTexture2D() {
    GLuint textureId;
    int width = 256;
    int height = 256;
    int level;
    GLubyte *pixels;
    GLubyte *prevImage;
    GLubyte *newImage;

    pixels = checkImage(width, height, 8);

    if (pixels == nullptr) {
        return 0;
    }

    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    level = 1;
    prevImage = &pixels[0];

    while (width > 1 && height > 1) {
        int newWidth, newHeight;
        mipMap2D(prevImage, &newImage, width, height, &newWidth, &newHeight);

        glTexImage2D(GL_TEXTURE_2D, level, GL_RGB,
                     newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, newImage);

        free(prevImage);

        prevImage = newImage;

        level++;

        width = newWidth;
        height = newHeight;
    }

    free(newImage);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureId;
}


int EGLInit(ESContext *esContext) {

    UserData *userData = (UserData *) esContext->userData;

    ANativeActivity* activity = esContext->activity;
    // 从asset文件夹加载shader
    char* vertex = readAssetFile("vertex_mipmap.glsl", activity->assetManager);
    char* fragment = readAssetFile("fragment_mipmap.glsl", activity->assetManager);
    // 加载program
    userData->program = loadProgram(vertex, fragment);
    if (userData->program == 0) {
        return FALSE;
    }

    // 获取sampler 的location
    userData->samplerLoc = glGetUniformLocation(userData->program, "s_texture");
    // 获取 offset 的location
    userData->offsetLoc = glGetUniformLocation(userData->program, "u_offset");

    // 加载texture
    userData->textureId = createMipMappedTexture2D();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return TRUE;
}


void onDraw(ESContext *esContext) {
    UserData *userData = (UserData *)esContext->userData;
    GLfloat vertices[] = {
            -0.5f, 0.5f, 0.0f, 1.5f,
            0.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 0.75f,
            0.0f, 1.0f,
            0.5f, -0.5f, 0.0f, 0.75f,
            1.0f, 1.0f,
            0.5f, 0.5f, 0.0f, 1.5f,
            1.0f, 0.0f
    };

    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glViewport(0, 0, esContext->width, esContext->height);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(userData->program);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), vertices);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), &vertices[4]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glUniform1i(userData->samplerLoc, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glUniform1f(userData->offsetLoc, -0.6f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glUniform1f(userData->offsetLoc, 0.6f);
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
    glDeleteProgram(userData->program);
}



/**
 * OpenGLES应用程序入口
 * @param esContext
 * @return
 */
int onCreate(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    createWindow(esContext, "Hello MipMap2D", 320, 240, ES_WINDOW_RGB);
    if (!EGLInit(esContext)) {
        return GL_FALSE;
    }
    // 注册应用程序关闭函数
    registerShutdownFunc(esContext, windowShutdown);
    // 注册应用程序绘制函数
    registerDrawFunc(esContext, onDraw);

    return GL_TRUE;
}