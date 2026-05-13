#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>

#include "vulkan_renderer.h"
#include "model_processor.h"
#include "main_menu.h"
#include "scene_elements.h"
#include "pipelines.h"

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
    makeEngineBase(&base);
    makeSurface(&base, wnd);
    mainMenu_scene.memory.vertexBuffers = (VkBuffer*)malloc(sizeof(VkBuffer));
    mainMenu_scene.memory.vertexBufferMemories = (VkDeviceMemory*)malloc(sizeof(VkDeviceMemory));
    createFinished = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_releaseResources(JNIEnv *env, jobject thiz)
{

}

float data[] =
        {
            0.5, 1.0, -0.5,
            1.0, 0.0, -0.5,
            0.0, 0.0,-0.5
        };

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_RenderActivity_surfaceChanged(JNIEnv *env,
        jobject thiz, jint width, jint height)
{
    makeRenderImage(&base, width, height);
    makeModelPipeline(&base, &mainMenu_scene, width, height);
    makeVertexBuffer(&base, &mainMenu_scene, 0, data, sizeof(data)/sizeof(float));
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
    beginModel(vertex_count, uv_count, normal_count, triangle_count);
}

extern "C" JNIEXPORT void JNICALL
Java_com_phenyl_productions_games_reactiongame_MainActivity_endModel(JNIEnv *env, jobject thiz)
{

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