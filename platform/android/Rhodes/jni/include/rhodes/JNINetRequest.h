#ifndef JNI_NET_REQUEST
#define JNI_NET_REQUEST

#include "net/INetRequest.h"
#include "logging/RhoLog.h"
#include "common/RhoFile.h"

namespace rho
{
namespace net
{

class JNINetRequest : public INetRequestImpl
{
    DEFINE_LOGCLASS;
    
public:
    JNINetRequest();
    
    virtual ~JNINetRequest();

    virtual INetResponse* doRequest(
        const char* method,
        const String& strUrl,
        const String& strBody,
        IRhoSession* oSession,
        Hashtable<String,String>* pHeaders
    );
    virtual INetResponse* pullFile(
        const String& strUrl,
        common::CRhoFile& oFile,
        IRhoSession* oSession,
        Hashtable<String, String>* pHeaders
    );
    virtual INetResponse* pushMultipartData(
        const String& strUrl,
        VectorPtr<CMultipartItem*>& arItems,
        IRhoSession* oSession,
        Hashtable<String, String>* pHeaders
    );

    virtual void cancel();

    virtual boolean getSslVerifyPeer();
    virtual void setSslVerifyPeer(boolean mode);

    virtual INetResponse* createEmptyNetResponse();
    
    virtual void setCallback(INetRequestCallback*);

private:
    // disable copy constructor and assignment
    JNINetRequest& operator=(const JNINetRequest&);
    JNINetRequest(const JNINetRequest&);

    class Impl;

    Impl *impl;
};

} // namespace net
} // namespace rho

#endif // JNI_NET_REQUEST
