package com.rhomobile.rhodes;

import java.io.IOException;
import java.net.ProtocolException;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;

import org.apache.http.entity.mime.MultipartEntityBuilder;

import com.rhomobile.rhodes.util.Utils;

public class NetRequest
{
    private static final String TAG = NetRequest.class.getSimpleName();
    private static void ERROR(String    msg) { Logger.E(TAG, msg); }
    private static void INFO (String    msg) { Logger.I(TAG, msg); }
    private static void ERROR(Throwable ex ) { Logger.E(TAG, ex ); }

    private static final INetConnection failedNetConnection = new INetConnection()
    {
        @Override public byte[] readResponseBody(int n) { return null; }
        @Override public byte[] readAllResponseBody() { return null; }
        @Override public int getResponseCode() { return -1; }
        @Override public String getCookies() { return null; }
        @Override public void disconnect() { }
    };

    private static final NetResponse failedNetResponse = new NetResponse(-1, null, null);

    public INetConnection doRequest(
        final String method,
        final String url,
        final String body,
        final String session,
        final Map<String, String> headers
    )
    {
        return Utils.computeAsync(new Callable<INetConnection>() {
            public INetConnection call() {
                return doRequest_(method, url, body, session, headers);
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
        }, failedNetResponse);
    }

    private static INetConnection doRequest_(
        String method,
        String url,
        String body,
        String session,
        Map<String, String> headers
    )
    {
        INFO("doRequest_");
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
            setupConnection(connection, method, headers, session);

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

    public static NetResponse pushMultipartData_(
        String url,
        List<MultipartItem> items,
        String session,
        Map<String, String> headers
    )
    {
        INFO("pushMultipartData_");
        INFO("url is [" + url + "]");
        INFO("session is [" + session + "]");
        INFO("headers are " + ((headers == null) ? "null" : "not null"));

        NetConnection connection = null;
        try {
            INFO("MARK 1");
            connection = new NetConnection(url);

            INFO("MARK 2");
            try {
                setupConnection(connection, "POST", headers, session);

                INFO("MARK 3");
                connection.setMultipartData(items);

                INFO("MARK 4");
                byte[] body = connection.readAllResponseBody();

                INFO("MARK 5");
                return new NetResponse(
                    connection.getResponseCode(),
                    (body == null) ? null : new String(body),
                    connection.getCookies()
                );
            } finally {
                INFO("MARK 6");
                connection.disconnect();
            }
        } catch (IOException e) {
            INFO("exception is [" + e.getMessage() + "]");
            return new NetResponse(getResponseCode(connection), null, null);
        }
    }

    private static void setupConnection(
        NetConnection connection,
        String method,
        Map<String, String> headers,
        String session
    ) throws ProtocolException
    {
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
    }

    private static int getResponseCode(INetConnection connection)
    {
        int response_code = (connection == null) ? -1 : connection.getResponseCode();
        INFO("response code is " + response_code);
        return response_code;
    }
}
