//
// Created by cain on 2017/7/8.
//

#include "../util/EGLUtil.h"

#define NUM_PARTICLES   1000
#define PARTICLE_SIZE   7

#define ATTRIBUTE_LIFETIME_LOCATION       0
#define ATTRIBUTE_STARTPOSITION_LOCATION  1
#define ATTRIBUTE_ENDPOSITION_LOCATION    2

typedef struct
{
    GLuint programObject;

    GLint timeLoc;
    GLint colorLoc;
    GLint centerPositionLoc;
    GLint samplerLoc;

    GLuint textureId;

    float particleData[ NUM_PARTICLES * PARTICLE_SIZE ];

    float time;

} UserData;


GLuint loadTexture(void *ioContext, char *fileName) {
    int width, height;
    char *buffer = loadTGA(ioContext, fileName, &width, &height);
    GLuint texId;

    if (buffer == NULL) {
        ALOGE("Error loading (%s) image.\n", fileName);
        return 0;
    }

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    free(buffer);

    return texId;
}


int Init(ESContext *esContext) {
    UserData *userData = (UserData *)esContext->userData;
    int i;

    char vShaderStr[] =
            "#version 300 es                                      \n"
                    "uniform float u_time;                                \n"
                    "uniform vec3 u_centerPosition;                       \n"
                    "layout(location = 0) in float a_lifetime;            \n"
                    "layout(location = 1) in vec3 a_startPosition;        \n"
                    "layout(location = 2) in vec3 a_endPosition;          \n"
                    "out float v_lifetime;                                \n"
                    "void main()                                          \n"
                    "{                                                    \n"
                    "  if ( u_time <= a_lifetime )                        \n"
                    "  {                                                  \n"
                    "    gl_Position.xyz = a_startPosition +              \n"
                    "                      (u_time * a_endPosition);      \n"
                    "    gl_Position.xyz += u_centerPosition;             \n"
                    "    gl_Position.w = 1.0;                             \n"
                    "  }                                                  \n"
                    "  else                                               \n"
                    "  {                                                  \n"
                    "     gl_Position = vec4( -1000, -1000, 0, 0 );       \n"
                    "  }                                                  \n"
                    "  v_lifetime = 1.0 - ( u_time / a_lifetime );        \n"
                    "  v_lifetime = clamp ( v_lifetime, 0.0, 1.0 );       \n"
                    "  gl_PointSize = ( v_lifetime * v_lifetime ) * 40.0; \n"
                    "}";

    char fShaderStr[] =
            "#version 300 es                                      \n"
                    "precision mediump float;                             \n"
                    "uniform vec4 u_color;                                \n"
                    "in float v_lifetime;                                 \n"
                    "layout(location = 0) out vec4 fragColor;             \n"
                    "uniform sampler2D s_texture;                         \n"
                    "void main()                                          \n"
                    "{                                                    \n"
                    "  vec4 texColor;                                     \n"
                    "  texColor = texture( s_texture, gl_PointCoord );    \n"
                    "  fragColor = vec4( u_color ) * texColor;            \n"
                    "  fragColor.a *= v_lifetime;                         \n"
                    "}                                                    \n";

    userData->programObject = loadProgram(vShaderStr, fShaderStr);

    userData->timeLoc = glGetUniformLocation ( userData->programObject, "u_time" );
    userData->centerPositionLoc = glGetUniformLocation ( userData->programObject, "u_centerPosition" );
    userData->colorLoc = glGetUniformLocation ( userData->programObject, "u_color" );
    userData->samplerLoc = glGetUniformLocation ( userData->programObject, "s_texture" );

    glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );

    srand ( 0 );
    // 初始化粒子系统的位置
    for (i = 0; i < NUM_PARTICLES; i++) {
        float *particleData = &userData->particleData[i * PARTICLE_SIZE];

        *particleData++ = ((float)(rand() % 10000) / 10000.0f);

        *particleData++ = ((float)(rand() % 10000) / 5000.0f) - 1.0f;
        *particleData++ = ((float)(rand() % 10000) / 5000.0f) - 1.0f;
        *particleData++ = ((float)(rand() % 10000) / 5000.0f) - 1.0f;

        *particleData++ = ((float)(rand() % 10000) / 40000.0f) - 0.125f;
        *particleData++ = ((float)(rand() % 10000) / 40000.0f) - 0.125f;
        *particleData++ = ((float)(rand() % 10000) / 40000.0f) - 0.125f;
    }

    userData->time = 1.0f;

    userData->textureId = loadTexture(esContext->activity->assetManager, "smoke.tga");

    if (userData->textureId <= 0) {
        return FALSE;
    }
    return TRUE;
}

void update(ESContext *esContext, float deltaTime) {
    UserData *userData = (UserData *)esContext->userData;

    userData->time += deltaTime;

    glUseProgram ( userData->programObject );

    // 1秒钟之后，更新粒子系统的位置
    if (userData->time >= 1.0f) {
        float centerPos[3];
        float color[4];

        userData->time = 0.0f;

        centerPos[0] = ((float) (rand() % 10000) / 10000.0f) - 0.5f;
        centerPos[1] = ((float) (rand() % 10000) / 10000.0f) - 0.5f;
        centerPos[2] = ((float) (rand() % 10000) / 10000.0f) - 0.5f;

        glUniform3fv(userData->centerPositionLoc, 1, &centerPos[0]);

        color[0] = ((float) (rand() % 10000) / 20000.0f) + 0.5f;
        color[1] = ((float) (rand() % 10000) / 20000.0f) + 0.5f;
        color[2] = ((float) (rand() % 10000) / 20000.0f) + 0.5f;
        color[3] = 0.5;

        glUniform4fv(userData->colorLoc, 1, &color[0]);
    }

    glUniform1f(userData->timeLoc, userData->time);
}

void onDraw(ESContext *esContext) {
    UserData *userData = (UserData *)esContext->userData;

    glViewport ( 0, 0, esContext->width, esContext->height );

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(userData->programObject);

    glVertexAttribPointer(ATTRIBUTE_LIFETIME_LOCATION, 1, GL_FLOAT,
                            GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat),
                            userData->particleData);

    glVertexAttribPointer(ATTRIBUTE_ENDPOSITION_LOCATION, 3, GL_FLOAT,
                            GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat),
                            &userData->particleData[1]);

    glVertexAttribPointer(ATTRIBUTE_STARTPOSITION_LOCATION, 3, GL_FLOAT,
                            GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat),
                            &userData->particleData[4]);


    glEnableVertexAttribArray(ATTRIBUTE_LIFETIME_LOCATION);
    glEnableVertexAttribArray(ATTRIBUTE_ENDPOSITION_LOCATION);
    glEnableVertexAttribArray(ATTRIBUTE_STARTPOSITION_LOCATION);

    // 混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId);

    glUniform1i(userData->samplerLoc, 0);

    glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
}

void windowShutdown(ESContext *esContext) {
    UserData *userData = (UserData *)esContext->userData;
    glDeleteTextures(1, &userData->textureId);
    glDeleteProgram(userData->programObject);
}

int onCreate(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    createWindow(esContext, "ParticleSystem", 640, 480, ES_WINDOW_RGB);
    if (!Init(esContext)) {
        return GL_FALSE;
    }
    registerDrawFunc(esContext, onDraw);
    registerUpdateFunc(esContext, update);
    registerShutdownFunc(esContext, windowShutdown);

    return GL_TRUE;
}
