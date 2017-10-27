package com.rhomobile.rhodes;

import java.io.IOException;
import java.net.ProtocolException;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutionException;

import org.apache.http.entity.mime.MultipartEntityBuilder;
import android.os.AsyncTask;

public class NetRequest
{
    private static final String TAG = NetRequest.class.getSimpleName();
    private static void ERROR(String    msg) { Logger.E(TAG, msg); }
    private static void INFO (String    msg) { Logger.I(TAG, msg); }
    private static void ERROR(Throwable ex ) { Logger.E(TAG, ex ); }

    private static final INetConnection failedNetConnection = new INetConnection()
    {
        @Override public String readResponseBody(int n) { return null; }
        @Override public String readAllResponseBody() { return null; }
        @Override public int getResponseCode() { return -1; }
        @Override public String getCookies() { return null; }
        @Override public void disconnect() { }
    };

    private static class Request
    {
        public final String method;
        public final String url;
        public final String body;
        public final String session;
        public final Map<String, String> headers;

        public Request(String method, String url, String body, String session, Map<String, String> headers)
        {
            this.method = method;
            this.url = url;
            this.body = body;
            this.session = session;
            this.headers = headers;
        }
    }

    private static class MultipartRequest
    {
        public final String url;
        public final List<MultipartItem> items;
        public final String session;
        public final Map<String, String> headers;

        public MultipartRequest(
            String url,
            List<MultipartItem> items,
            String session,
            Map<String, String> headers
        )
        {
            this.url = url;
            this.items = items;
            this.session = session;
            this.headers = headers;
        }
    }

    private class RequestTask extends AsyncTask<Request, Void, INetConnection>
    {
        protected INetConnection doInBackground(Request... requests) {
            assert requests.length == 1;
            Request request = requests[0];
            return doRequest_(request.method, request.url, request.body, request.session, request.headers);
        }
    }

    private class MultipartRequestTask extends AsyncTask<MultipartRequest, Void, NetResponse>
    {
        protected NetResponse doInBackground(MultipartRequest... requests) {
            assert requests.length == 1;
            MultipartRequest request = requests[0];
            return pushMultipartData_(request.url, request.items, request.session, request.headers);
        }
    }

    public INetConnection doRequest(
        String method,
        String url,
        String body,
        String session,
        Map<String, String> headers
    )
    {
        AsyncTask<Request, Void, INetConnection> task = new RequestTask().execute(
            new Request(method, url, body, session, headers)
        );
        try {
            return task.get();
        } catch (InterruptedException e) {
            return failedNetConnection;
        } catch (ExecutionException e) {
            return failedNetConnection;
        }
    }

    public NetResponse pushMultipartData(
        String url,
        List<MultipartItem> items,
        String session,
        Map<String, String> headers
    )
    {
        AsyncTask<MultipartRequest, Void, NetResponse> task = new MultipartRequestTask().execute(
            new MultipartRequest(url, items, session, headers)
        );
        try {
            return task.get();
        } catch (InterruptedException e) {
            return new NetResponse(-1, null, null);
        } catch (ExecutionException e) {
            return new NetResponse(-1, null, null);
        }
    }

    private INetConnection doRequest_(
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

    private NetResponse pushMultipartData_(
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
                return new NetResponse(
                    connection.getResponseCode(),
                    connection.readAllResponseBody(),
                    connection.getCookies()
                );
            } finally {
                INFO("MARK 5");
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
