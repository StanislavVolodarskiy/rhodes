package com.rhomobile.rhodes;

import java.io.IOException;
import java.net.HttpCookie;
import java.net.HttpURLConnection;
import java.net.ProtocolException;
import java.net.URL;
import java.util.List;

public class NetConnection
{
    private static final String TAG = NetConnection.class.getSimpleName();
    private static void ERROR(String    msg) { Logger.E(TAG, msg); }
    private static void INFO (String    msg) { Logger.I(TAG, msg); }
    private static void ERROR(Throwable ex ) { Logger.E(TAG, ex ); }

    private final HttpURLConnection connection;

    public NetConnection(String url) throws IOException
    {
        connection = (HttpURLConnection) new URL(url).openConnection();
    }

    public void setRequestMethod(String method) throws ProtocolException
    {
        connection.setRequestMethod(method);
    }

    public void setRequestProperty(String key, String value)
    {
        connection.setRequestProperty(key, value);
    }

    public void addRequestProperty(String key, String value)
    {
        connection.addRequestProperty(key, value);
    }

    public NetConnectionWriter getWriter() throws IOException
    {
        return new NetConnectionWriter(connection);
    }

    public NetConnectionReader getReader() throws IOException
    {
        return new NetConnectionReader(connection);
    }

    public int getResponseCode()
    {
        int response_code = -1;
        if (connection != null) {
            try {
                response_code = connection.getResponseCode();
            } catch (IOException ee) {
                INFO("response code exception is [" + ee.getMessage() + "]");
            }
        }
        return response_code;
    }

    public String getCookies()
    {
        StringBuilder sb = new StringBuilder();
        List<String> headers = connection.getHeaderFields().get("Set-Cookie");
        if (headers != null) {
            for (String header : headers) {
                for (HttpCookie cookie : HttpCookie.parse(header)) {
                    sb.append(cookie.getName()).append('=').append(cookie.getValue()).append(';');
                }
            }
        }
        return sb.toString();
    }

    public void disconnect()
    {
        connection.disconnect();
    }
}
