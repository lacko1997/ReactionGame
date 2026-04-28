#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>

#include "vulkan_renderer.h"
#include "main_menu.h"

EngineBase base;
RenderScene mainMenu_scene;

pthread_t thread;

bool createFinished = false;
bool changedFinished = false;

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_drawFrame(JNIEnv *env, jobject thiz)
{
    while(!createFinished && !changedFinished);
    pthread_create(&thread, NULL, mainLoop, &base);
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_createVulkanSurface(
        JNIEnv *env, jobject thiz, jobject surface)
{
    ANativeWindow *wnd = ANativeWindow_fromSurface(env, surface);
    __android_log_print(ANDROID_LOG_FATAL, "ANDROID", "wnd_ptr: %p", wnd);
    makeEngineBase(&base);
    makeSurface(&base, wnd);
    createFinished = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_releaseResources(JNIEnv *env, jobject thiz)
{

}
extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_surfaceChanged(JNIEnv *env,
        jobject thiz, jint width, jint height)
{
    makeRenderImage(&base, width, height);
    makeMainMenu(&base, &mainMenu_scene, width, height);
    setCurrentScene(&mainMenu_scene);
    changedFinished = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_addLine(JNIEnv *env, jobject thiz, jstring ln)
{
    char currentLine[256];
    const char *line = env->GetStringUTFChars(ln, nullptr);
    uint32_t len = env->GetStringUTFLength(ln);

    len = (len < 256) ? len : 256;

    memcpy(currentLine, line, len);
    if(currentLine[0] == 'v' && currentLine[1] == ' ')
    {
        processPosition
    }
    else if(currentLine[0] == 'v' && currentLine[1] == 'n')
    {

    }
    else if(currentLine[0] == 'v' && currentLine[1] == 't')
    {

    }
    else if(currentLine[0] == 'f' && currentLine[1] == ' ')
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_beginModel(JNIEnv *env, jobject thiz,
        jint vertex_count, jint uv_count, jint normal_count, jint triangle_count)
{
    beginModel(vertex_count, uv_count, normal_count, triangle_count);
}