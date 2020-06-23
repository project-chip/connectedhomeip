/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Definitions for the Trust Center Backup plugin.
 *******************************************************************************
   ******************************************************************************/

// *****************************************************************************
// * trust-center-backup.h
// *
// * Header for backing up and restoring a trust center.
// *
// * Copyright 2011 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

EmberStatus emberTrustCenterExportBackupData(EmberAfTrustCenterBackupData* backup);
EmberStatus emberTrustCenterImportBackupAndStartNetwork(const EmberAfTrustCenterBackupData* backup);

// Available only for EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT
void emAfTcExportCommand(void);
void emAfTcImportCommand(void);
EmberStatus emberAfTrustCenterImportBackupFromFile(const char* filepath);
EmberStatus emberAfTrustCenterExportBackupToFile(const char* filepath);
