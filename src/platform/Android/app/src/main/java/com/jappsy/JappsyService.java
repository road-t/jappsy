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

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.ConditionVariable;
import android.os.IBinder;

import com.jappsy.core.Log;

public class JappsyService extends Service {
    private static final String ACTION_FOREGROUND = "com.jappsy.FOREGROUND";
    private static final String ACTION_BACKGROUND = "com.jappsy.BACKGROUND";
    private static final int SERVICE_NOTIFICATION_ID = 1;

    private static final Class<?>[] m_SetForegroundSignature = new Class[] {boolean.class};
    private static final Class<?>[] m_StartForegroundSignature = new Class[] {int.class, Notification.class};
    private static final Class<?>[] m_StopForegroundSignature = new Class[] {boolean.class};

    private NotificationManager m_NM;
    private Method m_SetForeground;
    private Method m_StartForeground;
    private Method m_StopForeground;
    private Object[] m_SetForegroundArgs = new Object[1];
    private Object[] m_StartForegroundArgs = new Object[2];
    private Object[] m_StopForegroundArgs = new Object[1];

    @Override public void onStart(Intent intent, int startId) {
        handleCommand(intent);
    }

    @Override public int onStartCommand(Intent intent, int flags, int startId) {
        handleCommand(intent);
        return START_STICKY;
    }

    private boolean m_started = false;

    void handleCommand(Intent intent) {
        if (intent != null) {
            if (ACTION_FOREGROUND.equals(intent.getAction())) {
                if (!m_started) {
                    PendingIntent contentIntent = PendingIntent.getActivity(this, 0, new Intent(this, JappsyMain.class), 0);

                    Notification.Builder builder = new Notification.Builder(this)
                            .setAutoCancel(false)
                                    //.setTicker("this is ticker text")
                            .setContentTitle(getString(R.string.app_name))
                                    //.setContentText("")
                            .setSmallIcon(R.mipmap.ic_launcher)
                            .setContentIntent(contentIntent)
                            .setOngoing(true);
                                    //.setSubText("This is subtext...")   //API level 16
                                    //.setNumber(100)
                                    //.build();

                    Notification notification = builder.getNotification();

                    startForegroundCompat(SERVICE_NOTIFICATION_ID, notification);
                    JappsyStartup.onStartup(this);

                    m_started = true;
                }
            } else if (ACTION_BACKGROUND.equals(intent.getAction())) {
                stopForegroundCompat(SERVICE_NOTIFICATION_ID);
            }
        }
    }

    void startForegroundCompat(int id, Notification notification) {
        if (m_StartForeground != null) {
            m_StartForegroundArgs[0] = id;//Integer.valueOf(id);
            m_StartForegroundArgs[1] = notification;
            invokeMethod(m_StartForeground, m_StartForegroundArgs);
            return;
        }

        m_SetForegroundArgs[0] = Boolean.TRUE;
        invokeMethod(m_SetForeground, m_SetForegroundArgs);
        m_NM.notify(id, notification);
    }

    void stopForegroundCompat(int id) {
        if (m_StopForeground != null) {
            m_StopForegroundArgs[0] = Boolean.TRUE;
            invokeMethod(m_StopForeground, m_StopForegroundArgs);
            return;
        }

        m_NM.cancel(id);
        m_SetForegroundArgs[0] = Boolean.FALSE;
        invokeMethod(m_SetForeground, m_SetForegroundArgs);
    }

    void invokeMethod(Method method, Object[] args) {
        try {
            method.invoke(this, args);
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }

    private ConditionVariable m_ConditionStop;
    private ConditionVariable m_ConditionStoped;
    private Thread m_serviceThread;

    private BroadcastReceiver m_receiver;

    @Override public void onCreate() {
        m_NM = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        try {
            m_StartForeground = getClass().getMethod("startForeground", m_StartForegroundSignature);
            m_StopForeground = getClass().getMethod("stopForeground", m_StopForegroundSignature);
        } catch (NoSuchMethodException e1) {
            m_StartForeground = m_StopForeground = null;
            try {
                m_SetForeground = getClass().getMethod("setForeground", m_SetForegroundSignature);
            } catch (NoSuchMethodException e2) {
                throw new IllegalStateException("OS doesn't have Service.startForeground OR Service.setForeground!");
            }
        }

        /**********************************/

        Log.debug("Service > onCreate");

        IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        m_receiver = new JappsyStartup();
        registerReceiver(m_receiver, filter);

        m_ConditionStop = new ConditionVariable(false);
        m_ConditionStoped = new ConditionVariable(false);
        m_serviceThread = new Thread(m_Task);
        m_serviceThread.start();
    }

    @Override public void onDestroy() {
        if (m_receiver != null) {
            unregisterReceiver(m_receiver);
            m_receiver = null;
        }

        if (!m_started) {
            m_ConditionStop.open();
            m_ConditionStoped.block();

            m_started = false;
        }

        stopForegroundCompat(SERVICE_NOTIFICATION_ID);

        Log.debug("Service > onDestroy");
    }

    @Override public IBinder onBind(Intent intent) {
        return null;
    }

    /*
    private Notification createNotification(int iconResId, String from, String message, boolean showText, boolean autoCancel, String sound, long[] vibrate, Class<?> cls) {
        CharSequence m_from = from;
        CharSequence m_message = message;

        PendingIntent contentIntent;
        if (cls != null) {
        	contentIntent = PendingIntent.getActivity(this, 0, new Intent(this, cls), 0);
        } else {
        	contentIntent = PendingIntent.getActivity(this, 0, new Intent(), PendingIntent.FLAG_UPDATE_CURRENT);
        }

        Notification notif;
        if (showText) {
	        Formatter f = new Formatter();
	        f.format("%s: %s", m_from, m_message);
	        String tickerText = f.toString();

	        notif = new Notification(iconResId, tickerText, System.currentTimeMillis());
        } else {
	        notif = new Notification(iconResId, null, System.currentTimeMillis());
        }

        if (autoCancel) {
        	notif.flags = Notification.FLAG_AUTO_CANCEL;
        }

        if (vibrate != null) {
        	notif.defaults = Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE;
        }

      	notif.setLatestEventInfo(this, m_from, m_message, contentIntent);

        // after a 100ms delay, vibrate for 250ms, pause for 100 ms and
        // then vibrate for 500ms.
        if (vibrate != null) {
        	notif.vibrate = vibrate;//new long[] { 100, 250, 100, 500};
        }

        if (sound != null) {
        	notif.sound = Uri.parse("android.resource://" + this.getPackageName() + sound);
        }

        return notif;
    }
    */

    /**********************************/

    public static void startForegroundService(Context context) {
        Intent intent = new Intent(JappsyService.ACTION_FOREGROUND);
        intent.setClass(context, JappsyService.class);
        context.startService(intent);
    }

    public static void startBackgroundService(Context context) {
        Intent intent = new Intent(JappsyService.ACTION_BACKGROUND);
        intent.setClass(context, JappsyService.class);
        context.startService(intent);
    }

    public static void stopService(Context context) {
        context.stopService(new Intent(context, JappsyService.class));
    }

    /**********************************/

    private Runnable m_Task = new Runnable() {
        public void run() {
            Log.debug("Service > Thread Begin");
            while (true) {
                // TODO: SERVICE

                if (m_ConditionStop.block(100)) break;
            }
            Log.debug("Service > Thread End");
            m_ConditionStoped.open();
            JappsyService.this.stopSelf();
        }
    };

    @Override public void onConfigurationChanged(Configuration newConfig) {
        Log.debug("Service > onConfigurationChanged");
        super.onConfigurationChanged(newConfig);
    }

    @Override public void onTaskRemoved(Intent rootIntent) {
        Log.debug("Service > onTaskRemoved");

        Jappsy.free();

        /* EXAMPLE SERVICE RESTART ON APP CRASH
            Intent restartServiceIntent = new Intent(getApplicationContext(), this.getClass());
            restartServiceIntent.setPackage(getPackageName());

            PendingIntent restartServicePendingIntent = PendingIntent.getService(getApplicationContext(), 1, restartServiceIntent, PendingIntent.FLAG_ONE_SHOT);
            AlarmManager alarmService = (AlarmManager) getApplicationContext().getSystemService(Context.ALARM_SERVICE);
            alarmService.set(
                    AlarmManager.ELAPSED_REALTIME,
                    SystemClock.elapsedRealtime() + 1000,
                    restartServicePendingIntent);
        */

        super.onTaskRemoved(rootIntent);
    }

}
