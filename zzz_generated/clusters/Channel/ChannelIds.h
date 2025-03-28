// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Channel (cluster code: 1284/0x504)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace Channel {

inline constexpr ClusterId kClusterId = 0x00000504;

namespace Attributes {
namespace ChannelList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ChannelList
namespace Lineup {
inline constexpr AttributeId Id = 0x00000001;
} // namespace Lineup
namespace CurrentChannel {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentChannel

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace ChangeChannel {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeChannel
namespace ChangeChannelByNumber {
inline constexpr CommandId Id = 0x00000002;
} // namespace ChangeChannelByNumber
namespace SkipChannel {
inline constexpr CommandId Id = 0x00000003;
} // namespace SkipChannel
namespace GetProgramGuide {
inline constexpr CommandId Id = 0x00000004;
} // namespace GetProgramGuide
namespace RecordProgram {
inline constexpr CommandId Id = 0x00000006;
} // namespace RecordProgram
namespace CancelRecordProgram {
inline constexpr CommandId Id = 0x00000007;
} // namespace CancelRecordProgram
} // namespace Commands

namespace Events {} // namespace Events
} // namespace Channel
} // namespace clusters
} // namespace app
} // namespace chip
