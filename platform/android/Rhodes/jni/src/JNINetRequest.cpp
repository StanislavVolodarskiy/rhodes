#include "rhodes/JNIRhodes.h"
#include "rhodes/JNINetRequest.h"
#include "net/CURLNetRequest.h"
#include "logging/RhoLog.h"
#include "common/RhoFile.h"
#include "common/RhodesApp.h"
#include "common/RhoConf.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "Net"

namespace rho {
namespace net {

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
    IRhoSession* oSession,
    Hashtable<String,String>* pHeaders
)
{
    RAWLOG_INFO("UGU doRequest");

    JNIEnv *env = jnienv();
    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETREQUEST);
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    jmethodID do_request = getJNIClassMethod(env, class_, "doRequest", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)Z");

    jobject net_request = env->NewObject(class_, constructor);

    jhstring method_j = rho_cast<jstring>(env, method);
    jhstring url_j = rho_cast<jstring>(env, strUrl);
    jhstring body_j = rho_cast<jstring>(env, strBody);
        
    jboolean result = env->CallBooleanMethod(
        net_request,
        do_request,
        method_j.get(),
        url_j.get(),
        body_j.get(),
        null
    );

    RAWLOG_INFO((result) ? "UGU IS true" : "UGU IS false");
    RAWLOG_INFO("AGA doRequest");

    return impl->doRequest(method, strUrl, strBody, oSession, pHeaders);
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

void call_jni()
{
    JNIEnv *env = jnienv();
    if (env == NULL) {
        return;
    }

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETREQUEST);
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    jmethodID check = getJNIClassMethod(env, class_, "doRequest", "(Ljava/lang/String;)Z");
    if (check == NULL) {
        return;
    }

    RAWLOG_INFO("MARK 4");
    jobject net_request = env->NewObject(class_, constructor);
    if (net_request == NULL) {
        return;
    }

    RAWLOG_INFO("MARK 5");
    jhstring ugu = rho_cast<jstring>(env, "UGU");
    jboolean result = env->CallBooleanMethod(net_request, check, ugu.get());
    RAWLOG_INFO((result) ? "UGU IS true" : "UGU IS false");
    jhstring ugug = rho_cast<jstring>(env, "UGUG");
    result = env->CallBooleanMethod(net_request, check, ugug.get());
    RAWLOG_INFO((result) ? "UGUG IS true" : "UGUG IS false");

    RAWLOG_INFO("MARK 6");
    RAWLOG_INFO("MARK 7");
}
