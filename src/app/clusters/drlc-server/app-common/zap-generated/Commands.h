namespace DemandResponseLoadControl {
namespace Commands {

namespace RegisterLoadControlProgramRequest {
static constexpr CommandId Id = 0x00000000;
} // namespace RegisterLoadControlProgramRequest

namespace UnregisterLoadControlProgramRequest {
static constexpr CommandId Id = 0x00000001;
} // namespace UnregisterLoadControlProgramRequest

namespace AddLoadControlEventRequest {
static constexpr CommandId Id = 0x00000002;
} // namespace AddLoadControlEventRequest

namespace RemoveLoadControlEventRequest {
static constexpr CommandId Id = 0x00000003;
} // namespace RemoveLoadControlEventRequest

namespace ClearLoadControlEventsRequest {
static constexpr CommandId Id = 0x00000004;
} // namespace ClearLoadControlEventsRequest

} // namespace Commands
} // namespace DemandResponseLoadControl

