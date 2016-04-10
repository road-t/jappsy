package com.jappsy.core;

import java.util.Date;
import java.util.Formatter;

public class Log {

    private static Log m_log = null;

    public static synchronized Log getInstance() {
        if (m_log == null)
            m_log = new Log();
        return m_log;
    }

    public static String[] lastLog = new String[15];
    private static void log(String text, int type) {
        long time = System.currentTimeMillis();
        Date dt = new Date();
        int hours = dt.getHours();
        int minutes = dt.getMinutes();
        int seconds = dt.getSeconds();
        int ms = (int)(time % 1000);

        Formatter f = new Formatter();
        f.format("%02d:%02d:%02d.%03d > %s", hours, minutes, seconds, ms, text);
        String debugText = f.toString();
        f.close();

        switch (type) {
            case 0:
                android.util.Log.v("Verbose", debugText);
                break;
            case 1:
                android.util.Log.d("Debug", debugText);
                break;
            case 2:
                android.util.Log.i("Info", debugText);
                break;
            case 3:
                android.util.Log.w("Warning", debugText);
                break;
            case 4:
                android.util.Log.e("Error", debugText);
                break;
        }

        for (int i = 14; i > 0; i--) {
            lastLog[i] = lastLog[i-1];
        }
        lastLog[0] = debugText;
    }

    public static void verbose(String text) {
        log(text, 0);
    }

    public static void debug(String text) {
        log(text, 1);
    }

    public static void info(String text) {
        log(text, 2);
    }

    public static void warning(String text) {
        log(text, 3);
    }

    public static void error(String text) {
        log(text, 4);
    }

}
