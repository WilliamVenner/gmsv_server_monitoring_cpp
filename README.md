# gmsv_server_monitoring

Quick module thrown together to allow Lua to retrieve information about memory usage, CPU usage, etc. on the server.

## Lua API Example

```lua
if (ServerMonitoring == nil) then require("server_monitoring") end

local CPUUsage            = ServerMonitoring:GetCPUUsage()    -- 0-100
local TotalPhysicalMemory = ServerMonitoring:GetTotalMemory() -- bytes
local PhysicalMemoryUsage = ServerMonitoring:GetMemoryUsage() -- bytes
local Is64Bit             = ServerMonitoring:Is64Bit()        -- true/false

print(CPUUsage, TotalPhysicalMemory, PhysicalMemoryUsage, Is64Bit)
```

>33364553728 108796 0.10822510822511 false