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
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;

import com.jappsy.JappsyEngine;
import com.jappsy.JappsyView;
import com.jappsy.OMView;
import com.jappsy.gui.Constraint;
import com.jappsy.gui.ConstraintView;

import java.util.ArrayList;
import java.util.List;

public class JappsyMain extends Activity {

    private static final String TAG = "Activity";

    SensorManager sensorManager;
    Sensor accelerometerSensor;
    boolean accelerometerPresent;
    int lastSensorState = JappsyView.ORIENTATION_FACEUP;

    private double m_scale = 1.0;

    private OMView omView = null;
    private OMView.OMJappsyView m_view = null;

    private ArrayList<Constraint> lv; // Menu Visible
    private ArrayList<Constraint> lh; // Menu Hidden

    private ConstraintView mainView;
    private ConstraintView layoutView01;
    private ConstraintView layoutView02;
    private ConstraintView layoutView03;
    private Button hiddenView;

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

        Display display = getWindowManager().getDefaultDisplay();
        DisplayMetrics metrics = new DisplayMetrics();
        display.getMetrics(metrics);
        m_scale = metrics.density;

        // Background Color

        mainView = new ConstraintView(this);
        mainView.setBackgroundColor(Color.BLACK);

        // Menu

        ConstraintView menuView = new ConstraintView(this);
        menuView.setBackgroundColor(Color.TRANSPARENT);
        mainView.addView(menuView);

        lv = new ArrayList<Constraint>();
        lv.add(new Constraint(menuView, Constraint.LEFT, mainView, Constraint.LEFT, 1.0, 0.0, 999));

        lh = new ArrayList<Constraint>();
        lh.add(new Constraint(menuView, Constraint.RIGHT, mainView, Constraint.LEFT, 1.0, 0.0, 999));

        mainView.addConstraint(new Constraint(menuView, Constraint.TOP, mainView, Constraint.TOP, 1.0, 0.0, 999));
        mainView.addConstraint(new Constraint(menuView, Constraint.BOTTOM, mainView, Constraint.BOTTOM, 1.0, 0.0, 999));
        mainView.addConstraint(new Constraint(menuView, Constraint.WIDTH, null, Constraint.NONE, 1.0, 200.0, 999));

        // Menu Item 01

        Button menuItem01 = new Button(this);
        menuItem01.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { menuSelect01(v); } });
        menuItem01.setBackgroundColor(Color.BLUE);
        menuItem01.setText("Example01");
        menuItem01.setTextColor(Color.WHITE);
        //menuItem01.setTextAlignment(Button.TEXT_ALIGNMENT_CENTER);

        menuView.addView(menuItem01);

        menuView.addConstraint(new Constraint(menuItem01, Constraint.LEFT, menuView, Constraint.LEFT, 1.0, 8.0, 999));
        menuView.addConstraint(new Constraint(menuItem01, Constraint.TOP, menuView, Constraint.TOP, 1.0, 8.0, 999));
        menuView.addConstraint(new Constraint(menuItem01, Constraint.RIGHT, menuView, Constraint.RIGHT, 1.0, -8.0, 999));
        menuView.addConstraint(new Constraint(menuItem01, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 50.0, 999));

        // Menu Item 02

        Button menuItem02 = new Button(this);
        menuItem02.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { menuSelect02(v); } });
        menuItem02.setBackgroundColor(Color.GRAY);
        menuItem02.setText("Example02");
        menuItem02.setTextColor(Color.WHITE);
        //menuItem02.setTextAlignment(Button.TEXT_ALIGNMENT_CENTER);

        menuView.addView(menuItem02);

        menuView.addConstraint(new Constraint(menuItem02, Constraint.TOP, menuItem01, Constraint.BOTTOM, 1.0, 8.0, 998));
        menuView.addConstraint(new Constraint(menuItem02, Constraint.LEFT, menuView, Constraint.LEFT, 1.0, 8.0, 999));
        menuView.addConstraint(new Constraint(menuItem02, Constraint.RIGHT, menuView, Constraint.RIGHT, 1.0, -8.0, 999));
        menuView.addConstraint(new Constraint(menuItem02, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 50.0, 999));

        // Menu Item 03

        Button menuItem03 = new Button(this);
        menuItem03.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { menuSelect03(v); } });
        menuItem03.setBackgroundColor(Color.BLUE);
        menuItem03.setText("Game");
        menuItem03.setTextColor(Color.WHITE);
        //menuItem03.setTextAlignment(Button.TEXT_ALIGNMENT_CENTER);

        menuView.addView(menuItem03);

        menuView.addConstraint(new Constraint(menuItem03, Constraint.TOP, menuItem02, Constraint.BOTTOM, 1.0, 8.0, 997));
        menuView.addConstraint(new Constraint(menuItem03, Constraint.LEFT, menuView, Constraint.LEFT, 1.0, 8.0, 999));
        menuView.addConstraint(new Constraint(menuItem03, Constraint.RIGHT, menuView, Constraint.RIGHT, 1.0, -8.0, 999));
        menuView.addConstraint(new Constraint(menuItem03, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 50.0, 999));

        // Layout 01

        layoutView01 = new ConstraintView(this);
        layoutView01.setBackgroundColor(Color.RED);
        mainView.addView(layoutView01);

        mainView.addConstraint(new Constraint(layoutView01, Constraint.TOP, menuView, Constraint.TOP, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView01, Constraint.LEFT, menuView, Constraint.RIGHT, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView01, Constraint.BOTTOM, menuView, Constraint.BOTTOM, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView01, Constraint.WIDTH, mainView, Constraint.WIDTH, 1.0, 0.0, 998));

        {
            Button menuButton = new Button(this);
            menuButton.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { showMenu(v); } });
            menuButton.setBackgroundColor(Color.BLACK);
            menuButton.setText("Menu");
            menuButton.setTextColor(Color.WHITE);
            layoutView01.addView(menuButton);

            layoutView01.addConstraint(new Constraint(menuButton, Constraint.TOP, layoutView01, Constraint.TOP, 1.0, 8.0, 999));
            layoutView01.addConstraint(new Constraint(menuButton, Constraint.LEFT, layoutView01, Constraint.LEFT, 1.0, 8.0, 999));
            layoutView01.addConstraint(new Constraint(menuButton, Constraint.WIDTH, null, Constraint.NONE, 1.0, 60.0, 999));
            layoutView01.addConstraint(new Constraint(menuButton, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 60.0, 999));
        }

        // Layout 02

        layoutView02 = new ConstraintView(this);
        layoutView02.setBackgroundColor(Color.CYAN);
        mainView.addView(layoutView02);

        mainView.addConstraint(new Constraint(layoutView02, Constraint.TOP, menuView, Constraint.TOP, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView02, Constraint.LEFT, menuView, Constraint.RIGHT, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView02, Constraint.BOTTOM, menuView, Constraint.BOTTOM, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView02, Constraint.WIDTH, mainView, Constraint.WIDTH, 1.0, 0.0, 998));

        {
            Button menuButton = new Button(this);
            menuButton.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { showMenu(v); } });
            menuButton.setBackgroundColor(Color.BLACK);
            menuButton.setText("Menu");
            menuButton.setTextColor(Color.WHITE);
            layoutView02.addView(menuButton);

            layoutView02.addConstraint(new Constraint(menuButton, Constraint.TOP, layoutView02, Constraint.TOP, 1.0, 8.0, 999));
            layoutView02.addConstraint(new Constraint(menuButton, Constraint.LEFT, layoutView02, Constraint.LEFT, 1.0, 8.0, 999));
            layoutView02.addConstraint(new Constraint(menuButton, Constraint.WIDTH, null, Constraint.NONE, 1.0, 60.0, 999));
            layoutView02.addConstraint(new Constraint(menuButton, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 60.0, 999));
        }

        // Layout 03

        layoutView03 = new ConstraintView(this);
        layoutView03.setBackgroundColor(Color.GREEN);
        mainView.addView(layoutView03);

        mainView.addConstraint(new Constraint(layoutView03, Constraint.TOP, menuView, Constraint.TOP, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView03, Constraint.LEFT, menuView, Constraint.RIGHT, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView03, Constraint.BOTTOM, menuView, Constraint.BOTTOM, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(layoutView03, Constraint.WIDTH, mainView, Constraint.WIDTH, 1.0, 0.0, 998));

        // Layout Hidden

        hiddenView = new Button(this);
        hiddenView.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { showLayout(v); } });
        hiddenView.setBackgroundColor(Color.BLACK);
        hiddenView.setAlpha(0.5f);
        mainView.addView(hiddenView);

        mainView.addConstraint(new Constraint(hiddenView, Constraint.TOP, menuView, Constraint.TOP, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(hiddenView, Constraint.LEFT, menuView, Constraint.RIGHT, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(hiddenView, Constraint.BOTTOM, menuView, Constraint.BOTTOM, 1.0, 0.0, 998));
        mainView.addConstraint(new Constraint(hiddenView, Constraint.WIDTH, mainView, Constraint.WIDTH, 1.0, 0.0, 998));

        // Небольшое отличие от iOS изза того, что подобной реализации в Андроиде нет и ее приходится имитировать
        {
            lv.add(new Constraint(hiddenView, Constraint.ALPHA, null, Constraint.NONE, 1.0, 0.5, 999));
            lh.add(new Constraint(hiddenView, Constraint.ALPHA, null, Constraint.NONE, 1.0, 0.0, 999));

            mainView.addConstraints(lv);
        }

        layoutView01.setVisibility(View.VISIBLE);
        layoutView02.setVisibility(View.GONE);
        layoutView03.setVisibility(View.GONE);

        // Media Controls
        {
            Button volumeDownButton = new Button(this);
            volumeDownButton.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { volumeDown(v); } });
            volumeDownButton.setBackgroundColor(Color.DKGRAY);
            volumeDownButton.setText("-");
            volumeDownButton.setTextColor(Color.WHITE);
            mainView.addView(volumeDownButton);

            mainView.addConstraint(new Constraint(volumeDownButton, Constraint.LEFT, mainView, Constraint.LEFT, 1.0, 8.0, 999));
            mainView.addConstraint(new Constraint(volumeDownButton, Constraint.TOP, mainView, Constraint.CENTERY, 1.0, -25.0, 999));
            mainView.addConstraint(new Constraint(volumeDownButton, Constraint.WIDTH, null, Constraint.NONE, 1.0, 50.0, 999));
            mainView.addConstraint(new Constraint(volumeDownButton, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 50.0, 999));

            Button volumeUpButton = new Button(this);
            volumeUpButton.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { volumeUp(v); } });
            volumeUpButton.setBackgroundColor(Color.DKGRAY);
            volumeUpButton.setText("+");
            volumeUpButton.setTextColor(Color.WHITE);
            mainView.addView(volumeUpButton);

            mainView.addConstraint(new Constraint(volumeUpButton, Constraint.LEFT, volumeDownButton, Constraint.RIGHT, 1.0, 8.0, 998));
            mainView.addConstraint(new Constraint(volumeUpButton, Constraint.TOP, mainView, Constraint.CENTERY, 1.0, -25.0, 999));
            mainView.addConstraint(new Constraint(volumeUpButton, Constraint.WIDTH, null, Constraint.NONE, 1.0, 50.0, 999));
            mainView.addConstraint(new Constraint(volumeUpButton, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 50.0, 999));

            Button mixerCheckButton = new Button(this);
            mixerCheckButton.setOnClickListener(new Button.OnClickListener() { @Override public void onClick(View v) { mixerCheck(v); } });
            mixerCheckButton.setBackgroundColor(Color.DKGRAY);
            mixerCheckButton.setText("*");
            mixerCheckButton.setTextColor(Color.WHITE);
            mainView.addView(mixerCheckButton);

            mainView.addConstraint(new Constraint(mixerCheckButton, Constraint.LEFT, volumeUpButton, Constraint.RIGHT, 1.0, 8.0, 997));
            mainView.addConstraint(new Constraint(mixerCheckButton, Constraint.TOP, mainView, Constraint.CENTERY, 1.0, -25.0, 999));
            mainView.addConstraint(new Constraint(mixerCheckButton, Constraint.WIDTH, null, Constraint.NONE, 1.0, 50.0, 999));
            mainView.addConstraint(new Constraint(mixerCheckButton, Constraint.HEIGHT, null, Constraint.NONE, 1.0, 50.0, 999));
        }

        setContentView(mainView);

        /*
        sensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        List<Sensor> sensorList = sensorManager.getSensorList(Sensor.TYPE_ACCELEROMETER);
        if(sensorList.size() > 0){
            accelerometerPresent = true;
            accelerometerSensor = sensorList.get(0);
        } else{
            accelerometerPresent = false;
        }
        */

        /*
        RelativeLayout m_main = new RelativeLayout(this);
        m_view = OMView.OMJappsyView.create(this);
        m_view.setLayoutParams(new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, 600));
        m_main.addView(m_view);

        setContentView(m_main);
        */
    }

    private float volume = 1.0f;

    public void volumeDown(View sender) {
        if (omView != null) {
            volume -= 0.1f;
            if (volume < 0.0f) volume = 0.0f;
            omView.mixerVolume(volume);
        }
    }

    public void volumeUp(View sender) {
        if (omView != null) {
            volume += 0.1f;
            if (volume > 1.0f) volume = 1.0f;
            omView.mixerVolume(volume);
        }
    }

    public void mixerCheck(View sender) {
        if (omView != null) {
            omView.isMixerPlaying();
        }
    }

    public void showLayout(View sender) {
        mainView.layoutIfNeeded();

        mainView.removeConstraints(lv);
        mainView.addConstraints(lh);

        mainView.animate(0.3, new ConstraintView.AnimationCallbacks() {
            @Override
            public void onStart() {

            }

            @Override
            public void onComplete(boolean finished) {
                hiddenView.setVisibility(View.GONE);
            }
        });

        if (layoutView03.getVisibility() == View.VISIBLE) {
            omView.updateState(OMView.OMVIEW_SHOW);
        }
    }

    public void showMenu(View sender) {
        mainView.layoutIfNeeded();

        mainView.removeConstraints(lh);
        mainView.addConstraints(lv);

        hiddenView.setVisibility(View.VISIBLE);

        mainView.animate(0.3, null);
    }

    public void menuSelect01(View sender) {
        layoutView01.setVisibility(View.VISIBLE);
        layoutView02.setVisibility(View.GONE);
        layoutView03.setVisibility(View.GONE);
        showLayout(sender);
    }

    public void menuSelect02(View sender) {
        layoutView01.setVisibility(View.GONE);
        layoutView02.setVisibility(View.VISIBLE);
        layoutView03.setVisibility(View.GONE);
        showLayout(sender);
    }

    public void menuSelect03(View sender) {
        if (omView == null) {
            String basePath = "https://dev03-om.jappsy.com/jappsy/"; // Demo server
            //String basePath = "https://om.jappsy.com/jappsy/"; // Production server
            String token = "e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3";
            String sessid = "8ea5f70b15263872760d7e14ce8e579a";
            String devid = "";
            String locale = "RU";

            omView = new OMView(this, basePath, token, sessid, devid, locale, onCloseCallback);
            layoutView03.addView(omView);

            layoutView03.addConstraint(new Constraint(omView, Constraint.LEFT, layoutView03, Constraint.LEFT, 1.0, 0.0, 999));
            layoutView03.addConstraint(new Constraint(omView, Constraint.TOP, layoutView03, Constraint.TOP, 1.0, 0.0, 999));
            layoutView03.addConstraint(new Constraint(omView, Constraint.RIGHT, layoutView03, Constraint.RIGHT, 1.0, 0.0, 999));
            layoutView03.addConstraint(new Constraint(omView, Constraint.BOTTOM, layoutView03, Constraint.BOTTOM, 1.0, 0.0, 999));

            //omView.onStart();
            omView.updateState(OMView.OMVIEW_RUN);
        }

        layoutView01.setVisibility(View.GONE);
        layoutView02.setVisibility(View.GONE);
        layoutView03.setVisibility(View.VISIBLE);
        showLayout(sender);
    }

    private OMView.OMViewCloseCallback onCloseCallback = new OMView.OMViewCloseCallback() {
        @Override public void onClose() {
            JappsyMain.this.showMenu(null);
        }
    };

    @Override public void onBackPressed() {
        moveTaskToBack(true);
    }

    @Override public boolean onKeyDown(int keyCode, KeyEvent event) {
        return keyCode == KeyEvent.KEYCODE_MENU || super.onKeyDown(keyCode, event);
    }

    private SensorEventListener accelerometerListener = new SensorEventListener(){
        @Override public void onAccuracyChanged(Sensor arg0, int arg1) {
        }

        @Override public void onSensorChanged(SensorEvent arg0) {
            float z_value = arg0.values[2];
            if (z_value >= 0) {
                if (lastSensorState != JappsyView.ORIENTATION_FACEUP) {
                    lastSensorState = JappsyView.ORIENTATION_FACEUP;
                    if (omView != null) {
                        omView.orientationChanged(JappsyView.ORIENTATION_FACEUP);
                    }
                }
            } else {
                if (lastSensorState != JappsyView.ORIENTATION_FACEDOWN) {
                    lastSensorState = JappsyView.ORIENTATION_FACEDOWN;
                    if (omView != null) {
                        omView.orientationChanged(JappsyView.ORIENTATION_FACEDOWN);
                    }
                }
            }
        }
    };

    @Override protected void onResume() {
        Log.d(TAG, "onResume");

        super.onResume();
        if (omView != null) {
            omView.onResume();
        }

        if (accelerometerPresent) {
            sensorManager.registerListener(accelerometerListener, accelerometerSensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    @Override protected void onPause() {
        Log.d(TAG, "onPause");

        if (omView != null) {
            omView.onPause();
        }
        super.onPause();

        if (accelerometerPresent) {
            sensorManager.unregisterListener(accelerometerListener);
        }
    }

    @Override public void onConfigurationChanged(Configuration newConfig) {
        Log.d(TAG, "onConfigurationChanged");
        super.onConfigurationChanged(newConfig);

        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            if (omView != null) {
                omView.orientationChanged(JappsyView.ORIENTATION_LANDSCAPE);
            }
            omView.enterFullscreen();
        } else {
            if (omView != null) {
                omView.orientationChanged(JappsyView.ORIENTATION_PORTRAIT);
            }
            omView.exitFullscreen();
        }
    }

    @Override protected void onDestroy() {
        Log.d(TAG, "onDestroy");

        super.onDestroy();
    }

}
