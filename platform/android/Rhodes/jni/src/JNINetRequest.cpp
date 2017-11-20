#include <sstream>
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

extern "C" void rho_net_impl_network_indicator(int active);

static int pull_file(
    const rho::String& url,
    rho::common::CRhoFile& file,
    rho::net::IRhoSession* pSession,
    rho::Hashtable<rho::String, rho::String>* pHeaders
);
static jobject call_net_request_do_request(
    const char* method,
    const rho::String& url,
    const rho::String& body,
          rho::net::IRhoSession* pSession,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
);
static jobject call_net_request_do_request_2(
    const char* method,
    const rho::String& url,
    const rho::String& body,
          rho::net::IRhoSession* pSession,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
);
static jobject call_net_request_pull_file(
    const rho::String& url,
          long start_from,
          rho::net::IRhoSession* pSession,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
);
static jobject call_net_request_push_multipart_data(
    const rho::String& url,
    const rho::VectorPtr<rho::net::CMultipartItem*>& items,
          rho::net::IRhoSession* pSession,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
);
static int call_net_response_response_code(jobject response);
static rho::String call_net_response_body(jobject response);
static rho::String call_net_response_cookies(jobject response);
static int call_net_connection_response_code(jobject connection);
static rho::String get_session_string(rho::net::IRhoSession* pSession);
static jobject new_hashmap(const rho::Hashtable<rho::String, rho::String>& headers);
static rho::net::INetResponse* convert_net_response(jobject response);
static jobject new_multipart_item(const rho::net::CMultipartItem& item);
static jobject new_multipart_items(const rho::VectorPtr<rho::net::CMultipartItem*>& items);


namespace rho {
namespace net {

class NetworkIndicator
{
public:
    NetworkIndicator(bool enable_ = true) : enable(enable_) { set(1); }
    ~NetworkIndicator() { set(0); }
private:
    void set(int state)
    {
       if (enable) {
           rho_net_impl_network_indicator(state);
       }
    }
    const bool enable;
};

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


} // namespace net
} // namespace rho


class rho::net::JNINetRequest::Impl : public CURLNetRequest
{
};

class ConnectionReader
{
public:
    ConnectionReader();
    bool read(jobject connection, rho::common::CRhoFile& file);

private:
    bool read_part(jobject connection, rho::common::CRhoFile& file);
    JNIEnv *env;
    jmethodID method;
    jbyteArray data;
};

rho::net::JNINetRequest::JNINetRequest()
: impl(new Impl)
{
    RAWLOG_INFO("UGU constructor");
}

rho::net::JNINetRequest::~JNINetRequest()
{
    RAWLOG_INFO("UGU destructor");
    delete impl;
}

rho::net::INetResponse* rho::net::JNINetRequest::doRequest(
    const char* method,
    const rho::String& url,
    const rho::String& body,
    rho::net::IRhoSession* pSession,
    rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    RAWLOG_INFO("UGU doRequest");
    NetworkIndicator ni;
    return convert_net_response(call_net_request_do_request(method, url, body, pSession, pHeaders));
}

rho::net::INetResponse* rho::net::JNINetRequest::pullFile(
    const rho::String& url,
    rho::common::CRhoFile& file,
    rho::net::IRhoSession* pSession,
    rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    RAWLOG_INFO("UGU pullFile");
    NetworkIndicator ni = !RHODESAPP().isBaseUrl(url.c_str());
    return new rho::net::JNINetResponse("", pull_file(url, file, pSession, pHeaders), "", "");
}

rho::net::INetResponse* rho::net::JNINetRequest::pushMultipartData(
    const rho::String& url,
    VectorPtr<CMultipartItem*>& items,
    rho::net::IRhoSession* pSession,
    rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    RAWLOG_INFO("UGU pushMultipartData");
    NetworkIndicator ni;
    return convert_net_response(call_net_request_push_multipart_data(url, items, pSession, pHeaders));
}

void rho::net::JNINetRequest::cancel()
{
    RAWLOG_INFO("UGU cancel");
    impl->cancel();
}

rho::boolean rho::net::JNINetRequest::getSslVerifyPeer()
{
    RAWLOG_INFO("UGU getSslVerifyPeer");
    return impl->getSslVerifyPeer();
}

void rho::net::JNINetRequest::setSslVerifyPeer(boolean mode)
{
    RAWLOG_INFO("UGU setSslVerifyPeer");
    impl->setSslVerifyPeer(mode);
}

rho::net::INetResponse* rho::net::JNINetRequest::createEmptyNetResponse()
{
    RAWLOG_INFO("UGU createEmptyNetResponse");
    return impl->createEmptyNetResponse();
}

void rho::net::JNINetRequest::setCallback(INetRequestCallback* cb)
{
    RAWLOG_INFO("UGU setCallback");
    impl->setCallback(cb);
}


int pull_file(
    const rho::String& url,
    rho::common::CRhoFile& file,
    rho::net::IRhoSession* pSession,
    rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    int response_code = -1;
    ConnectionReader cr;
    for (int n = 0; n < 10; ++n) {
        bool have_read = true;
        do {
            RAWLOG_INFO("pull_file: 1");
            jobject connection = call_net_request_pull_file(url, file.size(), pSession, pHeaders);
            RAWLOG_INFO("pull_file: 2");
            have_read = cr.read(connection, file);
            RAWLOG_INFO("pull_file: 3");
            file.flush();
            RAWLOG_INFO("pull_file: 4");

            response_code = call_net_connection_response_code(connection);
            RAWLOG_INFO("pull_file: 5");
            switch (response_code) {
            case 416:
                // simulate successful completion
            case 206:
                RAWLOG_INFO("pull_file: 6");
                return 206;
            }
        } while (have_read);
    }
    RAWLOG_INFO("pull_file: 7");
    return response_code;
}

jobject call_net_request_do_request(
    const char*        method,
    const rho::String& url,
    const rho::String& body,
          rho::net::IRhoSession* pSession,
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
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)"
        "Lcom/rhomobile/rhodes/NetResponse;"
    );


    jobject net_request = env->NewObject(class_, constructor);

    jhstring method_j = rho_cast<jstring>(env, method);
    jhstring url_j = rho_cast<jstring>(env, url);
    jhstring body_j = rho_cast<jstring>(env, body);
    jhstring session_j = rho_cast<jstring>(env, get_session_string(pSession));
    jobject headers_j = (pHeaders == NULL) ? NULL : new_hashmap(*pHeaders);

    return env->CallObjectMethod(
        net_request,
        do_request,
        method_j.get(),
        url_j.get(),
        body_j.get(),
        session_j.get(),
        headers_j
    );
}

jobject call_net_request_do_request_2(
    const char*        method,
    const rho::String& url,
    const rho::String& body,
          rho::net::IRhoSession* pSession,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    RAWLOG_INFO("call_net_request_do_request_2: 1");
    JNIEnv *env = jnienv();

    RAWLOG_INFO("call_net_request_do_request_2: 2");
    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETREQUEST);
    RAWLOG_INFO("call_net_request_do_request_2: 3");
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    RAWLOG_INFO("call_net_request_do_request_2: 4");
    jmethodID do_request = getJNIClassMethod(
        env,
        class_,
        "doRequest2",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)"
        "Lcom/rhomobile/rhodes/INetConnection;"
    );

    RAWLOG_INFO("call_net_request_do_request_2: 5");

    jobject net_request = env->NewObject(class_, constructor);

    RAWLOG_INFO("call_net_request_do_request_2: 6");
    jhstring method_j = rho_cast<jstring>(env, method);
    RAWLOG_INFO("call_net_request_do_request_2: 7");
    jhstring url_j = rho_cast<jstring>(env, url);
    RAWLOG_INFO("call_net_request_do_request_2: 8");
    jhstring body_j = rho_cast<jstring>(env, body);
    RAWLOG_INFO("call_net_request_do_request_2: 9");
    jhstring session_j = rho_cast<jstring>(env, get_session_string(pSession));
    RAWLOG_INFO("call_net_request_do_request_2: 10");
    jobject headers_j = (pHeaders == NULL) ? NULL : new_hashmap(*pHeaders);
    RAWLOG_INFO("call_net_request_do_request_2: 11");

    jobject connection = env->CallObjectMethod(
        net_request,
        do_request,
        method_j.get(),
        url_j.get(),
        body_j.get(),
        session_j.get(),
        headers_j
    );
    RAWLOG_INFO("call_net_request_do_request_2: 12");
    return connection;
}

jobject call_net_request_pull_file(
    const rho::String& url,
          long start_from,
          rho::net::IRhoSession* pSession,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    RAWLOG_INFO("call_net_request_pull_file");

    rho::Hashtable<rho::String, rho::String> headers;
    if (pHeaders != NULL) {
        headers = *pHeaders;
    }
	std::ostringstream oss;
    oss << "bytes=" << start_from << "-";
    headers.put("Range", oss.str());

    return call_net_request_do_request_2("GET", url, "", pSession, &headers);
}

jobject call_net_request_push_multipart_data(
    const rho::String& url,
    const rho::VectorPtr<rho::net::CMultipartItem*>& items,
          rho::net::IRhoSession* pSession,
    const rho::Hashtable<rho::String, rho::String>* pHeaders
)
{
    JNIEnv *env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_NETREQUEST);
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    jmethodID do_push_multipart_data = getJNIClassMethod(
        env,
        class_,
        "pushMultipartData",
        "(Ljava/lang/String;Ljava/util/List;Ljava/lang/String;Ljava/util/Map;)"
        "Lcom/rhomobile/rhodes/NetResponse;"
    );

    jobject net_request = env->NewObject(class_, constructor);

    jhstring url_j = rho_cast<jstring>(env, url);
    jobject items_j = new_multipart_items(items);
    jhstring session_j = rho_cast<jstring>(env, get_session_string(pSession));
    jobject headers_j = (pHeaders == NULL) ? NULL : new_hashmap(*pHeaders);

    return env->CallObjectMethod(
        net_request,
        do_push_multipart_data,
        url_j.get(),
        items_j,
        session_j.get(),
        headers_j
    );
}

rho::String get_session_string(rho::net::IRhoSession* pSession)
{
    return (pSession == 0) ? "" : pSession->getSession();
}

jobject new_hashmap(const rho::Hashtable<rho::String, rho::String>& headers)
{
    RAWLOG_INFO("new_hashmap: 1");
    JNIEnv *env = jnienv();
    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_HASHMAP);
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    jmethodID put = getJNIClassMethod(
        env,
        class_,
        "put",
        "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"
    );
    jobject hashmap = env->NewObject(class_, constructor);
    RAWLOG_INFO("new_hashmap: 2");

    for (
        rho::Hashtable<rho::String, rho::String>::const_iterator it = headers.begin();
        it != headers.end();
        ++it
    ) {
        RAWLOG_INFO("new_hashmap: 3");
        jhstring key = rho_cast<jstring>(it->first);
        RAWLOG_INFO("new_hashmap: 4");
        jhstring value = rho_cast<jstring>(it->second);
        RAWLOG_INFO("new_hashmap: 5");
        env->CallObjectMethod(hashmap, put, key.get(), value.get());
        RAWLOG_INFO("new_hashmap: 6");
    }
    RAWLOG_INFO("new_hashmap: 7");
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

int call_net_connection_response_code(jobject connection)
{
    JNIEnv *env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_INETCONNECTION);
    jmethodID method = getJNIClassMethod(env, class_, "getResponseCode", "()I");
    return env->CallIntMethod(connection, method);
}

rho::net::INetResponse* convert_net_response(jobject response)
{
    int response_code = call_net_response_response_code(response);
    rho::String body = call_net_response_body(response);
    rho::String cookies = call_net_response_cookies(response);

    // error message is not supported
    return new rho::net::JNINetResponse(body, response_code, cookies, "");
}

jobject new_multipart_item(const rho::net::CMultipartItem& item)
{
    JNIEnv *env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_MULTIPARTITEM);
    jmethodID constructor = getJNIClassMethod(
        env,
        class_,
        "<init>",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
        "Ljava/lang/String;)V"
    );

    jhstring file_path_j    = rho_cast<jstring>(env, item.m_strFilePath   );
    jhstring body_j         = rho_cast<jstring>(env, item.m_strBody       );
    jhstring name_j         = rho_cast<jstring>(env, item.m_strName       );
    jhstring file_name_j    = rho_cast<jstring>(env, item.m_strFileName   );
    jhstring content_type_j = rho_cast<jstring>(env, item.m_strContentType);
    jhstring data_prefix_j  = rho_cast<jstring>(env, item.m_strDataPrefix );

    return env->NewObject(
        class_,
        constructor,
        file_path_j.get(),
        body_j.get(),
        name_j.get(),
        file_name_j.get(),
        content_type_j.get(),
        data_prefix_j.get()
    );
}

jobject new_multipart_items(const rho::VectorPtr<rho::net::CMultipartItem*>& items)
{
    JNIEnv *env = jnienv();
    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_ARRAYLIST);
    jmethodID constructor = getJNIClassMethod(env, class_, "<init>", "()V");
    jmethodID add = getJNIClassMethod(env, class_, "add", "(Ljava/lang/Object;)B");
    jobject arraylist = env->NewObject(class_, constructor);

    for (
        rho::VectorPtr<rho::net::CMultipartItem*>::const_iterator it = items.begin();
        it != items.end();
        ++it
    ) {
        if (*it != NULL) {
            env->CallObjectMethod(arraylist, add, new_multipart_item(**it));
        }
    }
    return arraylist;
}

ConnectionReader::ConnectionReader()
{
    env = jnienv();

    jclass class_ = getJNIClass(RHODES_JAVA_CLASS_INETCONNECTION);
    method = getJNIClassMethod(env, class_, "readResponseBody", "([B)I");

    data = env->NewByteArray(16384);
}

bool ConnectionReader::read(jobject connection, rho::common::CRhoFile& file)
{
    bool have_read = false;
    while (read_part(connection, file)) {
        have_read = true;
    }
    return have_read;
}

bool ConnectionReader::read_part(jobject connection, rho::common::CRhoFile& file)
{
    RAWLOG_INFO("ConnectionReader::read_part: 1");
    jint n = env->CallIntMethod(connection, method, data);
    RAWLOG_INFO("ConnectionReader::read_part: 2");
    if (n == 0) {
        RAWLOG_INFO("ConnectionReader::read_part: 3");
        return false;
    }
    RAWLOG_INFO("ConnectionReader::read_part: 4");
    jbyte *pData = env->GetByteArrayElements(data, NULL);
    RAWLOG_INFO("ConnectionReader::read_part: 5");
    file.write(pData, n);
    RAWLOG_INFO("ConnectionReader::read_part: 6");
    env->ReleaseByteArrayElements(data, pData, JNI_ABORT);
    RAWLOG_INFO("ConnectionReader::read_part: 7");
    return true;
}
