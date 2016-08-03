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

public class JappsyEngine {
	public static boolean m_initialized = false;

	static {
		System.loadLibrary("libJappsyEngine-jni");
	}

	/**
	 * Initialize JappsyEngine
	 * @return false on error
	 */
	public static native boolean initialize();

	/**
	 * Cleanup memory used by JappsyEngine
	 */
	public static native void free();

	/**
	 * Debug native memory usage
	 */
	public static native void mallinfo();

	public static native void onCreate();
	public static native void onDestroy();
	public static native void onUpdate(int width, int height);
	public static native void onPause();
	public static native void onResume();
	public static native void onFrame();
}