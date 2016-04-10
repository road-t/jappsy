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

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.jappsy.core.Log;

public class JappsyStartup extends BroadcastReceiver {

    @Override public void onReceive(Context context, Intent intent) {
        if(intent.getAction() != null) {
            String action = intent.getAction();
            switch (action) {
                case Intent.ACTION_BOOT_COMPLETED:
                    JappsyService.startForegroundService(context);
                    Log.debug("Receiver > onBootCompleted");
                    break;
                case Intent.ACTION_USER_PRESENT:
                    JappsyService.startForegroundService(context);
                    onStartup(context);
                    Log.debug("Receiver > onUserPresent");
                    break;
                case Intent.ACTION_SCREEN_OFF:
                    //Global.getInstance().m_sleep = true;
                    Log.debug("Receiver > onScreenOff");
                    break;
                case Intent.ACTION_SCREEN_ON:
                    //Global.getInstance().m_sleep = false;
                    onStartup(context);
                    Log.debug("Receiver > onScreenOn");
                    break;
                case Intent.ACTION_SHUTDOWN:
                    //Global.getInstance().m_sleep = true;
                    Log.debug("Receiver > onShutdown");
                    break;
            }
        }
    }

    public static void onStartup(Context context) {
        /* EXAMPLE AUTOSTART ENABLED ON ANDROID STARTUP
            if (Config.TVBOX != false) {
                Bundle bundle = new Bundle();
                bundle.putBoolean("Startup", true);
                Intent i = new Intent(context, JappsyMain.class);
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                i.putExtras(bundle);
                context.startActivity(i);
            }
        */
    }

    public static boolean onJappsyMain(Activity activity) {
        JappsyService.startForegroundService(activity);

        /* EXAMPLE AUTOSTART DISABLED ON ANDROID STARTUP
            Bundle bundle = activity.getIntent().getExtras();
            if ((bundle != null) && (Config.TVBOX == false)) {
                boolean serviceOnly = bundle.getBoolean("Startup", false);
                if (serviceOnly) {
                    activity.finish();
                    return false;
                }
            }
        */

        return true;
    }

}
