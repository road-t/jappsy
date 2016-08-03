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

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.Point;
import android.graphics.drawable.Drawable;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import com.jappsy.JappsyEngine;

/**
 * OpenGL Surface
 */
public class JappsyView extends GLSurfaceView {

	private static String TAG = "JappsyView";
	private Activity m_activity;
	private boolean m_created = false;

	/**
	 * Device independent pixel density per 1 inch
	 */
	private int m_dpi;

	/**
	 * Device interface scaling factor
	 */
	private float m_scale;

	/**
	 * Device screen size in inches
	 */
	private float m_diag;

	/**
	 *
	 */
	private int m_line;

	/**
	 * SDK Dependent Methods
	 */
	private abstract class Methods {
		public abstract void onCreate();
		public abstract void onEnterFullScreen();
		public abstract void onExitFullScreen();
	}

	/**
	 * Methods for SDK lower than 16 version
	 */
	@SuppressWarnings("deprecation")
	private class MethodsL16 extends Methods {
		@Override public void onCreate() {
			Display display = m_activity.getWindowManager().getDefaultDisplay();
			DisplayMetrics metrics = new DisplayMetrics();
			display.getMetrics(metrics);
			m_dpi = metrics.densityDpi;
			m_scale = metrics.density;
			m_line = (int)(metrics.density * 48);

			int width = display.getWidth();
			int height = display.getHeight();
			m_diag = (float) (Math.round(Math.sqrt(width * width + height * height) * 2.0 / m_dpi) / 2.0);
		}

		@Override public void onEnterFullScreen() {
			m_activity.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
					WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}

		@Override public void onExitFullScreen() {
			m_activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}
	}

	/**
	 * Methods for SDK grater or equal 16 version
	 */
	@TargetApi(16)
	private class MethodsGE16 extends Methods {
		@Override public void onCreate() {
			Display display = m_activity.getWindowManager().getDefaultDisplay();
			DisplayMetrics metrics = new DisplayMetrics();
			display.getMetrics(metrics);
			m_dpi = metrics.densityDpi;
			m_scale = metrics.density;
			m_line = (int)(metrics.density * 48);

			Point size = new Point();
			display.getSize(size);
			m_diag = (float) (Math.round(Math.sqrt(size.x * size.x + size.y * size.y) * 2.0 / m_dpi) / 2.0);
		}

		@Override public void onEnterFullScreen() {
			View decorView = m_activity.getWindow().getDecorView();
			// Hide the status bar.
			int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
			decorView.setSystemUiVisibility(uiOptions);
			// Remember that you should never show the action bar if the
			// status bar is hidden, so hide that too if necessary.
			ActionBar actionBar = m_activity.getActionBar();
			if (actionBar != null)
				actionBar.hide();
		}

		@Override public void onExitFullScreen() {
			View decorView = m_activity.getWindow().getDecorView();
			// Hide the status bar.
			int uiOptions = View.SYSTEM_UI_FLAG_VISIBLE;
			decorView.setSystemUiVisibility(uiOptions);
			// Remember that you should never show the action bar if the
			// status bar is hidden, so hide that too if necessary.
			ActionBar actionBar = m_activity.getActionBar();
			if (actionBar != null)
				actionBar.hide();
		}
	}

	private Methods m_methods;

	private static void checkEglError(String prompt, EGL10 egl) {
		int error;
		while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
			Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
		}
	}

	private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
		private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
		public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
			Log.w(TAG, "creating OpenGL ES 2.0 context");
			checkEglError("Before eglCreateContext", egl);
			int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
			EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
			checkEglError("After eglCreateContext", egl);
			return context;
		}

		public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
			JappsyEngine.onDestroy();
			egl.eglDestroyContext(display, context);
		}
	}

	public JappsyView(Activity context) {
		super(context);
		m_activity = context;

		// Configure methods
		if (Build.VERSION.SDK_INT < 16) {
			m_methods = new MethodsL16();
		} else {
			m_methods = new MethodsGE16();
		}

		setEGLContextFactory(new ContextFactory());
		setPreserveEGLContextOnPause(true);
		setRenderer(new Renderer());
	}

	/**
	 * Create OpenGL SurfaceView and initialize it
	 * @param context Current activity
	 * @return null - Device not supported
	 */
	public static JappsyView create(Activity context) {
		// Check if the system supports OpenGL ES 2.0.
		final ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
		final ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
		final boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000;

		if (supportsEs2) {
			return new JappsyView(context);
		}

		return null;
	}

	private class Renderer implements GLSurfaceView.Renderer {
		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			if (m_created) {
				JappsyEngine.onDestroy();
			}
			JappsyEngine.onCreate();
			m_created = true;
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int width, int height) {
			JappsyEngine.onUpdate(width, height);
		}

		@Override
		public void onDrawFrame(GL10 gl) {
			JappsyEngine.onFrame();
		}
	}

	@Override public void onResume() {
		m_activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		super.onResume();
		JappsyEngine.onResume();
	}

	@Override public void onPause() {
		JappsyEngine.onPause();
		m_activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		super.onPause();
	}

	private Activity m_restoreActivity = null;
	private View m_restoreView = null;

	/**
	 * Enter fullscreen mode replacing target activity with single OpenGL view
	 * @param target current active activity
	 */
	public void enterFullscreen(Activity target) {
		// Запоминаем содержимое и заменяем
		m_restoreActivity = target;
		m_restoreView = ((ViewGroup) target.findViewById(android.R.id.content)).getChildAt(0);
		target.setContentView(this);

		m_methods.onEnterFullScreen();
	}

	/**
	 * Exit fullscreen mode restoring target activity
	 */
	public void exitFullscreen() {
		m_methods.onExitFullScreen();

		// Восстанавливаем содержимое
		m_restoreActivity.setContentView(m_restoreView);
		m_restoreView = null;
		m_restoreActivity = null;
	}

}
