package com.rhomobile.rhodes;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpCookie;
import java.net.HttpURLConnection;
import java.net.ProtocolException;
import java.net.URL;
import java.util.List;
import java.util.concurrent.Callable;

import android.os.Looper;

import org.apache.http.entity.mime.MultipartEntityBuilder;
import org.apache.http.HttpEntity;

import com.rhomobile.rhodes.util.Utils;

public class NetConnection implements INetConnection
{
    private static final String TAG = NetConnection.class.getSimpleName();
    private static void ERROR(String    msg) { Logger.E(TAG, msg); }
    private static void INFO (String    msg) { Logger.I(TAG, msg); }
    private static void ERROR(Throwable ex ) { Logger.E(TAG, ex ); }

    private static class Reader
    {
        private final InputStream is;
        private boolean closed;

        public Reader(InputStream is)
        {
            this.is = is;
            closed = false;
        }

        public byte[] read(int n) throws IOException
        {
            if (closed) {
                return new byte[0];
            }

            byte[] data = new byte[n];
            int m = is.read(data, 0, n);
            if (m == -1) {
                close();
                return new byte[0];
            }
            if (m < n) {
                // shrink data array
                byte[] temp = new byte[m];
                System.arraycopy(data, 0, temp, 0, m);
                data = temp;
            }
            return data;
        }

        public byte[] readAll() throws IOException
        {
            ByteArrayOutputStream os = new ByteArrayOutputStream();
            if (!closed) {
                byte[] data = new byte[16384];
                while (true) {
                    int n = is.read(data, 0, data.length);
                    if (n == -1) {
                        break;
                    }
                    os.write(data, 0, n);
                }
                os.flush();
                close();
            }
            return os.toByteArray();
        }

        private void close() throws IOException
        {
            is.close();
            closed = true;
        }
    }

    private final HttpURLConnection connection;
    private Reader reader;

    public NetConnection(String url) throws IOException
    {
        assert Looper.getMainLooper().getThread() != Thread.currentThread();

        connection = (HttpURLConnection) new URL(url).openConnection();
        reader = null;
    }

    public void setRequestMethod(String method) throws ProtocolException
    {
        connection.setRequestMethod(method);
    }

    public void setRequestProperty(String key, String value)
    {
        connection.setRequestProperty(key, value);
    }

    public void writeRequestBody(String body) throws IOException
    {
        assert Looper.getMainLooper().getThread() != Thread.currentThread();

        OutputStream os = connection.getOutputStream();
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(os, "UTF-8"));
        writer.write(body);
        writer.flush();
        writer.close();
        os.close();
    }

    @Override
    public byte[] readResponseBody(final int n)
    {
        return Utils.computeAsync(new Callable<byte[]>() {
            public byte[] call() {
                return readResponseBodySync(n);
            }
        }, null);
    }

    @Override
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

    private byte[] readResponseBodySync(int n)
    {
        byte[] response_body = null;
        try {
            response_body = getReader().read(n);
            INFO("response body read size is " + response_body.length);
        } catch (IOException e) {
            INFO("response body exception is [" + e.getMessage() + "]");
        }
        return response_body;
    }

    private Reader getReader() throws IOException
    {
        if (reader == null) {
            assert Looper.getMainLooper().getThread() != Thread.currentThread();
            reader = new Reader(connection.getInputStream());
        }
        return reader;
    }
}
