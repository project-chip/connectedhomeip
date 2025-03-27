#pragma once

#include <transport.h>

// Connection object to store transport and stream IDs
struct Connection
{
    Transport * transport;
    uint16_t videoStreamID;
    uint16_t audioStreamID;
};

// Media Controller
class MediaController {
public:
    void RegisterTransport(Transport* transport, uint16_t videoStreamID, uint16_t audioStreamID)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.push_back({transport, videoStreamID, audioStreamID});
    }

    void UnregisterTransport(Transport* transport)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.erase(std::remove_if(connections.begin(), connections.end(),
                                         [transport](const Connection& c) { return c.transport == transport; }),
                          connections.end());
    }

    void DistributeVideo(const char* data, size_t size, uint16_t videoStreamID)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        for (const Connection& connection : connections)
        {
            if (connection.videoStreamID == videoStreamID)
            {
                connection.transport->SendVideo(data, size, videoStreamID);
            }
        }
    }

    void DistributeAudio(const char* data, size_t size, uint16_t audioStreamID)
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        for (const Connection& connection : connections)
        {
            if (connection.audioStreamID == audioStreamID)
            {
                connection.transport->SendAudio(data, size, audioStreamID);
            }
        }
    }

private:
    std::vector<Connection> connections;
    std::mutex connectionsMutex;
};