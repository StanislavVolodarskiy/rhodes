#include "rhodes/JNIRhodes.h"
#include "rhodes/JNINetRequest.h"
#include "net/CURLNetRequest.h"
#include "logging/RhoLog.h"
#include "common/RhoStd.h"
#include "common/RhoFile.h"
#include "common/RhodesApp.h"
#include "common/RhoConf.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "Net"

static jobject call_net_request_do_request(
    const char* method,
    const rho::String& url,
    const rho::String& body,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
);
static int call_net_response_response_code(jobject response);
static rho::String call_net_response_body(jobject response);
static rho::String call_net_response_cookies(jobject response);
static jobject new_hashmap(const rho::Hashtable<rho::String, rho::String>& headers);


namespace rho {
namespace net {

class JNINetResponse : public INetResponse
{
public:
    JNINetResponse(
        const String& data_,
              int     resp_code_,
        const String& cookies_,
        const String& error_message_
    )
    : data(data_), resp_code(resp_code_), cookies(cookies_), error_message(error_message_)
    {}

    virtual             ~JNINetResponse ()                   {}
    virtual const char*  getCharData    ()                   { return data.c_str(); }
    virtual unsigned int getDataSize    ()                   { return data.size(); }
    virtual int          getRespCode    ()                   { return resp_code; }
    virtual String       getCookies     ()                   { return cookies; }
    virtual String       getErrorMessage()                   { return error_message; }
    virtual void         setCharData    (const char* szData) { data = szData; }

private:
	String data;
    int    resp_code;
    String cookies;
    String error_message;
};


class JNINetRequest::Impl : public CURLNetRequest
{
};

JNINetRequest::JNINetRequest()
: impl(new Impl)
{
    RAWLOG_INFO("UGU constructor");
}

JNINetRequest::~JNINetRequest()
{
    RAWLOG_INFO("UGU destructor");
    delete impl;
}

INetResponse* JNINetRequest::doRequest(
    const char* method,
    const String& strUrl,
    const String& strBody,
    IRhoSession* oSession, // TODO: support session
    Hashtable<String, String>* pHeaders
)
{
    RAWLOG_INFO("UGU doRequest");

    jobject response = call_net_request_do_request(method, strUrl, strBody, pHeaders);
    int response_code = call_net_response_response_code(response);
    String body = call_net_response_body(response);
    String cookies = call_net_response_cookies(response);

    // error message is not supported
    return new JNINetResponse(body, response_code, cookies, "");
    // return impl->doRequest(method, strUrl, strBody, oSession, pHeaders);
}

INetResponse* JNINetRequest::pullFile(
    const String& strUrl,
    common::CRhoFile& oFile,
    IRhoSession* oSession,
    Hashtable<String, String>* pHeaders
)
{
    RAWLOG_INFO("UGU pullFile");
    return impl->pullFile(strUrl, oFile, oSession, pHeaders);
}

INetResponse* JNINetRequest::pushMultipartData(
    const String& strUrl,
    VectorPtr<CMultipartItem*>& arItems,
    IRhoSession* oSession,
    Hashtable<String, String>* pHeaders
)
{
    RAWLOG_INFO("UGU pushMultipartData");
    return impl->pushMultipartData(strUrl, arItems, oSession, pHeaders);
}

void JNINetRequest::cancel()
{
    RAWLOG_INFO("UGU cancel");
    impl->cancel();
}

boolean JNINetRequest::getSslVerifyPeer()
{
    RAWLOG_INFO("UGU getSslVerifyPeer");
    return impl->getSslVerifyPeer();
}

void JNINetRequest::setSslVerifyPeer(boolean mode)
{
    RAWLOG_INFO("UGU setSslVerifyPeer");
    impl->setSslVerifyPeer(mode);
}

INetResponse* JNINetRequest::createEmptyNetResponse()
{
    RAWLOG_INFO("UGU createEmptyNetResponse");
    return impl->createEmptyNetResponse();
}

void JNINetRequest::setCallback(INetRequestCallback* cb)
{
    RAWLOG_INFO("UGU setCallback");
    impl->setCallback(cb);
}

} // namespace net
} // namespace rho


jobject call_net_request_do_request(
    const char*        method,
    const rho::String& url,
    const rho::String& body,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    JNIEnv *env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETREQUEST);
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    jmethodID do_request = getJNIClassMethod(
        env,
        class_,
        "doRequest",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)Lcom/rhomobile/rhodes/NetResponse;"
    );


    jobject net_request = env->NewObject(class_, constructor);

    jhstring method_j = rho_cast<jstring>(env, method);
    jhstring url_j = rho_cast<jstring>(env, url);
    jhstring body_j = rho_cast<jstring>(env, body);
    jobject headers = (pHeaders == NULL) ? NULL : new_hashmap(*pHeaders);

    return env->CallObjectMethod(
        net_request,
        do_request,
        method_j.get(),
        url_j.get(),
        body_j.get(),
        headers
    );
}

jobject new_hashmap(const rho::Hashtable<rho::String, rho::String>& headers)
{
    JNIEnv *env = jnienv();
    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_HASHMAP);
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    jmethodID put = getJNIClassMethod(env, class_, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jobject hashmap = env->NewObject(class_, constructor);

    for (
        rho::Hashtable<rho::String, rho::String>::const_iterator it = headers.begin();
        it != headers.end();
        ++it
    ) {
        jhstring key = rho_cast<jstring>(it->first);
        jhstring value = rho_cast<jstring>(it->second);
        env->CallObjectMethod(hashmap, put, key.get(), value.get());
    }
    return hashmap;
}

int call_net_response_response_code(jobject response)
{
    JNIEnv *env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETRESPONSE);
    jmethodID response_code = getJNIClassMethod(env, class_, "responseCode", "()I");

    return env->CallIntMethod(response, response_code);
}

rho::String call_net_response_body(jobject response)
{
    JNIEnv *env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETRESPONSE);
    jmethodID method = getJNIClassMethod(env, class_, "body", "()Ljava/lang/String;");
    jhstring body = static_cast<jstring>(env->CallObjectMethod(response, method));
    return rho_cast<rho::String>(env, body);
}

rho::String call_net_response_cookies(jobject response)
{
    JNIEnv *env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETRESPONSE);
    jmethodID method = getJNIClassMethod(env, class_, "cookies", "()Ljava/lang/String;");
    jhstring cookies = static_cast<jstring>(env->CallObjectMethod(response, method));
    return rho_cast<rho::String>(env, cookies);
}
