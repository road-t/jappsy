/*
 * Copyright (C) 2016 The Jappsy Open Source Project (http://jappsy.com)
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.jappsy;

import android.view.MotionEvent;

public class JappsyEngine {
	public static boolean m_initialized = false;

	static {
		System.loadLibrary("JappsyEngine-jni");
	}

	/**
	 * Initialize JappsyEngine
	 * @return false on error
	 */
	public static native boolean initialize(final String cacheDir);

	/**
	 * Cleanup memory used by JappsyEngine
	 */
	public static native void free();

	/**
	 * Debug native memory usage
	 */
	public static native void mallinfo();

	public static native long onCreate();
	//public static native void onDestroy(long handle);

	public static native void setEngine(long handle, long engineHandle);

	public static native void onStart(long handle);
	public static native void onStop(long handle);

	public static native void onUpdate(long handle, int width, int height);
	public static native void onPause(long handle);
	public static native void onResume(long handle);
	public static native void onFrame(long handle);

	public static native void onTouch(long handle, MotionEvent event);

	/* ANTI-DEADLOCK */

	public static native void lockGLThread();
	public static native void unlockGLThread();
}
