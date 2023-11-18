/**
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DemandResponseLoadControl {

inline constexpr uint8_t kProgramIDSize = 16; // Per spec
inline constexpr uint8_t kEventIDSize   = 16; // Per spec

/**
 * Representation of a load control program that includes space to store the
 * ProgramID and name.
 */
class LoadControlProgram : protected Structs::LoadControlProgramStruct::Type
{
public:
    static constexpr uint8_t kMaxNameSize = 32; // Per spec

    template <typename T>
    using Nullable = DataModel::Nullable<T>;
    using Super    = Structs::LoadControlProgramStruct::Type;

    using Super::Encode;
    using Super::kIsFabricScoped;

    LoadControlProgram() {}

    // aName must be at most MaxNameSize() in length.
    CHIP_ERROR Init(const FixedByteSpan<kProgramIDSize> & aProgramID, const CharSpan & aName,
                    const Nullable<uint8_t> & aRandomStartMinutes, const Nullable<uint8_t> & aRandomDurationMinutes);

    // Accessors for const state.
    ByteSpan GetProgramID() const { return programID; }

private:
    uint8_t mProgramIDBuffer[kProgramIDSize];
    char mNameBuffer[kMaxNameSize];
};

/**
 * Delegate that needs to be implemented by the application.
 */
class Delegate
{
public:
    using Status = Protocols::InteractionModel::Status;

    virtual ~Delegate() {}

    /**
     * Get the value of the NumberOfLoadControlPrograms fixed attribute.
     */
    virtual uint8_t GetNumberOfLoadControlPrograms() = 0;

    /**
     * Get the value of the NumberOfEventsPerProgram fixed attribute.
     */
    virtual uint8_t GetNumberOfEventsPerProgram() = 0;

    /**
     * Get the value of the NumberOfTransitions fixed attribute.
     */
    virtual uint8_t GetNumberOfTransitions() = 0;

    /**
     * Get the value of the DefaultRandomStart non-volatile attribute.
     */
    virtual uint8_t GetDefaultRandomStart() = 0;

    /**
     * Set the value of the DefaultRandomStart non-volatile attribute.  It's the
     * callee's responsibility to persist the new value on success.
     *
     * The value passed in here has already been checked to satisfy the
     * specification's constraints.
     */
    virtual CHIP_ERROR SetDefaultRandomStart(uint8_t aNewValue) = 0;

    /**
     * Get the value of the DefaultRandomDuration non-volatile attribute.
     */
    virtual uint8_t GetDefaultRandomDuration() = 0;

    /**
     * Set the value of the DefaultRandomDuration non-volatile attribute.  It's
     * the callee's responsibility to persist the new value on success.
     *
     * The value passed in here has already been checked to satisfy the
     * specification's constraints.
     */
    virtual CHIP_ERROR SetDefaultRandomDuration(uint8_t aNewValue) = 0;

    /**
     * Get the load control program at the given index in the
     * LoadControlPrograms list.  The callee must call Init() on
     * aLoadControlProgram with the right data.
     *
     * @param aIndex [in] the 0-based index into the list.
     * @param aLoadControlProgram [out] on success, has had Init() called on it
     *        with the relevant values.
     *
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of bounds.
     */
    virtual CHIP_ERROR GetLoadControlProgram(size_t aIndex, LoadControlProgram & aLoadControlProgram) = 0;

    /**
     * Get the ID of the active event at the given index in the ActiveEvents list.
     *
     * @param aIndex [in] the 0-based index into the list.
     * @param aEventID [out] on success, the buffer has been filled with the event id.
     *
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of bounds.
     */
    virtual CHIP_ERROR GetActiveEventID(size_t aIndex, FixedSpan<uint8_t, kEventIDSize> & aEventID) = 0;

    /**
     * Update an existing LoadControlProgramStruct with the provided one.  The
     * caller guarantees that the two structs have the same ProgramID, and the
     * index is the index that was used to retrieve the existing struct.
     */
    virtual Status ReplaceLoadControlProgram(size_t aIndex,
                                             const Structs::LoadControlProgramStruct::DecodableType & aNewProgram) = 0;

    /**
     * Add a new LoadControlProgramStruct.  The caller guarantees that there are
     * fewer than GetNumberOfLoadControlPrograms() entries already in the list.
     */
    virtual Status AddLoadControlProgram(const Structs::LoadControlProgramStruct::DecodableType & aNewProgram) = 0;

    /**
     * Remove the load control program with the given id.  The caller guarantees
     * that the id follows spec-defined constraints.
     */
    virtual Status RemoveLoadControlProgram(const ByteSpan & aProgramID) = 0;

    /**
     * Add a new load control event.  The caller has already done the
     * spec-required validations.
     */
    virtual Status AddLoadControlEvent(const Structs::LoadControlEventStruct::DecodableType & aEvent) = 0;

    /**
     * Remove all load control events.
     */
    virtual Status ClearLoadControlEvents() = 0;
};

/**
 * Instance of a DRLC cluster on a given endpoint, using the given delegate.
 */
template <bool ClearLoadControlEventsRequestSupported>
class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Instance(EndpointId aEndpoint, Delegate & aDelegate);
    ~Instance();

    CHIP_ERROR Init();
    void Shutdown();

private:
    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    void HandleRegisterLoadControlProgramRequest(HandlerContext & ctx,
                                                 const Commands::RegisterLoadControlProgramRequest::DecodableType & commandData);
    void
    HandleUnregisterLoadControlProgramRequest(HandlerContext & ctx,
                                              const Commands::UnregisterLoadControlProgramRequest::DecodableType & commandData);
    void HandleAddLoadControlEventRequest(HandlerContext & ctx,
                                          const Commands::AddLoadControlEventRequest::DecodableType & commandData);
    void HandleRemoveLoadControlEventRequest(HandlerContext & ctx,
                                             const Commands::RemoveLoadControlEventRequest::DecodableType & commandData);
    void HandleClearLoadControlEventsRequest(HandlerContext & ctx,
                                             const Commands::ClearLoadControlEventsRequest::DecodableType & commandData);

    Delegate & mDelegate;
    EndpointId mEndpoint;
};

} // namespace DemandResponseLoadControl
} // namespace Clusters
} // namespace app
} // namespace chip
