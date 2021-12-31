#ifndef CLIENTWATCH_H
#define CLIENTWATCH_H

#include <PmLogLib.h>
#include <luna-service2++/error.hpp>
#include <luna-service2/lunaservice.hpp>
#include <string>

namespace LSUtils {

typedef std::function<void(void)> ClientWatchStatusCallback;

class ClientWatch
{
public:
    ClientWatch(LSHandle * handle, LSMessage * message, ClientWatchStatusCallback callback);
    ClientWatch(const ClientWatch & other) = delete;
    ~ClientWatch();

    LSMessage * getMessage() const { return mMessage; }
    void setCallback(ClientWatchStatusCallback callback) { mCallback = callback; }

private:
    LSHandle * mHandle;
    LSMessage * mMessage;
    void * mCookie;
    ClientWatchStatusCallback mCallback;
    guint mNotificationTimeout;

    void startWatching();
    void cleanup();
    void triggerClientDroppedNotification();

    void notifyClientDisconnected();
    void notifyClientCanceled(const char * clientToken);

    static gboolean sendClientDroppedNotification(gpointer user_data);
    static bool serverStatusCallback(LSHandle *, const char *, bool connected, void * context);
    static bool clientCanceledCallback(LSHandle *, const char * uniqueToken, void * context);
};

} // namespace LSUtils

#endif // CLIENTWATCH_H
