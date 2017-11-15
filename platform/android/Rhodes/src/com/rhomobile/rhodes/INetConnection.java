package com.rhomobile.rhodes;

public interface INetConnection
{
    byte[] readResponseBody(int n);
    int getResponseCode();
}
