package com.rhomobile.rhodes;

public interface INetConnection
{
    int readResponseBody(byte[] data);
    int getResponseCode();
}
