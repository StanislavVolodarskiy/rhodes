package com.rhomobile.rhodes;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.util.Scanner;

public class NetConnectionReader
{
    private final InputStream is;

    public NetConnectionReader(HttpURLConnection connection) throws IOException
    {
        is = new BufferedInputStream(connection.getInputStream());
    }

    public String readAll()
    {
        Scanner s = new Scanner(is).useDelimiter("\\A");
        return s.hasNext() ? s.next() : "";
    }

    public void close() throws IOException
    {
        is.close();
    }
}
