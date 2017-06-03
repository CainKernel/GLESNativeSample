//
// Created by cain on 2017/5/23.
//

#include "EGLUtil.h"
#include "../glue/android_native_app_glue.h"

/**
 * 获取当前时间
 * @return
 */
static float getCurrentTime() {
    struct timespec clockRealTime;
    clock_gettime(CLOCK_MONOTONIC, &clockRealTime);
    double timeInSeconds = clockRealTime.tv_sec + (double)clockRealTime.tv_nsec / 1e9;
    return (float) timeInSeconds;
}

/**
 * 处理命令
 * @param androidApp
 * @param cmd
 */
static void handleCommand(struct android_app* androidApp, int32_t cmd) {

    ESContext *esContext = (ESContext*)androidApp->userData;
    switch (cmd) {

        // 输出改变
        case APP_CMD_INPUT_CHANGED:
            break;

        // 初始化窗口
        case APP_CMD_INIT_WINDOW:
            esContext->eglNativeDisplay = EGL_DEFAULT_DISPLAY;
            esContext->eglNativeWindow = androidApp->window;
            // 如果没有main入口，则推出程序
            if (onCreate(esContext) != GL_TRUE) {
                exit(0);
            }
            break;

        // 关闭窗口
        case APP_CMD_TERM_WINDOW:
//            if (esContext->accelerometerSensor != nullptr) {
//                ASensorEventQueue_disableSensor(esContext->sensorEventQueue,
//                                                esContext->accelerometerSensor);
//            }
            //  如果定义了关闭窗口函数，则调用该函数处理
            if (esContext->shutdownFunc != nullptr) {
                esContext->shutdownFunc(esContext);
            }
            // 如果用户数据不为空，则需要清空该数据释放内存
            if (esContext->userData != nullptr) {
                free(esContext->userData);
            }
            // 清空引用的上下文数据，防止内存泄漏
            memset(esContext, 0, sizeof(ESContext));
            break;

        // 窗口大小改变
        case APP_CMD_WINDOW_RESIZED:
            if (esContext->updateFunc != nullptr) {
                esContext->onWindowResize(esContext, esContext->width, esContext->height);
            }
            break;

        // 窗口需要重绘
        case APP_CMD_WINDOW_REDRAW_NEEDED:
            if (esContext->onRedrawIfNeeded != nullptr) {
                esContext->onRedrawIfNeeded(esContext);
            }
            break;

        // 窗口内容区域发生改变
        case APP_CMD_CONTENT_RECT_CHANGED:
            if (esContext->onWindowContentChanged != nullptr) {
                esContext->onWindowContentChanged(esContext);
            }
            break;

        // 获取焦点
        case APP_CMD_GAINED_FOCUS:
//            if (esContext->accelerometerSensor != nullptr) {
//                // 启用传感器
//                ASensorEventQueue_enableSensor(esContext->sensorEventQueue,
//                                               esContext->accelerometerSensor);
//                // 设置获取事件的频率
//                ASensorEventQueue_setEventRate(esContext->sensorEventQueue,
//                                               esContext->accelerometerSensor,
//                                               (1000L / 60) * 1000);
//            }
            if (esContext->onWindowFocusChanged != nullptr) {
                esContext->onWindowFocusChanged(esContext, true);
            }
            break;

        // 失去焦点
        case APP_CMD_LOST_FOCUS:
//            // 失去焦点时需要关闭加速度传感器
//            if (esContext->accelerometerSensor != nullptr) {
//                ASensorEventQueue_disableSensor(esContext->sensorEventQueue,
//                                                esContext->accelerometerSensor);
//            }
            // 停止动画
            esContext->animate = false;

            if (esContext->onWindowFocusChanged != nullptr) {
                esContext->onWindowFocusChanged(esContext, false);
            }
            break;

        // 当前设备配置发生改变
        case APP_CMD_CONFIG_CHANGED:
            if (esContext->onConfigurationChanged != nullptr) {
                esContext->onConfigurationChanged(esContext);
            }
            break;

        // 低内存时
        case APP_CMD_LOW_MEMORY:
            if (esContext->onLowMemory != nullptr) {
                esContext->onLowMemory(esContext);
            }
            break;

        // 对应onStrart
        case APP_CMD_START:
            if (esContext->onStart != nullptr) {
                esContext->onStart(esContext);
            }
            break;

        // 对应onResume
        case APP_CMD_RESUME:
            if (esContext->onResume != nullptr) {
                esContext->onResume(esContext);
            }
            break;

        // 保存APP的状态，对应onSaveInstanceState
        case APP_CMD_SAVE_STATE:
            if (esContext->onSavedInstance != nullptr) {
                esContext->onSavedInstance(esContext, androidApp);
            }
            break;

        // 对应onPause
        case APP_CMD_PAUSE:
            if (esContext->onPause != nullptr) {
                esContext->onPause(esContext);
            }
            break;

        // 对应onStop
        case APP_CMD_STOP:
            if (esContext->onStop != nullptr) {
                esContext->onStop(esContext);
            }
            break;

        // 对应onDestory
        case APP_CMD_DESTROY:
            if (esContext->onDestroy != nullptr) {
                esContext->onDestroy(esContext);
            }
            // 如果用户数据不为空，则需要清空该数据释放内存
            if (esContext->userData != nullptr) {
                free(esContext->userData);
            }
            // 清空引用的上下文数据，防止内存泄漏
            memset(esContext, 0, sizeof(ESContext));
            break;
    }
}

/**
 * 处理输出事件
 * @param androidApp
 * @param event
 * @return
 */
static int32_t handleInput(struct android_app* app, AInputEvent* event) {
    ESContext* esContext = (ESContext *)app->userData;
    switch(AInputEvent_getType(event)) {
        // 触摸事件
        case AINPUT_EVENT_TYPE_MOTION:
            if (esContext != nullptr && esContext->onTouchEvent != nullptr) {
                return esContext->onTouchEvent(esContext, event);
            }
            break;

        // 按键事件
        case AINPUT_EVENT_TYPE_KEY:
            if (esContext != nullptr && esContext->onKeyEvent != nullptr) {
                return esContext->onKeyEvent(esContext, event);
            }
            break;
    }
    return 0;
}

/**
 * 胶水层暴露出来的app入口程序，相当于Activity中的onCreate方法
 * @param app
 */
void android_main(struct android_app* app) {

    ESContext esContext;
    float lastTime;
    app_dummy();
    memset(&esContext, 0, sizeof(ESContext));
    esContext.activity = app->activity;
    // 处理命令
    app->onAppCmd = handleCommand;
    // 处理输入事件
    app->onInputEvent = handleInput;
    // 绑定用户数据
    app->userData = &esContext;
    // 准备传感器管理器
    esContext.sensorManager = ASensorManager_getInstance();
    // 获得默认的加速度传感器
    esContext.accelerometerSensor = ASensorManager_getDefaultSensor(esContext.sensorManager,
                                                                    ASENSOR_TYPE_ACCELEROMETER);
    // 准备传感器事件队列
    esContext.sensorEventQueue = ASensorManager_createEventQueue(esContext.sensorManager,
                                                                 app->looper, LOOPER_ID_USER,
                                                                 nullptr, nullptr);
    // 获取暂存状态数据
    if (app->savedState != nullptr) {
        esContext.savedState = app->savedState;
    }

    // 获取当前配置信息
    if (app->config != nullptr) {
        esContext.config = app->config;
    }

    // 获取当前时间
    lastTime = getCurrentTime();

    while (true) {
        int events;
        int ident;
        struct  android_poll_source* source;
        // 如果消息队列中存在消息，则将消息取出来处理
        while((ident = ALooper_pollAll(0, nullptr, &events, (void**) &source)) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }

            //如果Looper ID是用户的ID，则可以使用监听传感器事件队列
//            if (ident == LOOPER_ID_USER) {
//                if (esContext.accelerometerSensor != nullptr) {
//                    ASensorEvent event;
//                    // 取出加速度传感器的值
//                    while (ASensorEventQueue_getEvents(esContext.sensorEventQueue, &event, 1) > 0) {
//                        ALOGI("accelometer: x= %f, y=%f, z=%f",
//                              event.acceleration.x,
//                              event.acceleration.y,
//                              event.acceleration.z);
//                    }
//                }
//            }

            // 如果请求退出app
            if (app->destroyRequested != 0) {
                // 推出应用时需要关掉EGL
                if (esContext.shutdownFunc != nullptr) {
                    esContext.shutdownFunc(&esContext);
                }
                return;
            }
        }

        // 等待native window 的创建
        if (esContext.eglNativeWindow == nullptr) {
            continue;
        }

        // 是否允许动画
        if (esContext.animate && esContext.onAnimate != nullptr) {
            esContext.onAnimate(&esContext);
        }

        // app请求刷新函数不为空，则刷新数据
        if (esContext.updateFunc != nullptr) {
            float curTime = getCurrentTime();
            float deltaTime = curTime - lastTime;
            lastTime = curTime;
            esContext.updateFunc(&esContext, deltaTime);
        }

        // 绘制函数不为空，调用绘制函数，并且采用双缓冲方式绘制显示
        if (esContext.drawFunc != nullptr) {
            esContext.drawFunc(&esContext);
            // 双缓冲方式绘制，避免闪屏现象
            eglSwapBuffers(esContext.eglDisplay, esContext.eglSurface);
        }
    }
}