#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>


#include "model_processor.h"
#include "game_area.h"
#include "pipelines.h"
#include "scene_manager.h"

EngineBase base;
pthread_t thread;

bool createFinished = false;
bool changedFinished = false;

bool paused = false, resumed = false;

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_drawFrame(JNIEnv *env, jobject thiz)
{
    while(!createFinished && !changedFinished);
    pthread_create(&thread, NULL, mainLoop, &base);

    __android_log_print(ANDROID_LOG_FATAL, "APP", "Surface created, running ...");
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_createVulkanSurface(
        JNIEnv *env, jobject thiz, jobject surface)
{
    ANativeWindow *wnd = ANativeWindow_fromSurface(env, surface);
    if(!resumed)
    {
        makeEngineBase(&base);
    }
    makeSurface(&base, wnd);
    if(!resumed)
    {
        sceneManager_allocateSceneMemory(0, 2, 2);
    }
    createFinished = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_surfaceChanged(JNIEnv *env,
        jobject thiz, jint width, jint height)
{
        makeRenderImage(&base, width, height);
        makeScenes(&base, width, height);
        setCurrentScene(sceneManager_getScene(0));

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
        processPositionLine(currentLine, len);
    }
    else if(currentLine[0] == 'v' && currentLine[1] == 't')
    {
        processTextureLine(currentLine, len);
    }
    else if(currentLine[0] == 'v' && currentLine[1] == 'n')
    {
        processNormalLine(currentLine, len);
    }
    else if(currentLine[0] == 'f' && currentLine[1] == ' ')
    {
        processTriangleLine(currentLine, len);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_beginModel(JNIEnv *env, jobject thiz,
        jint vertex_count, jint uv_count, jint normal_count, jint triangle_count)
{
    beginModel(vertex_count, uv_count, normal_count, triangle_count * 3);
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_endModel(JNIEnv *env, jobject thiz)
{
    endModel();
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_addShaderBytes(JNIEnv *env, jobject thiz,
       jbyteArray bytes, jint index)
{
    uint32_t arrayLen = env->GetArrayLength(bytes);
    signed char *arrayElements = env->GetByteArrayElements(bytes, NULL);
    putSpvCode(index, (char *)arrayElements, arrayLen);
    env->ReleaseByteArrayElements(bytes, arrayElements, 0);
}
extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_initApp(JNIEnv *env, jobject thiz)
{
    initCodesArray();
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_appResume(JNIEnv *env, jobject thiz)
{
    if(paused)
    {
        __android_log_print(ANDROID_LOG_FATAL, "APP", "resume");
        paused = false;
        resumed = true;
    }
}


extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_appPause(JNIEnv *env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_FATAL, "APP", "Pause");

    paused = true;
    running = false;
    pthread_join(thread, NULL);

    sceneMamager_releaseScenes(&base);
    releaseSurfaceImages(&base);
}
extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_destroyedSurface(JNIEnv *env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_FATAL, "APP", "Surface destroyed");
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_releaseResources(JNIEnv *env, jobject thiz)
{

}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_appStop(JNIEnv *env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_FATAL, "APP", "Stop");
}