package com.rhomobile.rhodes;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpCookie;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.Callable;

import org.apache.http.entity.mime.MultipartEntityBuilder;
import org.apache.http.HttpEntity;

import com.rhomobile.rhodes.util.Utils;

public class NetRequest
{
    private static final String TAG = NetRequest.class.getSimpleName();
    private static void ERROR(String    msg) { Logger.E(TAG, msg); }
    private static void INFO (String    msg) { Logger.I(TAG, msg); }
    private static void ERROR(Throwable ex ) { Logger.E(TAG, ex ); }

    private static final INetConnection failedNetConnection = new INetConnection()
    {
        @Override public int readResponseBody(byte[] data) { return 0; }
        @Override public int getResponseCode() { return -1; }
    };

    private static final NetResponse failedResponse = new NetResponse(-1, null, null);

    public NetResponse doRequest(
        final String method,
        final String url,
        final String body,
        final String session,
        final Map<String, String> headers
    )
    {
        return Utils.computeAsync(new Callable<NetResponse>() {
            public NetResponse call() {
                return doRequest_(method, url, body, session, headers);
            }
        }, failedResponse);
    }

    public INetConnection doRequest2(
        final String method,
        final String url,
        final String body,
        final String session,
        final Map<String, String> headers
    )
    {
        return Utils.computeAsync(new Callable<INetConnection>() {
            public INetConnection call() {
                return doRequest2_(method, url, body, session, headers);
            }
        }, failedNetConnection);
    }

    public NetResponse pushMultipartData(
        final String url,
        final List<MultipartItem> items,
        final String session,
        final Map<String, String> headers
    )
    {
        return Utils.computeAsync(new Callable<NetResponse>() {
            public NetResponse call() {
                return pushMultipartData_(url, items, session, headers);
            }
        }, failedResponse);
    }

    private NetResponse doRequest_(
        String method,
        String url_,
        String body,
        String session,
        Map<String, String> headers
    )
    {
        INFO("doRequest_");
        INFO("method is [" + method + "]");
        INFO("url is [" + url_ + "]");
        INFO("body is [" + body + "]");
        INFO("session is [" + session + "]");
        INFO("headers are " + ((headers == null) ? "null" : "not null"));

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
                if (headers != null) {
                    INFO("headers size is " + headers.size());
                    for (Map.Entry<String, String> e : headers.entrySet()) {
                        INFO("header '" + e.getKey() + "': '" + e.getValue() + "'");
                        connection.setRequestProperty(e.getKey(), e.getValue());
                    }
                }

                INFO("MARK 5");
                if (session != null && !session.isEmpty()) {
                    connection.setRequestProperty("Cookie", session);
                }

                INFO("MARK 6");
                if ("POST".equals(method) && body != null) {
                    INFO("MARK 6.1");
                    OutputStream os = connection.getOutputStream();
                    INFO("MARK 6.2");
                    BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(os, "UTF-8"));
                    INFO("MARK 6.3");
                    writer.write(body);
                    INFO("MARK 6.4");
                    writer.flush();
                    INFO("MARK 6.5");
                    writer.close();
                    INFO("MARK 6.6");
                    os.close();
                    INFO("MARK 6.7");
                }

                INFO("MARK 7");
                String response_body = readResponseBody(connection);
                INFO("response body size is " + response_body.length());

                INFO("MARK 8");
                return new NetResponse(connection.getResponseCode(), response_body, readCookies(connection));
            } finally {
                INFO("MARK 9");
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
            return new NetResponse(response_code, null, null);
        }
    }

    private static INetConnection doRequest2_(
        String method,
        String url,
        String body,
        String session,
        Map<String, String> headers
    )
    {
        INFO("doRequest2_");
        INFO("method is [" + method + "]");
        INFO("url is [" + url + "]");
        INFO("body is [" + body + "]");
        INFO("session is [" + session + "]");
        INFO("headers are " + ((headers == null) ? "null" : "not null"));

        NetConnection connection = null;
        try {
            INFO("MARK 1");
            connection = new NetConnection(url);

            INFO("MARK 2");
            connection.setRequestMethod(method);

            if (headers != null) {
                INFO("headers size is " + headers.size());
                for (Map.Entry<String, String> e : headers.entrySet()) {
                    INFO("header '" + e.getKey() + "': '" + e.getValue() + "'");
                    connection.setRequestProperty(e.getKey(), e.getValue());
                }
            }

            if (session != null && !session.isEmpty()) {
                connection.setRequestProperty("Cookie", session);
            }

            INFO("MARK 3");
            if ("POST".equals(method) && body != null) {
                connection.writeRequestBody(body);
            }

            INFO("MARK 4");
            return connection;
        } catch (IOException e) {
            INFO("exception is [" + e.getMessage() + "]");
            return failedNetConnection;
        }
    }

    private NetResponse pushMultipartData_(
        String url_,
        List<MultipartItem> items,
        String session,
        Map<String, String> headers
    )
    {
        INFO("pushMultipartData_");
        INFO("url is [" + url_ + "]");
        INFO("session is [" + session + "]");
        INFO("headers are " + ((headers == null) ? "null" : "not null"));

        INFO("number of parts is " + items.size());
        MultipartEntityBuilder builder = MultipartEntityBuilder.create();
        for (MultipartItem item : items) {
            if (item.filePath == null || item.filePath.isEmpty()) {
                INFO("part is [" + item.name + "], [" + item.body + "]");
                builder.addTextBody(item.name, item.body);
            } else {
                // TODO: add file support
                assert false;
            }
        }
        HttpEntity entity = builder.build();

        HttpURLConnection connection = null;
        try {
            INFO("MARK 1");
            URL url = new URL(url_);

            INFO("MARK 2");
            connection = (HttpURLConnection) url.openConnection();
            INFO("MARK 3");
            try {
                connection.setRequestMethod("POST");

                INFO("MARK 4");
                if (headers != null) {
                    INFO("headers size is " + headers.size());
                    for (Map.Entry<String, String> e : headers.entrySet()) {
                        INFO("header '" + e.getKey() + "': '" + e.getValue() + "'");
                        connection.setRequestProperty(e.getKey(), e.getValue());
                    }
                }

                INFO("MARK 5");
                if (session != null && !session.isEmpty()) {
                    connection.setRequestProperty("Cookie", session);
                }

                INFO("MARK 6");
                connection.addRequestProperty("Content-length", "" + entity.getContentLength());
                connection.addRequestProperty(
                    entity.getContentType().getName(),
                    entity.getContentType().getValue()
                );

                INFO("MARK 7");
                OutputStream os = connection.getOutputStream();
                entity.writeTo(os);
                os.close();

                INFO("MARK 8");
                String response_body = readResponseBody(connection);
                INFO("response body size is " + response_body.length());

                INFO("MARK 9");

                return new NetResponse(connection.getResponseCode(), response_body, readCookies(connection));
            } finally {
                INFO("MARK 10");
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
            return new NetResponse(response_code, null, null);
        }
    }

    private static String convertStreamToString(InputStream is) {
        Scanner s = new Scanner(is).useDelimiter("\\A");
        return s.hasNext() ? s.next() : "";
    }

    private static String readResponseBody(HttpURLConnection connection) throws IOException {
        return convertStreamToString(new BufferedInputStream(connection.getInputStream()));
    }

    private static String readCookies(HttpURLConnection connection) {
        StringBuilder sb = new StringBuilder();
        List<String> headers = connection.getHeaderFields().get("Set-Cookie");
        if (headers != null) {
            for (String header : headers) {
                for (HttpCookie cookie : HttpCookie.parse(header)) {
                    sb.append(cookie.getName()).append('=').append(cookie.getValue()).append(';');
                }
            }
        }
        String s = sb.toString();
        INFO("cookies are [" + s + "]");
        return s;
    }
}
