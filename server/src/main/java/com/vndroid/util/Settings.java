package com.vndroid.util;

import com.vndroid.wrappers.ContentProvider;
import com.vndroid.wrappers.ServiceManager;

public final class Settings {

    public static final String TABLE_SYSTEM = ContentProvider.TABLE_SYSTEM;
    public static final String TABLE_SECURE = ContentProvider.TABLE_SECURE;
    public static final String TABLE_GLOBAL = ContentProvider.TABLE_GLOBAL;

    private Settings() {
        /* not instantiable */
    }

    private static ContentProvider createProvider(String method, String table, String key, String value) throws SettingsException {
        ContentProvider provider = ServiceManager.getActivityManager().createSettingsProvider();
        if (provider == null) {
            // The settings provider is not available
            throw new SettingsException(method, table, key, value, null);
        }
        return provider;
    }

    public static String getValue(String table, String key) throws SettingsException {
        try (ContentProvider provider = createProvider("get", table, key, null)) {
            return provider.getValue(table, key);
        }
    }

    public static void putValue(String table, String key, String value) throws SettingsException {
        try (ContentProvider provider = createProvider("put", table, key, value)) {
            provider.putValue(table, key, value);
        }

    }

    public static String getAndPutValue(String table, String key, String value) throws SettingsException {
        try (ContentProvider provider = createProvider("get/put", table, key, value)) {
            String oldValue = provider.getValue(table, key);
            if (!value.equals(oldValue)) {
                provider.putValue(table, key, value);
            }
            return oldValue;
        }
    }
}
