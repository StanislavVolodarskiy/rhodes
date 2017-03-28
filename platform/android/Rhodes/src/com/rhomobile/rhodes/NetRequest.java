package com.rhomobile.rhodes;

import java.util.Map;

import com.rhomobile.rhodes.Logger;

public class NetRequest
{
    private static final String TAG = NetRequest.class.getSimpleName();
    private static void ERROR(String    msg) { Logger.E(TAG, msg); }
    private static void INFO (String    msg) { Logger.I(TAG, msg); }
    private static void ERROR(Throwable ex ) { Logger.E(TAG, ex ); }

    public boolean doRequest(String method, String url, String body, Map<String, String> headers)
    {
        INFO("method is [" + method + "]");
        INFO("url is [" + url + "]");
        INFO("body is [" + body + "]");

        return method.length() % 2 == 0;
    }
}
