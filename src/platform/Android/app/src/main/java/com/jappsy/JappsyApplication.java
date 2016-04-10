/*
 * Copyright (C) 2016 The Jappsy Open Source Project
 *
 * Project Web Page http://jappsy.com
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

import android.app.AlarmManager;
import android.app.Application;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.SystemClock;

import com.jappsy.core.Global;
import com.jappsy.core.Log;

public class JappsyApplication extends Application {
    @Override public void onCreate() {
        super.onCreate();

        if (Jappsy.initialize())
            Jappsy.m_initialized = true;

        Global.getInstance(JappsyApplication.this);
        Log.debug("Application > Start");

        Thread.setDefaultUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
            @Override public void uncaughtException(Thread thread, Throwable e) {
                e.printStackTrace(); // not all Android versions will print the stack trace automatically

                Jappsy.free();
                Log.debug("Application > Exit");

                if (JappsySendLog.extractLogToFile(getApplicationContext()) == null)
                    System.exit(1);

                Intent intent = new Intent();
                intent.setAction("com.jappsy.SEND_LOG");
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);

                PendingIntent pendingIntent = PendingIntent.getActivity(getApplicationContext(), 1, intent, PendingIntent.FLAG_CANCEL_CURRENT);
                AlarmManager mgr = (AlarmManager) getApplicationContext().getSystemService(Context.ALARM_SERVICE);
                mgr.set(AlarmManager.ELAPSED_REALTIME,
                        SystemClock.elapsedRealtime() + 100,
                        pendingIntent);

                System.exit(0);
            }
        });
    }
}