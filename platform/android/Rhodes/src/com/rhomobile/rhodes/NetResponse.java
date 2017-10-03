package com.rhomobile.rhodes;

public class NetResponse
{
    private final int responseCode;
    private final String body;
    private final String cookies;

    public NetResponse(int responseCode, String body, String cookies)
    {
        this.responseCode = responseCode;
        this.body = body;
        this.cookies = cookies;
    }

    public int responseCode() {
        return responseCode;
    }

    public String body() {
        return body;
    }

    public String cookies() {
        return cookies;
    }
}
