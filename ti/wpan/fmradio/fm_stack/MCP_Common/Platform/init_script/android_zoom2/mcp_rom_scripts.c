/*
 * TI's FM Stack
 *
 * Copyright 2001-2010 Texas Instruments, Inc. - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Automatically Generated on 'Wednesday 28th of April 2010 12:00:41 PM' */

#include "mcp_hal_types.h"
#include "mcp_hal_defs.h"
#include "mcp_rom_scripts_db.h"

#define MCP_ROM_SCRIPTS_BUF_SIZE_1	((McpUint)	32)

static const McpU8 mcpRomScripts_Buf_1[MCP_ROM_SCRIPTS_BUF_SIZE_1] =
{
	0x42, 0x54, 0x53, 0x42, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

extern const McpRomScripts_Data mcpRomScripts_Data[];

const McpRomScripts_Data mcpRomScripts_Data[] = {
	{"tipex.bts", MCP_ROM_SCRIPTS_BUF_SIZE_1, mcpRomScripts_Buf_1}
};

extern const McpUint mcpRomScripts_NumOfScripts;

const McpUint mcpRomScripts_NumOfScripts = 1;

