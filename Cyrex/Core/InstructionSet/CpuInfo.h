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
            int NumLogicalProcessors;
            int NumCores;
            InstructionSet InstructionSetFeatures;
        } Info;
    private:
        [[nodiscard]] const std::string& GetBrandString()  const noexcept { return m_instructionSet.Brandstring(); }
        [[nodiscard]] const std::string& GetVendor()       const noexcept { return m_instructionSet.Vendor(); }
        [[nodiscard]] const std::string& GetArchitecture() const noexcept { return m_architectures.at(m_sysInfo.wProcessorArchitecture); }

        [[nodiscard]] const uint32_t GetNumberOfLogicalProcessors() const noexcept { return std::thread::hardware_concurrency(); }
        [[nodiscard]] const uint32_t GetNumberOfCores() const noexcept;

        SYSTEM_INFO m_sysInfo;
        InstructionSet m_instructionSet;
        std::unordered_map<int, std::string> m_architectures{
            {9, "x86-64"},
            {5, "ARM"},
            {12, "ARM64"},
            {6, "Intel Itanium-based"},
            {0, "x86-32"},
            {0xFFFF, "Unknown architecture."}
        };
    };
}
