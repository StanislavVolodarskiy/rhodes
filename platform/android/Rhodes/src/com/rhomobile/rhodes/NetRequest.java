package com.rhomobile.rhodes;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.ExecutionException;

import android.os.AsyncTask;

import com.rhomobile.rhodes.Logger;

public class NetRequest
{
    private static final String TAG = NetRequest.class.getSimpleName();
    private static void ERROR(String    msg) { Logger.E(TAG, msg); }
    private static void INFO (String    msg) { Logger.I(TAG, msg); }
    private static void ERROR(Throwable ex ) { Logger.E(TAG, ex ); }

    private static class Request
    {
        public final String method;
        public final String url;
        public final String body;
        public final Map<String, String> headers;

        public Request(String method, String url, String body, Map<String, String> headers)
        {
            this.method = method;
            this.url = url;
            this.body = body;
            this.headers = headers;
        }
    }

    private class RequestTask extends AsyncTask<Request, Void, NetResponse>
    {
        protected NetResponse doInBackground(Request... requests) {
            assert requests.length == 1;
            Request request = requests[0];
            return doRequest_(request.method, request.url, request.body, request.headers);
        }
    }

    public NetResponse doRequest(String method, String url, String body, Map<String, String> headers)
    {
        AsyncTask<Request, Void, NetResponse> task = new RequestTask().execute(
            new Request(method, url, body, headers)
        );
        try {
            return task.get();
        } catch (InterruptedException e) {
            return new NetResponse(-1, null);
        } catch (ExecutionException e) {
            return new NetResponse(-1, null);
        }
    }

    private NetResponse doRequest_(String method, String url_, String body, Map<String, String> headers)
    {
        INFO("method is [" + method + "]");
        INFO("url is [" + url_ + "]");
        INFO("body is [" + body + "]");
        INFO("headers is " + ((headers == null) ? "null" : "not null"));

        HttpURLConnection connection = null;
        try {
            INFO("MARK 1");
            URL url = new URL(url_);

            INFO("MARK 2");
            connection = (HttpURLConnection) url.openConnection();
            INFO("MARK 3");
            try {
                connection.setRequestMethod(method);

                INFO("MARK 4");
                if ("POST".equals(method) && body != null) {
                    INFO("MARK 4.1");
                    OutputStream os = connection.getOutputStream();
                    INFO("MARK 4.2");
                    BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(os, "UTF-8"));
                    INFO("MARK 4.3");
                    writer.write(body);
                    INFO("MARK 4.4");
                    writer.flush();
                    INFO("MARK 4.5");
                    writer.close();
                    INFO("MARK 4.6");
                    os.close();
                    INFO("MARK 4.7");
                }

                INFO("MARK 5");
                if (headers != null) {
                    INFO("headers size is " + headers.size());
                    for (Map.Entry<String, String> e : headers.entrySet()) {
                        INFO("header '" + e.getKey() + "': '" + e.getValue() + "'");
                        connection.setRequestProperty(e.getKey(), e.getValue());
                    }
                }

                INFO("MARK 6");
                InputStream in = new BufferedInputStream(connection.getInputStream());

                return new NetResponse(
                    connection.getResponseCode(),
                    convertStreamToString(in)
                );
            } finally {
                INFO("MARK 7");
                connection.disconnect();
            }
        } catch (IOException e) {
            INFO("exception is [" + e.getMessage() + "]");
            int response_code = -1;
            if (connection != null) {
                try {
                    response_code = connection.getResponseCode();
                } catch (IOException ee) {
                    INFO("response code exception is [" + ee.getMessage() + "]");
                }
            }
            INFO("response code is " + response_code);
            return new NetResponse(response_code, null);
        }
    }

    private static String convertStreamToString(InputStream is) {
        Scanner s = new Scanner(is).useDelimiter("\\A");
        return s.hasNext() ? s.next() : "";
    }
}
