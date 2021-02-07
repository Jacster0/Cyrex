#include "InstructionSet.h"
#include <intrin.h>

using namespace Cyrex;

InstructionSet::InstructionSet() noexcept {
    __cpuid(*m_cpuData.Data32().data(), 0);

    m_ids = m_cpuData.Regs32().EAX;

    //Capture the cpu vendor name
    m_vendor = CaptureVendor();

    for (int i = 0; i <= m_ids; ++i) {
        __cpuidex(*m_cpuData.Data32().data(), i, 0);
        m_data.push_back(m_cpuData);
    }

    if (m_vendor == Vendor::Intel) {
        m_isIntel = true;
    }
    else if (m_vendor == Vendor::AMD) {
        m_isAMD = true;
    }

    // load bitset with flags for function 0x00000001
    if (m_ids >= 1) {
        m_bitsetFlags[0] = m_data[1].Regs32().ECX;
        m_bitsetFlags[1] = m_data[1].Regs32().EDX;
    }

    // load bitset with flags for function 0x00000007
    if (m_ids >= 7) {
        m_bitsetFlags[2] = m_data[7].Regs32().EBX;
        m_bitsetFlags[3] = m_data[7].Regs32().ECX;
    }

    __cpuid(*m_cpuData.Data32().data(), 0x80000000);
    m_exIds = m_cpuData.Regs32().EAX;

    //Capture the cpu BrandString 
    m_brandstring = CaptureBrandString();

    for (int i = 0x80000000; i <= m_exIds; ++i) {
        __cpuidex(*m_cpuData.Data32().data(), i, 0);
        m_extdata.push_back(m_cpuData);
    }

    // load bitset with flags for function 0x80000001
    if (m_exIds >= 0x80000001) {
        m_bitsetFlags[4] = m_extdata[1].Regs32().ECX;
        m_bitsetFlags[5] = m_extdata[1].Regs32().EDX;
    }
}


//Returns the 12 character CPU vendor ID string stored in EBX, EDX, ECX (in that order) 
const std::string InstructionSet::CaptureVendor() const noexcept {
    const auto& registers = m_cpuData.Regs8();

    return  std::string{}.append(registers.EBX.data(), registers.EBX.size())
                         .append(registers.EDX.data(), registers.EDX.size())
                         .append(registers.ECX.data(), registers.EDX.size());
}


//Returns the 48 byte null terminated cpu brandstring stored in EAX, EBX, ECX and EDX
//by calling the __cpuid intrinsic with EAX set to 0x80000002, 0x80000003 and 0x80000004
const std::string InstructionSet::CaptureBrandString() const noexcept {
    if (m_exIds < 0x80000004) [[unlikely]] {
        return "Unable to get CPU brand information";
    }

    static constexpr std::array cpuIDBrandStringCalls{ 0x80000002,0x80000003,0x80000004 };
    std::string brandString{};

    for (const auto& funcID : cpuIDBrandStringCalls) {
        __cpuid(*m_cpuData.Data32().data(), funcID);
        const auto& data8 = m_cpuData.Data8();

        brandString.append(*data8.data(), data8.size());
    }
    return brandString;
}
