#include "clientwatch.h"

namespace LSUtils
{

ClientWatch::ClientWatch(LSHandle *handle, LSMessage *message, ClientWatchStatusCallback callback) :
    mHandle(handle),
    mMessage(message),
    mCookie(0),
    mCallback(callback),
    mNotificationTimeout(0)
{
	if (!mMessage)
		return;

	LSMessageRef(mMessage);
	startWatching();
}

ClientWatch::~ClientWatch()
{
	// Don't send any pending notifications as that will fail
	if (mNotificationTimeout)
		g_source_remove(mNotificationTimeout);

	if (mCookie)
	{
		LS::Error error;

		/*if (!LSCancelServerStatus(mHandle, mCookie, error.get()))
			error.log(PmLogGetLibContext(), "LS_FAILED_TO_UNREG_SRV_STAT");*/
	}

	if (mMessage)
		LSMessageUnref(mMessage);

	LSCallCancelNotificationRemove(mHandle, &ClientWatch::clientCanceledCallback, this, NULL);
}

bool ClientWatch::serverStatusCallback(LSHandle *, const char *, bool connected, void *context)
{
	ClientWatch *watch = static_cast<ClientWatch*>(context);
	if (nullptr == watch)
		return false;

	if (connected)
		return true;

	watch->notifyClientDisconnected();

	return true;
}

bool ClientWatch::clientCanceledCallback(LSHandle *, const char *uniqueToken, void *context)
{
	ClientWatch *watch = static_cast<ClientWatch*>(context);
	if (nullptr == watch)
		return false;

	watch->notifyClientCanceled(uniqueToken);

	return true;
}

void ClientWatch::startWatching()
{
	if (!mMessage)
		return;

	const char *serviceName = LSMessageGetSender(mMessage);

	LS::Error error;
	if (!LSRegisterServerStatusEx(mHandle, serviceName, &ClientWatch::serverStatusCallback,
	                              this, &mCookie, error.get()))
		throw error;

	if (!LSCallCancelNotificationAdd(mHandle, &ClientWatch::clientCanceledCallback, this, error.get()))
		throw error;
}

gboolean ClientWatch::sendClientDroppedNotification(gpointer user_data)
{
	ClientWatch *watch = static_cast<ClientWatch*>(user_data);
	if (nullptr == watch)
		return FALSE;

	watch->mNotificationTimeout = 0;

	if (watch->mCallback)
		watch->mCallback();

	return FALSE;
}

void ClientWatch::triggerClientDroppedNotification()
{
	if (mNotificationTimeout)
		return;

	// We have to offload the actual callback here as otherwise we risk
	// a deadlock when someone tries to destroy us while stilling being
	// in the callback from ls2
	mNotificationTimeout = g_timeout_add(0, &ClientWatch::sendClientDroppedNotification, this);
}

void ClientWatch::notifyClientDisconnected()
{
	triggerClientDroppedNotification();
}

void ClientWatch::notifyClientCanceled(const char *clientToken)
{
	if (!mMessage)
		return;

	const char *messageToken = LSMessageGetUniqueToken(mMessage);

	if(!g_strcmp0(messageToken, clientToken))
		triggerClientDroppedNotification();
}

} // namespace LS
