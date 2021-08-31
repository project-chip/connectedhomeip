/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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




#include "support/ErrorStr.h"
#include "ShellTask.h"
#include "SerialManager.h"
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <stdio.h>
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
//namespace chip {
//namespace Shell {

ShellTask ShellTask::sShellTask;


int ShellTask::StartShellTask()
{
    BaseType_t xReturned;

    xReturned = xTaskCreate(ShellTask::ShellTaskMain, "chip_shell", 8192, NULL, 5, NULL);

    if (xReturned == pdPASS)
    {
	return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_MIN;//e ok acest cod?
}

char* ShellTask::ShellReadLine()
{
	char * line = (char *)CHIPPlatformMemoryCalloc(100, sizeof(uint8_t));
	uint8_t *pData;
	uint16_t bytesRead;
	int lineSize = 0;

	while (true)
	{
		Serial_Read(gShellSerMgrIf, pData, sizeof(uint8_t), bytesRead);
		if(pData[0] == '\n' || pData[0] == '\0')
		{
			break;
		}
		else
		{
			strncat(line, pData, bytesRead);
			lineSize += bytesRead;
		}
	}

	line[lineSize] = '\0';

	return line;

}


void ShellTask::ShellTaskMain(void * pvParameter)
{

	int ret = chip::Shell::streamer_init(chip::Shell::streamer_get());
	assert(ret == 0);
	cmd_ping_init();

	while (true)
	{
		const char * prompt = LOG_COLOR_I "> " LOG_RESET_COLOR;
		printf("%s\n", prompt);

	}
}

//}//namespace DeviceLayer
//}//namespace chip
