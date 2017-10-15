package com.rhomobile.rhodes;

public class MultipartItem
{
    public final String filePath   ;
    public final String body       ;
    public final String name       ;
    public final String fileName   ;
    public final String contentType;
    public final String dataPrefix ;

    public MultipartItem(
        String filePath   ,
        String body       ,
        String name       ,
        String fileName   ,
        String contentType,
        String dataPrefix
    )
    {
        this.filePath    = filePath   ;
        this.body        = body       ;
        this.name        = name       ;
        this.fileName    = fileName   ;
        this.contentType = contentType;
        this.dataPrefix  = dataPrefix ;
    }
}
