#include <lib/core/CHIPError.h>
#include <tracing/backend.h>

#include <memory>
namespace chip {
namespace Tracing {
namespace Insights {

// Provide the user the ability to add/remove trace filters.
namespace ESP32Filter {

CHIP_ERROR AddHashToPermitlist(const char * alabel);
void RemoveHashFromPermitlist(const char * alabel);
} // namespace ESP32Filter

/// A Backend that outputs data to chip logging.
///
/// Structured data is formatted as json strings.
class ESP32Backend : public ::chip::Tracing::Backend
{
public:
    ESP32Backend() = default;

    void TraceBegin(const char * label, const char * group) override;

    void TraceEnd(const char * label, const char * group) override;

    /// Trace a zero-sized event
    void TraceInstant(const char * label, const char * group) override;

    void LogMessageSend(MessageSendInfo &) override;
    void LogMessageReceived(MessageReceivedInfo &) override;

    void LogNodeLookup(NodeLookupInfo &) override;
    void LogNodeDiscovered(NodeDiscoveredInfo &) override;
    void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) override;
};

} // namespace Insights
} // namespace Tracing
} // namespace chip
