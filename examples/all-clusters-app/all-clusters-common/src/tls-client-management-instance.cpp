/*
 *
 *    Copyright (c) 2025 Matter Authors
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

#include <clusters/TlsCertificateManagement/Commands.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/tls-client-management-server/tls-client-management-server.h>
#include <tls-client-management-instance.h>
#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsClientManagement;

TlsClientManagementCommandDelegate TlsClientManagementCommandDelegate::instance;

static CertificateTableImpl gCertificateTableInstance;
static TlsClientManagementServer gTlsClientManagementClusterServerInstance =
    TlsClientManagementServer(EndpointId(1), TlsClientManagementCommandDelegate::getInstance(), gCertificateTableInstance);

void emberAfTlsClientManagementClusterInitCallback(EndpointId endpoint)
{
    gCertificateTableInstance.SetEndpoint(EndpointId(1));
    gTlsClientManagementClusterServerInstance.Init();
}

void emberAfTlsClientManagementClusterShutdownCallback(EndpointId endpoint)
{
    gTlsClientManagementClusterServerInstance.Finish();
}
