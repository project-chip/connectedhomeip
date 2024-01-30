/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MCClusterObjects.h"

#import "MCAttribute_Internal.h"
#import "MCCluster_Internal.h"
#import "MCCommand_Internal.h"

#include "core/Attribute.h"
#include "core/Command.h"
#include <app-common/zap-generated/cluster-objects.h>

#import <Foundation/Foundation.h>

@implementation MCContentLauncherCluster
- (id)launchURLCommand
{
    void * cppCommand = self.cppCluster->GetCommand(chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Id);
    return cppCommand != nil ? [[MCContentLauncherClusterLaunchURLCommand alloc] initWithCppCommand:cppCommand] : nil;
}
@end

@implementation MCApplicationBasicCluster
- (id)vendorIDAttribute
{
    void * cppAttribute = self.cppCluster->GetAttribute(chip::app::Clusters::ApplicationBasic::Attributes::VendorID::Id);
    return cppAttribute != nil ? [[MCApplicationBasicClusterVendorIDAttribute alloc] initWithCppAttribute:cppAttribute] : nil;
}
@end

@implementation MCMediaPlaybackCluster
- (id)currentStateAttribute
{
    void * cppAttribute = self.cppCluster->GetAttribute(chip::app::Clusters::MediaPlayback::Attributes::CurrentState::Id);
    return cppAttribute != nil ? [[MCMediaPlaybackClusterCurrentStateAttribute alloc] initWithCppAttribute:cppAttribute] : nil;
}
@end
