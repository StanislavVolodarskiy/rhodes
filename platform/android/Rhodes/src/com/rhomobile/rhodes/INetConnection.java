package com.rhomobile.rhodes;

public interface INetConnection
{
    byte[] readResponseBody(int n);
    byte[] readAllResponseBody();
    int getResponseCode();
    String getCookies();
    void disconnect();
}
