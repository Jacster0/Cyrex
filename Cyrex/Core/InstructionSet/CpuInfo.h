#pragma once
#include "Platform/Windows/CrxWindow.h"
#include <string>
#include "InstructionSet.h"
#include <array>
#include <unordered_map>
#include <thread>

namespace Cyrex {
    class CPUInfo {
    public:
        CPUInfo();
        
        struct {
            std::string BrandString;
            std::string Vendor;
            std::string Architecture;
            int NumberOfThreads;
            int NumberOfCores;
            std::map<std::string, bool> InstructionSetFeatures;
        } Info;
    private:
        const std::string GetBrandString()  const noexcept { return ins.Brandstring(); }
        const std::string GetVendor()       const noexcept { return ins.Vendor(); }
        const std::string GetArchitecture() const noexcept { return architectures.at(sysInfo.wProcessorArchitecture); }

        const int GetNumberOfLogicalProcessors() const noexcept { return std::thread::hardware_concurrency(); }
        const int GetNumberOfPhysicalProcessors() const noexcept;

        SYSTEM_INFO sysInfo;
        InstructionSet ins;
        std::unordered_map<int, std::string> architectures{
            {9, "x86-64"},
            {5, "ARM"},
            {12, "ARM64"},
            {6, "Intel Itanium-based"},
            {0, "x86-32"},
            {0xFFFF, "Unknown architecture."}
        };
    };
}
