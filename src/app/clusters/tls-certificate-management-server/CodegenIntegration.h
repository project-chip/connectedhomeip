/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/clusters/tls-certificate-management-server/TLSCertificateManagementCluster.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * Set the delegate for the TLS Certificate Management cluster.
 *
 * MUST be called before Matter data model initialization (e.g. in main() before ServerInit()).
 * The delegate is used during cluster initialization.
 * If called after cluster initialization, it will have no effect as the cluster instance
 * has already been created with the previous delegate.
 *
 * If not called before initialization, the cluster will NOT be initialized.
 *
 * @param delegate The delegate to be used by the TLS Certificate Management cluster.
 */
void MatterTlsCertificateManagementSetDelegate(TLSCertificateManagementDelegate & delegate);

/**
 * Set the certificate table for the TLS Certificate Management cluster.
 *
 * MUST be called before Matter data model initialization (e.g. in main() before ServerInit()).
 * The certificate table is used during cluster initialization via emberAfTLSCertificateManagementClusterInitCallback.
 * If called after cluster initialization, it will have no effect as the cluster instance
 * has already been created with the previous certificate table.
 *
 * If not called before initialization, a default certificate table will be used.
 *
 * @param certificateTable The certificate table to be used by the TLS Certificate Management cluster.
 */
void MatterTlsCertificateManagementSetCertificateTable(Tls::CertificateTableImpl & certificateTable);

/**
 * Set the dependency checker for the TLS Certificate Management cluster.
 *
 * MUST be called before Matter data model initialization (e.g. in main() before ServerInit()).
 * The dependency checker is used during cluster initialization via emberAfTLSCertificateManagementClusterInitCallback.
 * If called after cluster initialization, it will have no effect as the cluster instance
 * has already been created with the previous dependency checker.
 *
 * If not called before initialization, a default dependency checker will be used.
 *
 * @param dependencyChecker The dependency checker to be used by the TLS Certificate Management cluster.
 */
void MatterTlsCertificateManagementSetDependencyChecker(Tls::CertificateDependencyChecker & dependencyChecker);

} // namespace Clusters
} // namespace app
} // namespace chip
