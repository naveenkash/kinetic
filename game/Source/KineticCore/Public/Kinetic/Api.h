// KINETICCORE_API marks what KineticCore exports to the modules that link it. Unreal's build
// defines it for the KineticCore module as DLLEXPORT, and as DLLIMPORT for the modules that use
// it (Windows needs both; Mac and Linux editor builds hide every other symbol). The headless
// build links everything into one binary, so for it it is empty.
#pragma once

#ifdef KINETICCORE_API
// KineticCore's own sources include no engine headers, so the engine's platform header that
// defines these has not been seen. These are its definitions, token for token, so a later
// engine include redefines them identically.
#ifndef DLLEXPORT
#if defined(_MSC_VER)
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLIMPORT __attribute__((visibility("default")))
#endif
#endif
#else
#define KINETICCORE_API
#endif
