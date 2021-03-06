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

public class OMEngine {

	static {
		System.loadLibrary("GameOM-jni");
	}

	public static native long onCreate(OMView view, String basePath, String token, String sessid, String devid, String locale);
	public static native void setMinimized(long engine, boolean minimized);

	public static native void onWebLocation(long engine, int index, String location);
	public static native void onWebReady(long engine, int index);
	public static native void onWebFail(long engine, int index, String error);
}
