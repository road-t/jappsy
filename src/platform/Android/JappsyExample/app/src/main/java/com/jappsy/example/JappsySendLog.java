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
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;

public class JappsySendLog extends Activity implements View.OnClickListener {
    private static final String m_sendLogFile = "sendlog.txt";

    private static String getLogPath(Context context) {
        return Environment.getExternalStorageDirectory() + File.separator + context.getString(R.string.app_name) + File.separator;
    }

    @Override public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE); // make a dialog without a titlebar
        setFinishOnTouchOutside(false); // prevent users from dismissing the dialog by tapping outside

        File file = new File(getLogPath(this) + m_sendLogFile);
        StringBuilder text = new StringBuilder();

        try {
            BufferedReader br = new BufferedReader(new FileReader(file));
            String line;

            while ((line = br.readLine()) != null) {
                text.append(line);
                text.append('\n');
            }
            br.close();
        }
        catch (IOException e) {
            //You'll need to add proper error handling here
        }

        setContentView(R.layout.sendlog);

        TextView tv = (TextView)findViewById(R.id.crashlog);
        tv.setText(text.toString());

        Button bt = (Button)findViewById(R.id.sendlog);
        bt.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        sendLogFile();
        finish();
    }

    private void sendLogFile () {
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent.setType ("plain/text");
        intent.putExtra (Intent.EXTRA_EMAIL, new String[] {"support@jappsy.com"});
        intent.putExtra (Intent.EXTRA_SUBJECT, "Jappsy crash log file");
        intent.putExtra (Intent.EXTRA_STREAM, Uri.parse("file://" + getLogPath(this) + m_sendLogFile));
        intent.putExtra (Intent.EXTRA_TEXT, "Crash log file attached."); // do this so some email clients don't complain about empty body.
        startActivity(intent);
    }

    public static final String extractLogToFile(Context context) {
        PackageManager manager = context.getPackageManager();
        PackageInfo info = null;
        String packageName = context.getPackageName();
        try {
            info = manager.getPackageInfo(packageName, 0);
        } catch (PackageManager.NameNotFoundException e2) {
        }
        String model = Build.MODEL;
        if (!model.startsWith(Build.MANUFACTURER))
            model = Build.MANUFACTURER + " " + model;

        // Make file name - file must be saved to external storage or it wont be readable by
        // the email app.
        String path = getLogPath(context);
        File filePath = new File(path);
        filePath.mkdirs();

        String fullName = path + m_sendLogFile;

        // Extract to file.
        File file = new File (fullName);
        InputStreamReader reader = null;
        FileWriter writer = null;
        try {
            // For Android 4.0 and earlier, you will get all app's log output, so filter it to
            // mostly limit it to your app's output.  In later versions, the filtering isn't needed.
            String cmd = (Build.VERSION.SDK_INT <= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) ?
                    "logcat -d -v time " + packageName + ":v dalvikvm:v System.err:v *:s" :
                    "logcat -d -v time";

            // get input stream
            Process process = Runtime.getRuntime().exec(cmd);
            reader = new InputStreamReader (process.getInputStream());

            // write output stream
            writer = new FileWriter (file);
            writer.write ("Android version: " +  Build.VERSION.SDK_INT + "\n");
            writer.write ("Device: " + model + "\n");
            writer.write ("App version: " + (info == null ? "(null)" : info.versionCode) + "\n");

            char[] buffer = new char[10000];
            do
            {
                int n = reader.read (buffer, 0, buffer.length);
                if (n == -1)
                    break;
                writer.write (buffer, 0, n);
            } while (true);

            reader.close();
            writer.close();
        }
        catch (IOException e) {
            if (writer != null)
                try { writer.close(); } catch (IOException e1) {}
            if (reader != null)
                try { reader.close(); } catch (IOException e1) {}

            // You might want to write a failure message to the log here.
            return null;
        }

        return fullName;
    }

}
