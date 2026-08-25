// ========================================================
// تطبيق أندرويد بلغة C++ الكاملة المضمون للتجميع
// ========================================================

#include <jni.h>
#include <errno.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <cmath>

#include "native_app_glue.h"

#define LOG_TAG "NativeCppApp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

enum AppState {
    STATE_SPLASH_SCREEN,
    STATE_MAIN_APP
};

struct Engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    
    AppState state;
    float splashTimer;
    float bgR, bgG, bgB;
};

static int engine_init_display(struct Engine* engine) {
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
        EGL_NONE
    };
    EGLint w, h, format, numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);

    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    context = eglCreateContext(display, config, NULL, contextAttribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGE("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->surface = surface;
    engine->context = context;
    engine->width = w;
    engine->height = h;

    LOGI("OpenGL ES Initialized: %d x %d", w, h);
    return 0;
}

static void engine_draw_frame(struct Engine* engine) {
    if (engine->display == NULL) return;

    if (engine->state == STATE_SPLASH_SCREEN) {
        engine->splashTimer += 0.03f;
        float pulse = (sinf(engine->splashTimer * 3.0f) + 1.0f) * 0.5f;
        glClearColor(0.12f, 0.1f * pulse, 0.25f + 0.2f * pulse, 1.0f);
        
        if (engine->splashTimer > 3.0f) {
            engine->state = STATE_MAIN_APP;
            LOGI("Transitioning from Splash Screen to Main App");
        }
    } else {
        glClearColor(engine->bgR, engine->bgG, engine->bgB, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(engine->display, engine->surface);
}

static void engine_term_display(struct Engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) eglDestroyContext(engine->display, engine->context);
        if (engine->surface != EGL_NO_SURFACE) eglDestroySurface(engine->display, engine->surface);
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct Engine* engine = (struct Engine*)app->userData;
    
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        if ((action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_DOWN) {
            if (engine->state == STATE_SPLASH_SCREEN) {
                engine->state = STATE_MAIN_APP;
                return 1;
            }

            float x = AMotionEvent_getX(event, 0);
            float y = AMotionEvent_getY(event, 0);
            engine->bgR = x / (float)engine->width;
            engine->bgG = y / (float)engine->height;
            engine->bgB = 0.5f;

            return 1;
        }
    }
    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct Engine* engine = (struct Engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            engine_draw_frame(engine);
            break;
    }
}

void android_main(struct android_app* state) {
    struct Engine engine;
    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    engine.state = STATE_SPLASH_SCREEN;
    engine.splashTimer = 0.0f;
    engine.bgR = 0.1f;
    engine.bgG = 0.4f;
    engine.bgB = 0.6f;

    while (1) {
        int ident, events;
        struct android_poll_source* source;

        while ((ident = ALooper_pollOnce(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }
        engine_draw_frame(&engine);
    }
}