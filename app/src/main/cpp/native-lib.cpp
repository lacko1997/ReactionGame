#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "vulkan_renderer.h"

EngineBase base;

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_drawFrame(JNIEnv *env, jobject thiz)
{

}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_createVulkanSurface(
        JNIEnv *env, jobject thiz, jobject surface)
{
    ANativeWindow *wnd = ANativeWindow_fromSurface(env, surface);
    makeEngineBase(&base);
    makeSurface(&base, wnd);
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_createVulkan(JNIEnv *env, jobject thiz)
{

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
}