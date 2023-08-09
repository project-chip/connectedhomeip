/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "attribute.hpp"
#include "mpc_command_sender.hpp"

using namespace attribute_store;

/**
 * @defgroup MPCNodeReportables
 * @ingroup mpc_components
 * @brief Class to maintain the information and handles reported each reportables that are setup
 * @{
 */

#ifndef MPCNODEREPORTABLES_HPP
#define MPCNODEREPORTABLES_HPP

class MPCNodeReportables
{
public:
    class MPCNodeReportablesDelegate : public chip::app::ReadClient::Callback
    {
    public:
        MPCNodeReportablesDelegate(MPCNodeReportables * ctxt, attribute node) : mCtxt(ctxt), mNode(node) {}

        void OnDone(chip::app::ReadClient * client) override;
        void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;

        void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override;

        void OnError(CHIP_ERROR aError) override;

        bool IsReportabledFailed() { return mFailureReported; }

    private:
        bool mFailureReported = false; // indicates if OnError has reported a failure
        MPCNodeReportables * mCtxt;    // hold this so we can free it in onDone callback
        attribute mNode;               // hold this to update when reports arrive
    };

    MPCNodeReportables(attribute node, NodeId dest, std::vector<AttributePathParams> & path, SubscribeRequestParams params) :
        mDelegate(this, node), mSubscribe(dest, path, params)
    {}
    sl_status_t Initiate();

    sl_status_t Terminate();

private:
    MPCNodeReportablesDelegate mDelegate;
    SubscribeRequest mSubscribe;
};

#endif // MPCNODEREPORTABLES_HPP
/** @} end MPCNodeReportables */
