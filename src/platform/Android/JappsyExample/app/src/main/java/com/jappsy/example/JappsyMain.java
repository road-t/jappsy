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

package com.jappsy.example;

import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;

import com.jappsy.JappsyEngine;
import com.jappsy.OMView;

public class JappsyMain extends Activity {

    private static final String TAG = "Activity";
    private OMView m_view;

    @Override public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");

        super.onCreate(savedInstanceState);

        if (!JappsyEngine.m_initialized) {
            finish();
            return;
        }

        if (!JappsyStartup.onJappsyMain(this)) {
            finish();
            return;
        }

        m_view = OMView.create(this);

        setContentView(m_view);
    }

    @Override public void onBackPressed() {
        moveTaskToBack(true);
    }

    @Override public boolean onKeyDown(int keyCode, KeyEvent event) {
        return keyCode == KeyEvent.KEYCODE_MENU || super.onKeyDown(keyCode, event);
    }

    @Override protected void onResume() {
        Log.d(TAG, "onResume");

        super.onResume();
        m_view.onResume();
    }

    @Override protected void onPause() {
        Log.d(TAG, "onPause");

        m_view.onPause();
        super.onPause();
    }

    @Override public void onConfigurationChanged(Configuration newConfig) {
        Log.d(TAG, "onConfigurationChanged");
        super.onConfigurationChanged(newConfig);

        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            m_view.enterFullscreen(this);
        } else {
            m_view.exitFullscreen();
        }
    }

    @Override protected void onDestroy() {
        Log.d(TAG, "onDestroy");

        super.onDestroy();
    }

}
