package com.rhomobile.rhodes;

public class NetResponse
{
    private final int responseCode;
    private final String body;

    public NetResponse(int responseCode, String body)
    {
        this.responseCode = responseCode;
        this.body = body;
    }

    public int responseCode() {
        return responseCode;
    }

    public String body() {
        return body;
    }
}
