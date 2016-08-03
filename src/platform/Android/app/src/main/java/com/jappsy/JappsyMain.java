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

import java.io.File;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import com.jappsy.cipher.CRC;
import com.jappsy.cipher.MD5;
import com.jappsy.core.Global;
import com.jappsy.core.Log;
import com.jappsy.core.util.SmoothValue;
import com.jappsy.exceptions.EGLInvalidResource;
import com.jappsy.exceptions.EGlobal;
import com.jappsy.exceptions.EOutOfMemory;
import com.jappsy.gui.JAlignX;
import com.jappsy.gui.JCanvas;
import com.jappsy.gui.JPaint;
import com.jappsy.gui.JAlignY;
import com.jappsy.io.CacheFile;
import com.jappsy.opengl.GLFramebuffer;
import com.jappsy.opengl.image.GLImage;
import com.jappsy.opengl.image.GLRender;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;

public class JappsyMain extends Activity {

    private Context m_context = this;
    public static JappsyMain m_main = null;

    private GLSurfaceView m_view;
    private JCanvas m_canvas = null;

    private GLFramebuffer m_fb = null;

    private class Renderer implements GLSurfaceView.Renderer {
        private JPaint m_paint;
        private JPaint m_textPaint;
        private JPaint m_fpsPaint;

        private GLImage m_fonts[] = new GLImage[7];
        private GLImage m_logo;
        private GLImage m_image;
        private SmoothValue m_fpsValue = new SmoothValue(30);
        private SmoothValue m_renderValue = new SmoothValue(30);

        String fontFaces[] = {
                "sans_serif",
                "serif",
                "monospace",
                "script",
                "segoe",
                "gothic",
                "old"
        };

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            JCanvas.fill(0x80808080);

            if (m_canvas != null) {
                m_canvas.Destroy();
            }
            m_canvas = new JCanvas(640, 480);

            try {
                if (m_fb != null)
                    m_fb.release();
                m_fb = GLFramebuffer.create(250, 250, false);
            } catch (EOutOfMemory e) {
                e.printStackTrace();
            }

            if (m_paint == null) {
                m_paint = new JPaint();
                m_paint.setAntiAlias(true);
                m_paint.setScale(2.0f);
                int[] colors = {0xFF0080C0, 0xFF0080C0, 0xFF00C2FF, 0xFF004080};
                m_paint.setSDFColors(colors);
                m_paint.setStroke(-0.5f, 1.0f, 0xFF000000);
                m_paint.setAlignX(JAlignX.CENTER);
                m_paint.setAlignY(JAlignY.MIDDLE);
            }

            if (m_textPaint == null) {
                m_textPaint = new JPaint();
                m_textPaint.setAntiAlias(true);
                m_textPaint.setScale(Global.m_scale);
                m_textPaint.setColor(0xFFFFFFFF);
                m_textPaint.setStroke(0.0f,  0.0f, 0xFF000000);
                m_textPaint.setAlignX(JAlignX.LEFT);
                m_textPaint.setAlignY(JAlignY.TOP);
            }

            if (m_fpsPaint == null) {
                m_fpsPaint = new JPaint();
                m_fpsPaint.setAntiAlias(true).setScale(Global.m_scale).setColor(0xFFFFFFFF).setStroke(-0.0f, 1.0f, 0xFFFFFFFF).setAlignX(JAlignX.LEFT).setAlignY(JAlignY.TOP).setTextSize(8.0f);
            }

            File cache;
            try {
                cache = Global.getDiskCacheDir("resources" + File.separator + "gui");
                String resName = "appicon2";//"appicon";//"ic_daynight";//"test_patch_1";
                String fileName = resName + ".jpg";//".png";//".jimg";//".9.jimg";
                InputStream is = Global.getRawStream(resName);
                CacheFile file = CacheFile.createFromStream(cache, fileName, is);
                m_image = GLImage.create(file);
            } catch (EGlobal e) {
                e.printStackTrace();
            }

            try {
                cache = Global.getDiskCacheDir("resources" + File.separator + "fonts");
                for (int i = 0; i < 7; i++) {
                    String resName = "font_48_" + fontFaces[i];
                    String fileName = resName + ".sdff";
                    InputStream is = Global.getRawStream(resName);
                    CacheFile file = CacheFile.createFromStream(cache, fileName, is);
                    m_fonts[i] = GLImage.create(file);
                }
            } catch (EGlobal e) {
                e.printStackTrace();
            }

            try {
                cache = Global.getDiskCacheDir("resources");
                String resName = "jappsy";
                String fileName = resName + ".sdfi";
                InputStream is = Global.getRawStream(resName);
                CacheFile file = CacheFile.createFromStream(cache, fileName, is);
                m_logo = GLImage.create(file);
            } catch (EGlobal e) {
                e.printStackTrace();
            }

        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            m_canvas.onRender(width, height, width, height);
        }

        private int c = -255;
        private int idx = 2;
        private long nanoTime = 0;

        @Override
        public void onDrawFrame(GL10 gl) {
            GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);

			/*
			m_canvas.beginFramebufferUpdate(m_fb);
				m_paint.setColor(0xFF000000 | (int)(0xFF - c));
				m_canvas.drawRect(50, 50, 150, 150, m_paint);
			m_canvas.endFramebufferUpdate();
			*/

            int cc = Math.abs(c);
            JCanvas.fill(0xFF000000 | (cc << 16));// | (cc << 8) | (cc));
            c = c + 1;
            if (c >= 255) {
                c = -255;
                idx++;
                if (idx >= 7) idx = 0;
            }

            long time1 = System.nanoTime();
			
			try {
				//m_fb.render(m_canvas, 10, 10, 410 + c, 410 + c, m_paint);
				m_image.render(m_canvas, 50, 50 + Math.abs(c), 200, 200, m_paint);

				float size = 1;
				int x = 0;
				int y = 0;
				int mh = 0;
				for (int i = 1; i < 25; i++) {
					m_textPaint.setTextSize(size);
					GLRender render = m_fonts[idx].render(m_canvas, "@" + String.valueOf((int)size) + "Текст для проверки скорости вывода", x + 10, y + 30, 2, m_textPaint);
					int h = (int)Math.ceil(render.m_size[1]);
					if (mh < h) mh = h;
					x += (int)Math.ceil(render.m_size[0]);
					if (x >= 500) {
						x = 0;
						y += mh;
						mh = 0;
					}
					size = (float)Math.ceil(size * 1.2f);
				}

				//m_logo.render(m_canvas, m_canvas.m_size[0] / 2, m_canvas.m_size[1] / 2, 256 + Math.abs(c), m_paint);
			} catch (EGLInvalidResource e) {
				e.printStackTrace();
			} catch (EOutOfMemory e) {
				e.printStackTrace();
			}

            long time2 = System.nanoTime();
			
			try {
				m_fpsValue.put(1000000000.0 / (double)(time1 - nanoTime));
				m_renderValue.put((double)(time2 - time1) / 1000000.0);
				String fps = Double.toString(Math.round(m_fpsValue.value() * 10.0) / 10.0) + "fps ~ " + Double.toString(Math.round(m_renderValue.value() * 10.0) / 10.0) + "ms";
				m_fonts[2].render(m_canvas, fps, 5, 5, 0, m_fpsPaint);
			} catch (EGLInvalidResource e) {
				e.printStackTrace();
			} catch (EOutOfMemory e) {
				e.printStackTrace();
			}
            //Log.debug("Frame: " + Float.toString(1000f / ((float)(time1 - nanoTime) / 1000000f)) + "fps - " + Float.toString((float)(time2 - time1) / 1000000f));

            nanoTime = time1;

            //throw new RuntimeException();

            Jappsy.malinfo();
        }

    }

    @Override public void onCreate(Bundle savedInstanceState) {
        m_main = this;

        Log.debug("Main > onCreate");

        super.onCreate(savedInstanceState);

        if (!Jappsy.m_initialized) {
            finish();
            return;
        }

        if (!JappsyStartup.onJappsyMain(this)) {
            finish();
            return;
        }

        //Log.debug("MD5: " + Global.md5("A"));
        Log.debug("MD5: " + MD5.utf8("A"));
        Log.debug("MD5: " + MD5.unicode("A"));

        ByteBuffer buffer = ByteBuffer.allocateDirect(4).order(ByteOrder.nativeOrder());
        IntBuffer test = buffer.asIntBuffer();
        test.put(0, 0x12345678);
        test.position(0);

        Log.debug("CRC7: " + CRC.crc7(buffer, 0, -1));
        Log.debug("CRC16: " + CRC.crc16(buffer, 0, -1));
        Log.debug("CRC32: " + CRC.crc32(buffer, 0, -1));

        Display display = getWindowManager().getDefaultDisplay();
        DisplayMetrics metrics = new DisplayMetrics();
        display.getMetrics(metrics);
        Global.m_dpi = metrics.densityDpi;
        Global.m_scale = metrics.density;
        //Point size = new Point();
        //display.getSize(size);
        int width = display.getWidth();
        int height = display.getHeight();
        Global.m_diag = (float) (Math.round(Math.sqrt(width * width + height * height) * 2.0 / Global.m_dpi) / 2.0);
        Global.m_pad = (int)(4 * Global.m_scale);
        Drawable icon = m_context.getResources().getDrawable(R.mipmap.ic_launcher);
        Global.m_line = icon != null ? icon.getMinimumHeight() : 48;

        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);

        /*
        m_view = new GLSurfaceView(this);

        // Check if the system supports OpenGL ES 2.0.
        final ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        final ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
        final boolean supportsEs2 = configurationInfo.reqGlEsVersion >= 0x20000;

        if (supportsEs2) {
            // Request an OpenGL ES 2.0 compatible context.
            m_view.setEGLContextClientVersion(2);

            // Set the renderer to our demo renderer, defined below.
            m_view.setRenderer(new Renderer());
        } else {
            // This is where you could create an OpenGL ES 1.x compatible
            // renderer if you wanted to support both ES 1 and ES 2.
            return;
        }
        */
        m_view = JappsyView.create(this);

        setContentView(m_view);
    }

    @Override public void onBackPressed() {
        moveTaskToBack(true);
    }

    @Override public boolean onKeyDown(int keyCode, KeyEvent event)  {
        if (keyCode == KeyEvent.KEYCODE_MENU) {
            //vRemote.onKeyMenu();
            return true;
        }
        //vRemote.onKey(keyCode, event);

        return super.onKeyDown(keyCode, event);
    }

    @Override protected void onResume() {
        Log.debug("Main > onResume");
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        super.onResume();
        m_view.onResume();
    }

    @Override protected void onPause() {
        Log.debug("Main > onPause");
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_view.onPause();
        super.onPause();
    }

    private void hideSoftKeyboard() {
        //InputMethodManager imm = (InputMethodManager)m_context.getSystemService(Context.INPUT_METHOD_SERVICE);
        //imm.hideSoftInputFromWindow(vChat.m_edit.getWindowToken(), 0);
    }

    @Override public void onConfigurationChanged(Configuration newConfig) {
        Log.debug("Main > onConfigurationChanged");
        super.onConfigurationChanged(newConfig);

        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            enterFullscreen();
        } else {
            exitFullscreen();
        }
        //setContentView(vMain);

        hideSoftKeyboard();
    }

    @Override protected void onDestroy() {
        Log.debug("Main > onDestroy");
        // TODO: Cache.clearAudioCache();

        super.onDestroy();
    }

    @Override public void onAttachedToWindow() {
        super.onAttachedToWindow();
        Window window = getWindow();
        window.setFormat(PixelFormat.RGBA_8888);
    }

    public void enterFullscreen() {
        //updateView();

        //if (Build.VERSION.SDK_INT < 16) {
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
		/*} else {
		    View decorView = getWindow().getDecorView();
		    // Hide the status bar.
		    int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
		    decorView.setSystemUiVisibility(uiOptions);
		    // Remember that you should never show the action bar if the
		    // status bar is hidden, so hide that too if necessary.
		    ActionBar actionBar = getActionBar();
		    actionBar.hide();
		}*/
    }

    public void exitFullscreen() {
        //updateView();

        //if (Build.VERSION.SDK_INT < 16) {
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
		/*} else {
		    View decorView = getWindow().getDecorView();
		    // Hide the status bar.
		    int uiOptions = View.SYSTEM_UI_FLAG_VISIBLE;
		    decorView.setSystemUiVisibility(uiOptions);
		    // Remember that you should never show the action bar if the
		    // status bar is hidden, so hide that too if necessary.
		    ActionBar actionBar = getActionBar();
		    actionBar.hide();
		}*/
    }

}
