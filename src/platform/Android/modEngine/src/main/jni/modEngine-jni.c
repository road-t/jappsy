#include "include/platform.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onCreate(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_INFO, "JNI", "onCreate");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onDestroy(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_INFO, "JNI", "onDestroy");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onUpdate(JNIEnv *env, jclass type, jint width, jint height) {
	__android_log_print(ANDROID_LOG_INFO, "JNI", "onUpdate");

	glViewport(0, 0, width, height);
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onPause(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_INFO, "JNI", "onPause");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onResume(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_INFO, "JNI", "onResume");
}

static int color = 0;

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onFrame(JNIEnv *env, jclass type) {
	//__android_log_print(ANDROID_LOG_INFO, "JNI", "onFrame");

	float c = (float)color / 255.0;
	glClearColor(c, c, c, 1.0f);
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	color++;
	if (color >= 256) color = 0;
}
