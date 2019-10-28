# gmsv_server_monitoring

Quick module thrown together to allow Lua to retrieve information about memory usage, CPU usage, etc. on the server.

## Lua API Example

```lua
if (ServerMonitoring == nil) then require("server_monitoring") end

local CPUUsage            = ServerMonitoring:GetCPUUsage()    -- 0-100
local TotalPhysicalMemory = ServerMonitoring:GetTotalMemory() -- bytes
local PhysicalMemoryUsage = ServerMonitoring:GetMemoryUsage() -- bytes

print(CPUUsage, TotalPhysicalMemory, PhysicalMemoryUsage)
```

>0.10822510822511 33364553728 108796

## Installation

1. [Download the module](https://github.com/WilliamVenner/gmsv_server_monitoring/releases); make sure to select the correct .dll for your server's operating system (Linux or Windows)
2. On your server, create the folder `garrysmod/lua/bin` if it doesn't already exist
3. Drop the .dll in the folder
