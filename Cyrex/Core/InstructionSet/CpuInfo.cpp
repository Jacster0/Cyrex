#include "CpuInfo.h"
#include <cassert>



Cyrex::CPUInfo::CPUInfo() {
    GetNativeSystemInfo(&sysInfo);

    Info.BrandString     = GetBrandString();
    Info.Vendor          = GetVendor();
    Info.Architecture    = GetArchitecture();
    Info.NumberOfThreads = GetNumberOfLogicalProcessors();
    Info.NumberOfCores   = GetNumberOfPhysicalProcessors();

    Info.InstructionSetFeatures = {
        {"SSE3",        ins.SSE3()},
        {"PCLMULQDQ",   ins.PCLMULQDQ()},
        {"MONITOR",     ins.MONITOR()},
        {"FMA",         ins.FMA()},
        {"CMPXCHG16B",  ins.CMPXCHG16B()},
        {"SSE4.1",      ins.SSE41()},
        {"SSE4.2",      ins.SSE42()},
        {"MOVBE",       ins.MOVBE()},
        {"POPCNT",      ins.POPCNT()},
        {"AES",         ins.AES()},
        {"XSAVE",       ins.XSAVE()},
        {"OSXSAVE",     ins.OSXSAVE()},
        {"AVX",         ins.AVX()},
        {"F16C",        ins.F16C()},
        {"RDRAND",      ins.RDRAND()},
        {"MSR",         ins.MSR()},
        {"CX8",         ins.CX8()},
        {"SEP",         ins.SEP()},
        {"CMOV",        ins.CMOV()},
        {"CLFSH",       ins.CLFSH()},
        {"MMX",         ins.MMX()},
        {"FXSR",        ins.FXSR()},
        {"SSE",         ins.SSE()},
        {"SSE2",        ins.SSE2()},
        {"FSGSBASE",    ins.FSGSBASE()},
        {"BMI1",        ins.BMI1()},
        {"HLE",         ins.HLE()},
        {"AVX2",        ins.AVX2()},
        {"BMI2",        ins.BMI2()},
        {"ERMS",        ins.ERMS()},
        {"INVPCID",     ins.INVPCID()},
        {"RTM",         ins.RTM()},
        {"AVX512F",     ins.AVX512F()},
        {"RDSEED",      ins.RDSEED()},
        {"ADX",         ins.ADX()},
        {"AVX512PF",    ins.AVX512PF()},
        {"AVX512ER",    ins.AVX512ER()},
        {"AVX512CD",    ins.AVX512CD()},
        {"SHA",         ins.SHA()},
        {"PREFETCHWT1", ins.PREFETCHWT1()},
        {"LAHF",        ins.LAHF()},
        {"LZCNT",       ins.LZCNT()},
        {"ABM",         ins.ABM()},
        {"SSE4a",       ins.SSE4a()},
        {"XOP",         ins.XOP()},
        {"TBM",         ins.TBM()},
        {"SYSCALL",     ins.SYSCALL()},
        {"MMXEXT",      ins.MMXEXT()},
        {"RDTSCP",      ins.RDTSCP()},
        {"3DNOWEXT",    ins._3DNOWEXT()},
        {"3DNOW",       ins._3DNOW()}
    };
}

const int Cyrex::CPUInfo::GetNumberOfPhysicalProcessors() const noexcept {
    DWORD length{ 0 };
    size_t offset{ 0 };
    int numCores{ 0 };

    //retrieve the buffer length
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);

    std::unique_ptr<std::byte[]> buffer(new std::byte[length]);

    if (GetLogicalProcessorInformationEx(
        RelationProcessorCore,
        reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get()),
        &length))
    {
        do {
            const auto processorInformation = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get() + offset);

            offset += processorInformation->Size;
            numCores++;

        } while (offset < length);
    }
    return numCores;
}
