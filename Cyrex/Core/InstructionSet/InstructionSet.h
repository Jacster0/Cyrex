#include <vector>
#include <array>
#include <bitset>
#include <map>

namespace Cyrex {
    struct Vendor {
        static constexpr auto Intel                  = "GenuineIntel";
        static constexpr auto AMD                    = "AuthenticAMD";
        static constexpr auto AMD_K5                 = "AMDisbetter";
        static constexpr auto Centaur                = "CentaurHauls";
        static constexpr auto Cyrix                  = "CyrixInstead";
        static constexpr auto Transmeta              = "TransmetaCPU";
        static constexpr auto Transmeta2             = "GenuineTMx86";
        static constexpr auto National_Semiconductor = "Geode by NSC";
        static constexpr auto NexGen                 = "NexGenDriven";
        static constexpr auto Rise                   = "RiseRiseRise";
        static constexpr auto SiS                    = "SiS SiS SiS ";
        static constexpr auto UMC                    = "UMC UMC UMC ";
        static constexpr auto VIA                    = "VIA VIA VIA ";
        static constexpr auto Vortex                 = "Vortex86 SoC";
        static constexpr auto Zhaoxin                = " Shanghai ";
        static constexpr auto Hygon                  = "HygonGenuine";
        static constexpr auto MCST_Elbrus            = "E2K MACHINE";
        static constexpr auto Bhyve                  = "bhyve bhyve ";
        static constexpr auto KVM                    = " KVMKVMKVM ";
        static constexpr auto QEMU                   = " TCGTCGTCGTCG ";
        static constexpr auto Windows_Virtual_PC     = " Microsoft Hv ";
        static constexpr auto Parallels              = " lrpepyh vr ";
        static constexpr auto VMware                 = "VMwareVMware ";
        static constexpr auto Xen_HVM                = "XenVMMXenVMM ";
        static constexpr auto Project_ACRN           = "ACRNACRNACRN ";
        static constexpr auto QNX                    = " QNXQVMBSQG  ";
        static constexpr auto Apple_Rosetta_2        = " VirtualApple  ";
    };

    using Register32 = int32_t;
    using Register8  = char;

    struct Registers32_t {
        Register32 EAX;
        Register32 EBX;
        Register32 ECX;
        Register32 EDX;
    };

    struct Registers8_t {
        std::array<Register8, 4> EAX;
        std::array<Register8, 4> EBX;
        std::array<Register8, 4> ECX;
        std::array<Register8, 4> EDX;
    };

    class CpuData {
    public:
        [[nodiscard]] constexpr std::array<Register32*, 4> Data32() const noexcept { return { std::bit_cast<Register32*>(&m_regs32) }; }
        [[nodiscard]] constexpr std::array<Register8*, 16> Data8()  const noexcept { return { std::bit_cast<Register8*>(&m_regs32) }; }

        [[nodiscard]] constexpr Registers32_t Regs32() const noexcept { return m_regs32; }
        [[nodiscard]] constexpr Registers8_t  Regs8()  const noexcept { return *std::bit_cast<Registers8_t*>(&m_regs32); }
    private:
        Registers32_t m_regs32;
    };

    class InstructionSet {
    public:
        InstructionSet() noexcept;

        //BitsetFlags for function with EAX set to 0x00000001
        //ECX
        [[nodiscard]] constexpr bool SSE3()        const noexcept { return m_bitsetFlags[0][0];  }
        [[nodiscard]] constexpr bool PCLMULQDQ()   const noexcept { return m_bitsetFlags[0][1];  }
        [[nodiscard]] constexpr bool MONITOR()     const noexcept { return m_bitsetFlags[0][3];  }
        [[nodiscard]] constexpr bool SSSE3()       const noexcept { return m_bitsetFlags[0][9];  }
        [[nodiscard]] constexpr bool FMA()         const noexcept { return m_bitsetFlags[0][12]; }
        [[nodiscard]] constexpr bool CMPXCHG16B()  const noexcept { return m_bitsetFlags[0][13]; }
        [[nodiscard]] constexpr bool SSE41()       const noexcept { return m_bitsetFlags[0][19]; }
        [[nodiscard]] constexpr bool SSE42()       const noexcept { return m_bitsetFlags[0][20]; }
        [[nodiscard]] constexpr bool MOVBE()       const noexcept { return m_bitsetFlags[0][22]; }
        [[nodiscard]] constexpr bool POPCNT()      const noexcept { return m_bitsetFlags[0][23]; }
        [[nodiscard]] constexpr bool AES()         const noexcept { return m_bitsetFlags[0][25]; }
        [[nodiscard]] constexpr bool XSAVE()       const noexcept { return m_bitsetFlags[0][26]; }
        [[nodiscard]] constexpr bool OSXSAVE()     const noexcept { return m_bitsetFlags[0][27]; }
        [[nodiscard]] constexpr bool AVX()         const noexcept { return m_bitsetFlags[0][28]; }
        [[nodiscard]] constexpr bool F16C()        const noexcept { return m_bitsetFlags[0][29]; }
        [[nodiscard]] constexpr bool RDRAND()      const noexcept { return m_bitsetFlags[0][30]; }

        //EDX
        [[nodiscard]] constexpr bool MSR()         const noexcept { return m_bitsetFlags[1][5];  }
        [[nodiscard]] constexpr bool CX8()         const noexcept { return m_bitsetFlags[1][8];  }
        [[nodiscard]] constexpr bool SEP()         const noexcept { return m_bitsetFlags[1][11]; }
        [[nodiscard]] constexpr bool CMOV()        const noexcept { return m_bitsetFlags[1][15]; }
        [[nodiscard]] constexpr bool CLFSH()       const noexcept { return m_bitsetFlags[1][19]; }
        [[nodiscard]] constexpr bool MMX()         const noexcept { return m_bitsetFlags[1][23]; }
        [[nodiscard]] constexpr bool FXSR()        const noexcept { return m_bitsetFlags[1][24]; }
        [[nodiscard]] constexpr bool SSE()         const noexcept { return m_bitsetFlags[1][25]; }
        [[nodiscard]] constexpr bool SSE2()        const noexcept { return m_bitsetFlags[1][26]; }

        //BitsetFlags for function with EAX set to 0x00000007
        //EBX
        [[nodiscard]] constexpr bool FSGSBASE()    const noexcept { return m_bitsetFlags[2][0];  }
        [[nodiscard]] constexpr bool BMI1()        const noexcept { return m_bitsetFlags[2][3];  }
        [[nodiscard]] constexpr bool AVX2()        const noexcept { return m_bitsetFlags[2][5];  }
        [[nodiscard]] constexpr bool BMI2()        const noexcept { return m_bitsetFlags[2][8];  }
        [[nodiscard]] constexpr bool ERMS()        const noexcept { return m_bitsetFlags[2][9];  }
        [[nodiscard]] constexpr bool INVPCID()     const noexcept { return m_bitsetFlags[2][10]; }
        [[nodiscard]] constexpr bool AVX512F()     const noexcept { return m_bitsetFlags[2][16]; }
        [[nodiscard]] constexpr bool RDSEED()      const noexcept { return m_bitsetFlags[2][18]; }
        [[nodiscard]] constexpr bool ADX()         const noexcept { return m_bitsetFlags[2][19]; }
        [[nodiscard]] constexpr bool AVX512PF()    const noexcept { return m_bitsetFlags[2][26]; }
        [[nodiscard]] constexpr bool AVX512ER()    const noexcept { return m_bitsetFlags[2][27]; }
        [[nodiscard]] constexpr bool AVX512CD()    const noexcept { return m_bitsetFlags[2][28]; }
        [[nodiscard]] constexpr bool SHA()         const noexcept { return m_bitsetFlags[2][29]; }

        [[nodiscard]] constexpr bool HLE()         const noexcept { return m_isIntel && m_bitsetFlags[2][4];  }
        [[nodiscard]] constexpr bool RTM()         const noexcept { return m_isIntel && m_bitsetFlags[2][11]; }

        //ECX
        [[nodiscard]] constexpr bool PREFETCHWT1() const noexcept { return m_bitsetFlags[3][0]; }

        //BitsetFlags for function with EAX set to 0x80000001
        //ECX
        [[nodiscard]] constexpr bool LAHF()        const noexcept { return m_bitsetFlags[4][0]; }
        [[nodiscard]] constexpr bool LZCNT()       const noexcept { return m_isIntel && m_bitsetFlags[4][5]; }
        [[nodiscard]] constexpr bool ABM()         const noexcept { return m_isAMD && m_bitsetFlags[4][5];   }
        [[nodiscard]] constexpr bool SSE4a()       const noexcept { return m_isAMD && m_bitsetFlags[4][6];   }
        [[nodiscard]] constexpr bool XOP()         const noexcept { return m_isAMD && m_bitsetFlags[4][11];  }
        [[nodiscard]] constexpr bool TBM()         const noexcept { return m_isAMD && m_bitsetFlags[4][21];  }

        //EDX
        [[nodiscard]] constexpr bool SYSCALL()     const noexcept { return m_isIntel && m_bitsetFlags[5][11]; }
        [[nodiscard]] constexpr bool RDTSCP()      const noexcept { return m_isIntel && m_bitsetFlags[5][27]; }

        [[nodiscard]] constexpr bool MMXEXT()      const noexcept { return m_isAMD && m_bitsetFlags[5][22]; }
        [[nodiscard]] constexpr bool _3DNOWEXT()   const noexcept { return m_isAMD && m_bitsetFlags[5][30]; }
        [[nodiscard]] constexpr bool _3DNOW()      const noexcept { return m_isAMD && m_bitsetFlags[5][31]; }

        [[nodiscard]] const std::string& Vendor()      const noexcept { return m_vendor;      }
        [[nodiscard]] const std::string& Brandstring() const noexcept { return m_brandstring; }
    private:
        const [[nodiscard]] std::string CaptureVendor()      const noexcept;
        const [[nodiscard]] std::string CaptureBrandString() const noexcept;

        int m_ids{};
        int m_exIds{};

        bool m_isIntel{};
        bool m_isAMD{};

        CpuData m_cpuData;
        std::array<std::bitset<32>, 6> m_bitsetFlags{};

        std::vector<CpuData> m_data{};
        std::vector<CpuData> m_extdata{};

        std::string m_vendor{};
        std::string m_brandstring{};
    };
}
