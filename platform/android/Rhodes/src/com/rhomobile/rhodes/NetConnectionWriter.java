package com.rhomobile.rhodes;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;

public class NetConnectionWriter
{
    private final OutputStream os;
    private final BufferedWriter writer;

    public NetConnectionWriter(HttpURLConnection connection) throws IOException
    {
        os = connection.getOutputStream();
        writer = new BufferedWriter(new OutputStreamWriter(os, "UTF-8"));
    }

    public void write(String s) throws IOException
    {
        writer.write(s);
    }

    public void close() throws IOException
    {
        writer.flush();
        writer.close();
        os.close();
    }
}
