# AcceleratorCS2

A fork of [AcceleratorLocal](https://github.com/komashchenko/AcceleratorLocal) which is a fork of [asherkin's accelerator](https://github.com/asherkin/accelerator) using Premake with Linux and Windows support.

## Configuration

The configuration file is located at `AcceleratorCS2/config.json`. It contains a `MinidumpAccountSteamId64` field for you to specify your steamid64. This allows you to view full details for your crash dumps on [Throttle](https://crash.limetech.org/).

## Running Accelerator on Windows with CounterStrikeSharp crashes the server on startup

Run the server with `-DoNotPreloadDLLs` startup parameter. The engine loops over every single loaded dll and tries to read an address in every single page of memory. This causes access violations for the .NET binaries that Accelerator catches and handles (dumps and quits).