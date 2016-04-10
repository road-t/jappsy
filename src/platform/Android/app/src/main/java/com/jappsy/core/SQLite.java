package com.jappsy.core;

import java.io.IOException;

import com.jappsy.core.util.Base64;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class SQLite extends SQLiteOpenHelper {

    private static final int DATABASE_VERSION = 1;
    private static final String DATABASE_CREATE =
            "CREATE TABLE IF NOT EXISTS `config` (" +
                    "`config_name` text NOT NULL, " +
                    "`config_value` text NOT NULL, " +
                    "PRIMARY KEY (`config_name`) " +
                    ");";

    public SQLite(Context context, String dbname) {
        super(context, dbname, null, DATABASE_VERSION);
    }

    @Override public void onCreate(SQLiteDatabase db) {
        db.execSQL(DATABASE_CREATE);
    }

    @Override public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        //db.execSQL("DROP TABLE IF EXISTS notes");
        onCreate(db);
    }

    @SuppressLint("Override")
    public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        onCreate(db);
    }

    public void wipeDatabase() {
        SQLiteDatabase db = getWritableDatabase();
        db.execSQL("DROP TABLE IF EXISTS notes");
        db.execSQL(DATABASE_CREATE);
        db.close();
    }

    public void remove(String keyName) {
        String name = DatabaseUtils.sqlEscapeString(keyName);
        SQLiteDatabase db = getWritableDatabase();
        db.execSQL("DELETE FROM `config` WHERE `config_name` LIKE " + name + ";");
        db.close();
    }

    public void putString(String keyName, String keyValue) {
        String name = DatabaseUtils.sqlEscapeString(keyName);
        String value = DatabaseUtils.sqlEscapeString(keyValue);
        SQLiteDatabase db = getWritableDatabase();
        db.execSQL("UPDATE `config` SET `config_value` = " + value + " WHERE `config_name` LIKE " + name + ";");
        db.execSQL("INSERT OR IGNORE INTO `config` (`config_name`, `config_value`) VALUES (" + name + ", " + value + ");");
        db.close();
    }

    public void putInteger(String keyName, int keyValue) {
        putString(keyName, Integer.toString(keyValue));
    }

    public void putBoolean(String keyName, boolean keyValue) {
        putString(keyName, Boolean.toString(keyValue));
    }

    public void putBlob(String keyName, byte[] keyValue) {
        if (keyValue == null) {
            putString(keyName, "");
        } else {
            try {
                putString(keyName, Base64.encodeBytes(keyValue, Base64.GZIP));
            } catch (IOException e) {
                //e.printStackTrace();
            }
        }
    }

    public String getString(String keyName, String defaultValue) {
        String value = defaultValue;
        String name = DatabaseUtils.sqlEscapeString(keyName);
        SQLiteDatabase db = getWritableDatabase();
        Cursor c = db.rawQuery("SELECT * FROM `config` WHERE `config_name` LIKE " + name, null);
        if (c != null) {
            if (c.moveToFirst()) {
                do {
                    value = c.getString(c.getColumnIndex("config_value"));
                } while (c.moveToNext());
            }
        }
        c.close();
        db.close();
        return value;
    }

    public int getInteger(String keyName, int defaultValue) {
        String value = getString(keyName, null);
        if (value == null) {
            return defaultValue;
        }
        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }

    public boolean getBoolean(String keyName, boolean defaultValue) {
        String value = getString(keyName, null);
        if (value == null) {
            return defaultValue;
        }
        return Boolean.parseBoolean(value);
    }

    public byte[] getBlob(String keyName, byte[] defaultValue) {
        String value = getString(keyName, null);
        if (value == null) {
            return defaultValue;
        }
        try {
            return Base64.decode(value);
        } catch (IOException e) {
            return defaultValue;
        }
    }

}
