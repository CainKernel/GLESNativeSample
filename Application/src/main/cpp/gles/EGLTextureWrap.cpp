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


GLubyte *checkImage(int width, int height, int checkSize) {

    GLubyte *pixels = (GLubyte *) malloc(width * height * 3);

    if (pixels == NULL) {
        return NULL;
    }

    for (int y = 0; y < height; y++ ) {
        for (int x = 0; x < width; x++ ) {
            GLubyte rColor = 0;
            GLubyte bColor = 0;

            if ((x / checkSize) % 2 == 0) {
                rColor = 255 * ((y / checkSize) % 2);
                bColor = 255 * (1 - ((y / checkSize) % 2));
            } else {
                bColor = 255 * ((y / checkSize) % 2);
                rColor = 255 * (1 - ((y / checkSize ) % 2));
            }

            pixels[(y * width + x) * 3] = rColor;
            pixels[(y * width + x) * 3 + 1] = 0;
            pixels[(y * width + x) * 3 + 2] = bColor;
        }
    }

    return pixels;
}


GLuint createTexture2D() {
    GLuint textureId;
    int width = 256, height = 256;
    GLubyte *pixels = checkImage(width, height, 64);

    if (pixels == nullptr) {
        return 0;
    }

    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureId;
}



int EGLInit(ESContext *esContext) {

    UserData *userData = (UserData *) esContext->userData;

    char vertex[] =
            "#version 300 es\n"
                    "uniform float u_offset;\n"
                    "layout(location = 0) in vec4 a_position;\n"
                    "layout(location = 1) in vec2 a_texCoord;\n"
                    "out vec2 v_texCoord;\n"
                    "void main()\n"
                    "{"
                    "gl_Position = a_position;\n"
                    "gl_Position.x += u_offset;\n"
                    "v_texCoord = a_texCoord;\n"
                    "}\n";


    char fragment[] =
            "#version 300 es\n"
                    "precision mediump float;\n"
                    "in vec2 v_texCoord;\n"
                    "layout(location = 0) out vec4 outColor;\n"
                    "uniform sampler2D s_texture;\n"
                    "void main()\n"
                    "{\n"
                    "outColor = texture( s_texture, v_texCoord );\n"
                    "}\n";

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
    userData->textureId = createTexture2D();

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    return TRUE;
}


void onDraw(ESContext *esContext) {
    UserData *userData = (UserData *)esContext->userData;
    GLfloat vertices[] = {
            -0.3f,  0.3f, 0.0f, 1.0f,  // Position 0
            -1.0f,  -1.0f,              // TexCoord 0
            -0.3f, -0.3f, 0.0f, 1.0f, // Position 1
            -1.0f,  2.0f,              // TexCoord 1
            0.3f, -0.3f, 0.0f, 1.0f, // Position 2
            2.0f,  2.0f,              // TexCoord 2
            0.3f,  0.3f, 0.0f, 1.0f,  // Position 3
            2.0f,  -1.0f               // TexCoord 3
    };

    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glViewport(0, 0, esContext->width, esContext->height);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(userData->program);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), vertices);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), &vertices[4]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId);

    glUniform1i(userData->samplerLoc, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1f(userData->offsetLoc, -0.7f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1f(userData->offsetLoc, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glUniform1f(userData->offsetLoc, 0.7f);
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
    createWindow(esContext, "Hello TextureWrap", 640, 640, ES_WINDOW_RGB);
    if (!EGLInit(esContext)) {
        return GL_FALSE;
    }
    // 注册应用程序关闭函数
    registerShutdownFunc(esContext, windowShutdown);
    // 注册应用程序绘制函数
    registerDrawFunc(esContext, onDraw);

    return GL_TRUE;
}