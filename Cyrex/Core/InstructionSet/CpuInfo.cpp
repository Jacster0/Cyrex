#include "CpuInfo.h"
#include <cassert>
#include <algorithm>
#include <ranges>
#include <iostream>

namespace views = std::ranges::views;

using namespace Cyrex;

CPUInfo::CPUInfo() {
    GetNativeSystemInfo(&m_sysInfo);

    Info.BrandString            = GetBrandString();
    Info.Vendor                 = GetVendor();
    Info.Architecture           = GetArchitecture();
    Info.NumLogicalProcessors   = GetNumberOfLogicalProcessors();
    Info.NumCores               = GetNumberOfCores();
    Info.InstructionSetFeatures = std::move(m_instructionSet);
}

const uint32_t CPUInfo::GetNumberOfCores() const noexcept {
    DWORD length{ 0 };
    uint32_t numCores{ 0 };
    constexpr auto filterLambda = [](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION& elem) {
        return elem.Relationship == RelationProcessorCore; 
    };

    //retrieve the buffer length in bytes
    GetLogicalProcessorInformation(nullptr, &length);

    //create the buffer
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));

    //Fill the buffer. If the function fails we return 0 because the contents of the buffer will be undefined
    if (not GetLogicalProcessorInformation(buffer.data(), &length)) [[unlikely]] {
        return numCores;
    }

    //Count the number of physical cores on the users cpu.
    for ([[maybe_unused]] const auto elem : buffer | views::filter(filterLambda)) {
        numCores++;
    }

    return numCores;
}
