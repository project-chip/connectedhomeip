#include "ClusterTester.h"

namespace chip {
namespace Testing {

template <typename T>
app::DataModel::ActionReturnStatus chip::Testing::ClusterTester::ReadAttribute(const app::DataModel::ReadAttributeRequest & path, T & out)
{
    return app::DataModel::ActionReturnStatus();
}

template <typename T>
app::DataModel::ActionReturnStatus ClusterTester::WriteAttribute(const app::DataModel::WriteAttributeRequest & path,
                                                                 const T & value)
{
    return app::DataModel::ActionReturnStatus();
}

template <typename T>
std::optional<std::variant<app::DataModel::ActionReturnStatus, CommandResponse>>
ClusterTester::InvokeCommand(const app::DataModel::InvokeRequest & request, const T & arguments)
{
    return std::optional<std::variant<app::DataModel::ActionReturnStatus, CommandResponse>>();
}

EventInformation ClusterTester::GetNextGeneratedEvent()
{
    return EventInformation();
}

CHIP_ERROR ClusterTester::TestGeneratedCommands(const app::ConcreteClusterPath & path, Span<CommandId> compareWith)
{
    return CHIP_ERROR();
}

CHIP_ERROR ClusterTester::TestAcceptedCommands(const app::ConcreteClusterPath & path,
                                               Span<app::DataModel::AcceptedCommandEntry> compareWith)
{
    return CHIP_ERROR();
}

CHIP_ERROR ClusterTester::TestAttributes(const app::ConcreteClusterPath & path, Span<app::DataModel::AttributeEntry> compareWith)
{
    return CHIP_ERROR();
}

} // namespace Testing
} // namespace chip