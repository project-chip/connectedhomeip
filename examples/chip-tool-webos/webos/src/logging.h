#ifndef LOGGING_H
#define LOGGING_H

#include <PmLogLib.h>

extern PmLogContext logContext;

#define MATTER_CRITICAL(msgid, kvcount, ...) \
	PmLogCritical(logContext, msgid, kvcount, ##__VA_ARGS__)

#define MATTER_ERROR(msgid, kvcount, ...) \
	PmLogError(logContext, msgid, kvcount,##__VA_ARGS__)

#define MATTER_WARNING(msgid, kvcount, ...) \
	PmLogWarning(logContext, msgid, kvcount, ##__VA_ARGS__)

#define MATTER_INFO(msgid, kvcount, ...) \
	PmLogInfo(logContext, msgid, kvcount, ##__VA_ARGS__)

#define MATTER_DEBUG(fmt, ...) \
	PmLogDebug(logContext, "%s:%s() " fmt, __FILE__, __FUNCTION__, ##__VA_ARGS__)

#define MSGID_LS2_FAILED_TO_SEND                    "LS2_FAILED_TO_SEND"
#define MSGID_SIL_WRONG_API                         "SIL_WRONG_API"
#define MSGID_SIL_DOESNT_EXIST                      "SIL_DOESNT_EXIST"
#define MSGID_ENABLED_PROFILE_NOT_SUPPORTED_BY_SIL  "ENABLED_PROFILE_NOT_SUPP_BY_SIL"
#define MSGID_NO_PAIRING_SUBSCRIBER                 "NO_PAIRING_SUBSCRIBER"
#define MSGID_LS2_FAIL_REGISTER_CANCEL_NOTI         "LS2_FAIL_REGISTER_CANCEL_NOTI"
#define MSGID_INVALID_PAIRING_CAPABILITY            "INVALID_PAIRING_CAPABILITY"
#define MSGID_SUBSCRIPTION_CLIENT_DROPPED           "SUBSCRIPTION_CLIENT_DROPPED"
#define MSGID_INCOMING_PAIR_REQ_FAIL                "INCOMING_PAIR_REQ_FAIL"
#define MSGID_UNPAIR_FROM_ANCS_FAILED               "OUTGOING_UNPAIR_FROM_ANCS_FAIL"

#endif // LOGGING_H