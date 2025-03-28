// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Messages (cluster code: 151/0x97)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace Messages {

inline constexpr ClusterId kClusterId = 0x00000097;

namespace Attributes {
namespace Messages {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Messages
namespace ActiveMessageIDs {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ActiveMessageIDs

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace PresentMessagesRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace PresentMessagesRequest
namespace CancelMessagesRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace CancelMessagesRequest
} // namespace Commands

namespace Events {
namespace MessageQueued {
inline constexpr EventId Id = 0x00000000;
} // namespace MessageQueued
namespace MessagePresented {
inline constexpr EventId Id = 0x00000001;
} // namespace MessagePresented
namespace MessageComplete {
inline constexpr EventId Id = 0x00000002;
} // namespace MessageComplete
} // namespace Events

} // namespace Messages
} // namespace clusters
} // namespace app
} // namespace chip
