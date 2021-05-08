#include "CpuInfo.h"
#include <cassert>
#include <algorithm>
#include <ranges>

namespace views = std::ranges::views;

using namespace Cyrex;

CPUInfo::CPUInfo() {
    GetNativeSystemInfo(&m_sysInfo);

    Info.Architecture           = GetArchitecture();
    Info.NumLogicalProcessors   = GetNumberOfLogicalProcessors();
    Info.NumCores               = GetNumberOfCores();
}

const uint32_t CPUInfo::GetNumberOfCores() const noexcept {
    DWORD length{ 0 };
    uint32_t numCores{ 0 };

    constexpr auto filterLambda = [](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION& elem) {
        return elem.Relationship == RelationProcessorCore; 
    };

    const auto coreCounterLambda = [&numCores]([[maybe_unused]] const SYSTEM_LOGICAL_PROCESSOR_INFORMATION& elem) {
        numCores++;
    };

    //retrieve the buffer length in bytes
    GetLogicalProcessorInformation(nullptr, &length);

    //create the buffer
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));

    //Fill the buffer. If the function succeed we can start counting cores.
    if (GetLogicalProcessorInformation(buffer.data(), &length)) [[likely]] {
        //Count the number of physical cores on the users cpu.
        std::ranges::for_each(buffer | views::filter(filterLambda), coreCounterLambda);
    }

    return numCores;
}
