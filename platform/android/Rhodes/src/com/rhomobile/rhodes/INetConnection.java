package com.rhomobile.rhodes;

public interface INetConnection
{
    String readResponseBody(int n);
    String readAllResponseBody();
    int getResponseCode();
    String getCookies();
    void disconnect();
}
