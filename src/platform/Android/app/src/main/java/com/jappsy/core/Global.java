package com.jappsy.core;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.os.Environment;
import android.preference.PreferenceManager;

public class Global {

    private static Global m_global = null;
    private static Application m_applicationContext = null;
    private static SharedPreferences m_settings = null;
    private static SQLite m_db = null;

    public static int m_dpi = 160;
    public static float m_scale = 1;
    public static float m_diag = 5;
    public static int m_pad = 8;
    public static int m_line = 48;

    public static synchronized Global getInstance() {
        if (m_global == null)
            m_global = new Global();
        return m_global;
    }

    public static synchronized Global getInstance(Application context) {
        if (m_global == null) {
            m_global = new Global();
        }
        if (m_settings == null) {
            m_settings = PreferenceManager.getDefaultSharedPreferences(context);
        }
        if (m_db == null) {
            m_db = new SQLite(context, Environment.getExternalStorageDirectory() + File.separator + "jappsy.db");
        }
        if (m_applicationContext == null) {
            m_applicationContext = context;

            //TODO: Init Cache - Cache.getInstance();
            Log.getInstance();

            m_global.initGlobals(context);
        }

        return m_global;
    }

    public static synchronized SharedPreferences getSharedPreferences() {
        return m_settings;
    }

    public static synchronized SQLite getDatabase() {
        return m_db;
    }

    public static synchronized Context getContext() {
        return m_applicationContext;
    }

    private synchronized void initGlobals(Application context) {
        String currentVersionName = "Unknown";
        try {
            currentVersionName = context.getPackageManager()
                    .getPackageInfo(context.getPackageName(), 0).versionName;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        String versionName = m_db.getString("Version", null);
        if ((versionName == null) || (!currentVersionName.equals(versionName))) {
            m_db.putString("Version", currentVersionName);
            //TODO: Wipe Cache - Cache.deleteCache();
        }
    }

    /////

    public static int parseInt(String val) {
        try {
            return Integer.parseInt(val);
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    public static double parseFloat(final String val) {
        if (val == null)
            return 0;
        try {
            return Double.parseDouble(val);
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    public static int parseInt(final String val, int radix) {
        try {
            return Integer.parseInt(val, radix);
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    public static String jsonDecode(final String val) {
        try {
            JSONObject o = new JSONObject("{\"value\":\"" + val + "\"}");
            return o.optString("value", val);
        } catch (JSONException e) {
            return val;
        }
    }

    public static String urlEncode(final String val) {
        try {
            return URLEncoder.encode(val, "utf-8");
        } catch (UnsupportedEncodingException e) {
            return "";
        }
    }

    /*
    public static final String md5(final String s) {
        final String MD5 = "MD5";
        try {
            // Create MD5 Hash
            MessageDigest digest = java.security.MessageDigest
                    .getInstance(MD5);
            digest.update(s.getBytes());
            byte messageDigest[] = digest.digest();

            // Create Hex String
            StringBuilder hexString = new StringBuilder();
            for (byte aMessageDigest : messageDigest) {
                String h = Integer.toHexString(0xFF & aMessageDigest);
                while (h.length() < 2)
                    h = "0" + h;
                hexString.append(h);
            }
            return hexString.toString();

        } catch (NoSuchAlgorithmException e) {
        }
        return "";
    }
    */

    // Streams

    private static final int BUFFER_IO_SIZE = 8000;

    private static long copyStream(final InputStream is, final OutputStream os) throws IOException {
        int size = 0;
        byte[] buf = new byte[256];
        int l;
        while ((l = is.read(buf)) >= 0) {
            os.write(buf, 0, l);
            size += l;
        }
        return size;
    }

    private static byte[] readStream(final InputStream is) {
        byte[] result = null;
        BufferedInputStream bis = null;
        BufferedOutputStream bos = null;
        try {
            if (is instanceof BufferedInputStream) {
                bis = (BufferedInputStream)is;
            } else {
                bis = new BufferedInputStream(is);
            }
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            bos = new BufferedOutputStream(baos, BUFFER_IO_SIZE);
            try {
                copyStream(bis, bos);
                bos.flush();

                if (baos.size() > 0) {
                    result = baos.toByteArray();
                }
            } catch (OutOfMemoryError e) {
            } catch (FileNotFoundException e) {
            } catch (IOException e) {
            }
        } finally {
            if (bos != null)
                try {
                    bos.close();
                } catch (IOException e) {
                }
            if (bis != null)
                try {
                    bis.close();
                } catch (IOException e) {
                }
        }
        return result;
    }

    public static String getRawString(final String name) {
        Resources res = getContext().getResources();
        InputStream is = res.openRawResource(res.getIdentifier(name, "raw", getContext().getPackageName()));
        byte[] buffer = readStream(is);
        if (buffer != null) {
            return new String(buffer);
        }
        return null;
    }

    public static InputStream getRawStream(final String name) {
        Resources res = getContext().getResources();
        return res.openRawResource(res.getIdentifier(name, "raw", getContext().getPackageName()));
    }

    public static File getDiskCacheDir(final String uniqueName) {
        final String cachePath = getContext().getCacheDir().getPath();
        File f = new File(cachePath + File.separator + uniqueName);
        f.mkdirs();
        return f;
    }

    public static long saveStream(final File cacheDir, final String fileName, InputStream is) {
        long size = 0;
        File file = new File(cacheDir.getPath(), fileName);
        BufferedInputStream bis = null;
        BufferedOutputStream bos = null;
        try {
            if (is instanceof BufferedInputStream) {
                bis = (BufferedInputStream)is;
            } else {
                bis = new BufferedInputStream(is);
            }
            try {
                bos = new BufferedOutputStream(new FileOutputStream(file), BUFFER_IO_SIZE);
                size = copyStream(bis, bos);
                bos.flush();
            } catch (FileNotFoundException e) {
            } catch (IOException e) {
            }
        } finally {
            if (bos != null)
                try {
                    bos.close();
                } catch (IOException e) {
                }
            if (bis != null)
                try {
                    bis.close();
                } catch (IOException e) {
                }
        }
        return size;
    }

    public static InputStream openStream(final File cacheDir, final String fileName) {
        File file = new File(cacheDir.getPath(), fileName);
        if (file.exists()) {
            try {
                return new BufferedInputStream(new FileInputStream(file));
            } catch (FileNotFoundException e) {
            }
        }
        return null;
    }

    static {
        System.loadLibrary("jappsy-jni");
    }

}
