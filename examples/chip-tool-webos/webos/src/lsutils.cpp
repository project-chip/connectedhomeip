#include "lsutils.h"
#include "logging.h"
#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

void LSUtils::postToClient(LS::Message & message, pbnjson::JValue & object)
{
    std::string payload;
    LSUtils::generatePayload(object, payload);

    try
    {
        message.respond(payload.c_str());
    } catch (LS::Error & error)
    {
        printf("Failed to submit response: %s\n", error.what());
    }
}
