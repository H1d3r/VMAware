/**
 * ██╗   ██╗███╗   ███╗ █████╗ ██╗    ██╗ █████╗ ██████╗ ███████╗
 * ██║   ██║████╗ ████║██╔══██╗██║    ██║██╔══██╗██╔══██╗██╔════╝
 * ██║   ██║██╔████╔██║███████║██║ █╗ ██║███████║██████╔╝█████╗
 * ╚██╗ ██╔╝██║╚██╔╝██║██╔══██║██║███╗██║██╔══██║██╔══██╗██╔══╝
 *  ╚████╔╝ ██║ ╚═╝ ██║██║  ██║╚███╔███╔╝██║  ██║██║  ██║███████╗
 *   ╚═══╝  ╚═╝     ╚═╝╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝ 1.4 (May 2024)
 *
 *  C++ VM detection library
 *
 *  - Made by: @kernelwernel (https://github.com/kernelwernel)
 *  - Contributed by:
 *      - @Requiem (https://github.com/NotRequiem)
 *      - @Alex (https://github.com/greenozon)
 *      - @Marek Knápek (https://github.com/MarekKnapek)
 *      - @Vladyslav Miachkov (https://github.com/fameowner99)
 *  - Repository: https://github.com/kernelwernel/VMAware
 *  - Docs: https://github.com/kernelwernel/VMAware/docs/documentation.md
 *  - Full credits: https://github.com/kernelwernel/VMAware#credits-and-contributors-%EF%B8%8F
 *  - License: MIT
 *
 *                               MIT License
 *  
 *  Copyright (c) 2024 kernelwernel
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * 
 * ================================ SECTIONS ==================================
 * - enums for publicly accessible techniques  => line 305
 * - struct for internal cpu operations        => line 492
 * - struct for internal memoization           => line 784
 * - struct for internal utility functions     => line 846
 * - struct for internal core components       => line 6589
 * - start of internal VM detection techniques => line 1522
 * - start of public VM detection functions    => line 6703
 * - start of externally defined variables     => line 7003
 * 
 * 
 * ================================ EXAMPLE ==================================
#include "vmaware.hpp"
#include <iostream>

int main() {
    if (VM::detect()) {
        std::cout << "Virtual machine detected!" << std::endl;
        std::cout << "VM name: " << VM::brand() << std::endl;
    } else {
        std::cout << "Running in baremetal" << std::endl;
    }
    
    std::cout << "VM certainty: " << (int)VM::percentage() << "%" << std::endl;
}

 */


#if (defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__))
#define MSVC 1
#define LINUX 0
#define APPLE 0
#elif (defined(__linux__))
#define MSVC 0
#define LINUX 1
#define APPLE 0
#elif (defined(__APPLE__) || defined(__APPLE_CPP__) || defined(__MACH__) || defined(__DARWIN))
#define MSVC 0
#define LINUX 0
#define APPLE 1
#else
#define MSVC 0
#define LINUX 0
#define APPLE 0
#endif

// shorter and succinct macros
#if __cplusplus == 202302L
#define CPP 23
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++23")
#endif
#elif __cplusplus == 202002L
#define CPP 20
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++20")
#endif
#elif __cplusplus == 201703L
#define CPP 17
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++17")
#endif
#elif __cplusplus == 201402L
#define CPP 14
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++14")
#endif
#elif __cplusplus == 201103L
#define CPP 11
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++11")
#endif
#else
#define CPP 0
#ifdef __VMAWARE_DEBUG__
#pragma message("using pre C++11 :(")
#endif
#endif

#if (CPP < 11 && !MSVC)
#error "VMAware only supports C++11 or above, set your compiler flag to '-std=c++20' for gcc/clang, or '/std:c++20' for MSVC"
#endif

#if (defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64))
#define x86 1
#else
#define x86 0
#endif
#if (defined(_M_IX86))
#define x86_32 1
#else
#define x86_32 0
#endif
#if (defined(__arm__) || defined(__ARM_LINUX_COMPILER__) || defined(__aarch64__) || defined(_M_ARM64))
#define ARM 1
#else
#define ARM 0
#endif

#if defined(__clang__)
#define GCC 0
#define CLANG 1
#elif defined(__GNUC__)
#define GCC 1
#define CLANG 0
#else
#define GCC 0
#define CLANG 0
#endif

#if !(defined(MSVC) || defined(LINUX) || defined(APPLE))
#warning "Unknown OS detected, tests will be severely limited"
#endif

#if (CPP >= 20)
#include <bit>
#include <ranges>
#include <source_location>
#endif
#if (CPP >= 17)
#include <filesystem>
#endif
#ifdef __VMAWARE_DEBUG__
#include <iomanip>
#include <ios>
#endif

#if (MSVC)
#pragma warning(push, 0) // disable the windows SDK errors temporarily
#endif

#include <functional>
#include <cstring>
#include <string>
#include <fstream>
#include <regex>
#include <thread>
#include <limits>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <cmath>
#include <sstream>
#include <bitset>
#include <type_traits>

#if (MSVC)
#include <windows.h>
#include <intrin.h>
#include <tchar.h>
#include <stdbool.h>
#include <stdio.h>
#include <Iphlpapi.h>
#include <Assert.h>
#include <excpt.h>
#include <winternl.h>
#include <winnetwk.h>
#include <winuser.h>
#include <versionhelpers.h>
#include <psapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <shlwapi.h>
#include <shlobj_core.h>
#include <strmif.h>
#include <dshow.h>
#include <stdio.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "MPR")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "ntdll.lib")

#ifdef _UNICODE
#define tregex std::wregex
#else
#define tregex std::regex
#endif

#elif (LINUX)
#if (x86)
#include <cpuid.h>
#include <x86intrin.h>
#include <immintrin.h>
#endif
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <net/if.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <memory>
#include <cctype>
#elif (APPLE)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <chrono>
#endif

#if (!MSVC)
#define TCHAR char
#endif

#if (MSVC)
#pragma warning(pop) // enable all warnings
#endif

// macro shortcut to disable MSVC warnings
#if (MSVC)
#define MSVC_DISABLE_WARNING(...) __pragma(warning(disable : __VA_ARGS__))
#define MSVC_ENABLE_WARNING(...) __pragma(warning(default : __VA_ARGS__))
#else
#define MSVC_DISABLE_WARNING(...)
#define MSVC_ENABLE_WARNING(...)
#endif

// MSVC-specific errors
#define SPECTRE 5045
#define ASSIGNMENT_OPERATOR 4626
#define NO_INLINE_FUNC 4514
#define PADDING 4820
#define FS_HANDLE 4733

MSVC_DISABLE_WARNING(ASSIGNMENT_OPERATOR NO_INLINE_FUNC SPECTRE)

#ifdef __VMAWARE_DEBUG__
#define debug(...) VM::util::debug_msg(__VA_ARGS__)
#else
#define debug(...)
#endif

struct VM {
private:
    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    static constexpr auto enum_line_start = __LINE__; // hacky way to fetch enum size
public:
    enum enum_flags : u8 {
        VMID = 1,
        BRAND,
        HYPERVISOR_BIT,
        CPUID_0X4,
        HYPERVISOR_STR,
        RDTSC,
        THREADCOUNT,
        MAC,
        TEMPERATURE,
        SYSTEMD,
        CVENDOR,
        CTYPE,
        DOCKERENV,
        DMIDECODE,
        DMESG,
        HWMON,
        SIDT5,
        CURSOR,
        VMWARE_REG,
        VBOX_REG,
        USER,
        DLL,
        REGISTRY,
        CWSANDBOX_VM,
        VM_FILES,
        HWMODEL,
        DISK_SIZE,
        VBOX_DEFAULT,
        VBOX_NETWORK,
        VM_PROCESSES,
        LINUX_USER_HOST,
        GAMARUE,
        VMID_0X4,
        PARALLELS_VM,
        RDTSC_VMEXIT,
        QEMU_BRAND,
        BOCHS_CPU,
        VPC_BOARD,
        HYPERV_WMI,
        HYPERV_REG,
        BIOS_SERIAL,
        VBOX_FOLDERS,
        MSSMBIOS,
        MAC_MEMSIZE,
        MAC_IOKIT,
        IOREG_GREP,
        MAC_SIP,
        HKLM_REGISTRIES,
        QEMU_GA,
        VALID_MSR,
        QEMU_PROC,
        VPC_PROC,
        VPC_INVALID,
        SIDT,
        SGDT,
        SLDT,
        OFFSEC_SIDT,
        OFFSEC_SGDT,
        OFFSEC_SLDT,
        HYPERV_BOARD,
        VM_FILES_EXTRA,
        VPC_SIDT,
        VMWARE_IOMEM,
        VMWARE_IOPORTS,
        VMWARE_SCSI,
        VMWARE_DMESG,
        VMWARE_STR,
        VMWARE_BACKDOOR,
        VMWARE_PORT_MEM,
        SMSW,
        MUTEX,
        UPTIME,
        ODD_CPU_THREADS,
        INTEL_THREAD_MISMATCH,
        XEON_THREAD_MISMATCH,
        EXTREME,
        NO_MEMO,
        WIN_HYPERV_DEFAULT,
        MULTIPLE
    };

private:
    static constexpr u8 enum_size = __LINE__ - enum_line_start - 4; // get enum size
    static constexpr u8 technique_count = enum_size - 4; // get total number of techniques

    // for the bitset
    using flagset = std::bitset<enum_size>;

#if (MSVC)
    using brand_score_t = i32;
#else
    using brand_score_t = u8;
#endif

public:
    // this will allow the enum to be used in the public interface as "VM::TECHNIQUE"
    enum enum_flags tmp_ignore_this = NO_MEMO;

    // constructor shit ignore this
    VM() = delete;
    VM(const VM&) = delete;
    VM(VM&&) = delete;

    static flagset DEFAULT; // default bitset that will be run if no parameters are specified
    static flagset ALL; // same as default, but with cursor check included

private:

    // global values
    static flagset flags; // global flags
    static bool cpuid_supported; // cpuid check value

    // manage the flags 
    static flagset flag_manager(flagset p_flags) {
        if (p_flags != DEFAULT) {
            if (
                (p_flags.test(EXTREME)) || \
                (p_flags.test(NO_MEMO))
            ) {
                p_flags = DEFAULT;
                if (p_flags.test(EXTREME)) {
                    p_flags.set(EXTREME);
                }

                if (p_flags.test(NO_MEMO)) {
                    p_flags.set(NO_MEMO);
                }
            }
        }

        return p_flags;
    }

    /**
    * Official aliases for VM brands. This is added to avoid accidental typos
    * which could really fuck up the result. Also, no errors/warnings are
    * issued if the string is invalid in case of a typo. For example:
    * scoreboard[VBOX]++;
    * is much better and safer against typos than:
    * scoreboard["VirtualBox"]++;
    * Hopefully this makes sense.
    *
    * TL;DR I have wonky fingers :(
    */
    static constexpr const char* VBOX = "VirtualBox";
    static constexpr const char* VMWARE = "VMware";
    static constexpr const char* VMWARE_EXPRESS = "VMware Express";
    static constexpr const char* VMWARE_ESX = "VMware ESX";
    static constexpr const char* VMWARE_GSX = "VMware GSX";
    static constexpr const char* VMWARE_WORKSTATION = "VMware Workstation";
    static constexpr const char* KVM = "KVM";
    static constexpr const char* BHYVE = "bhyve";
    static constexpr const char* QEMU = "QEMU";
    static constexpr const char* HYPERV = "Microsoft Hyper-V";
    static constexpr const char* MSXTA = "Microsoft x86-to-ARM";
    static constexpr const char* PARALLELS = "Parallels";
    static constexpr const char* XEN = "Xen HVM";
    static constexpr const char* ACRN = "ACRN";
    static constexpr const char* QNX = "QNX hypervisor";
    static constexpr const char* HYBRID = "Hybrid Analysis";
    static constexpr const char* SANDBOXIE = "Sandboxie";
    static constexpr const char* DOCKER = "Docker";
    static constexpr const char* WINE = "Wine";
    static constexpr const char* VAPPLE = "Virtual Apple";
    static constexpr const char* VPC = "Virtual PC";
    static constexpr const char* ANUBIS = "Anubis";
    static constexpr const char* JOEBOX = "JoeBox";
    static constexpr const char* THREADEXPERT = "Thread Expert";
    static constexpr const char* CWSANDBOX = "CWSandbox";
    static constexpr const char* COMODO = "Comodo";
    static constexpr const char* BOCHS = "Bochs";
    static constexpr const char* KVM_HYPERV = "KVM Hyper-V Enlightenment";
    static constexpr const char* NVMM = "NVMM";
    static constexpr const char* BSD_VMM = "OpenBSD VMM";
    static constexpr const char* INTEL_HAXM = "Intel HAXM";
    static constexpr const char* UNISYS = "Unisys s-Par";
    static constexpr const char* LMHS = "Lockheed Martin LMHS"; // yes, you read that right. The library can now detect VMs running on US military fighter jets, apparently.


    // macro for bypassing unused parameter/variable warnings
#define UNUSED(x) ((void)(x))

// likely and unlikely macros
#if (LINUX)
#define VMAWARE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define VMAWARE_LIKELY(x)   __builtin_expect(!!(x), 1)
#else
#define VMAWARE_UNLIKELY
#define VMAWARE_LIKELY
#endif

    // various cpu operation stuff
    struct cpu {
        // cpuid leaf values
        struct leaf {
            static constexpr u32
                func_ext = 0x80000000,
                proc_ext = 0x80000001,
                brand1 = 0x80000002,
                brand2 = 0x80000003,
                brand3 = 0x80000004,
                hypervisor = 0x40000000,
                amd_easter_egg = 0x8fffffff;
        };

        // cross-platform wrapper function for linux and MSVC cpuid
        static void cpuid
        (
            u32& a, u32& b, u32& c, u32& d,
            const u32 a_leaf,
            const u32 c_leaf = 0xFF  // dummy value if not set manually
        ) {
#if (x86)
            // may be unmodified for older 32-bit processors, clearing just in case
            b = 0;
            c = 0;
    #if (MSVC)
            int32_t x[4]{};
            __cpuidex((int32_t*)x, static_cast<int>(a_leaf), static_cast<int>(c_leaf));
            a = static_cast<u32>(x[0]);
            b = static_cast<u32>(x[1]);
            c = static_cast<u32>(x[2]);
            d = static_cast<u32>(x[3]);
    #elif (LINUX)
            __cpuid_count(a_leaf, c_leaf, a, b, c, d);
    #endif
#else
            return;
#endif
        };

        // same as above but for array type parameters (MSVC specific)
        static void cpuid
        (
            int32_t x[4],
            const u32 a_leaf,
            const u32 c_leaf = 0xFF
        ) {
#if (x86)
            // may be unmodified for older 32-bit processors, clearing just in case
            x[1] = 0;
            x[2] = 0;
    #if (MSVC)
            __cpuidex((int32_t*)x, static_cast<int>(a_leaf), static_cast<int>(c_leaf));
    #elif (LINUX)
            __cpuid_count(a_leaf, c_leaf, x[0], x[1], x[2], x[3]);
    #endif
#else
            return;
#endif
        };

        // check for maximum function leaf
        static bool is_leaf_supported(const u32 p_leaf) {
            u32 eax, unused = 0;
            cpu::cpuid(eax, unused, unused, unused, cpu::leaf::func_ext);

            debug("CPUID function: highest leaf = ", eax);

            return (p_leaf <= eax);
        }

        // check AMD
        [[nodiscard]] static bool is_amd() {
            constexpr u32 amd_ecx = 0x69746e65;

            u32 unused, ecx = 0;
            cpuid(unused, unused, ecx, unused, 0);

            return (ecx == amd_ecx);
        }

        // check Intel
        [[nodiscard]] static bool is_intel() {
            constexpr u32 intel_ecx = 0x6c65746e;

            u32 unused, ecx = 0;
            cpuid(unused, unused, ecx, unused, 0);

            return (ecx == intel_ecx);
        }

        [[nodiscard]] static bool has_hyperthreading() {
            u32 eax, ebx, ecx, edx;

            cpuid(eax, ebx, ecx, edx, 1);

            bool htt_available = (edx & (1 << 28));

            if (!htt_available) {
                return false;
            }

            i32 logical_cores = ((ebx >> 16) & 0xFF);
            i32 physical_cores = 0;

#if (MSVC)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            physical_cores = sysinfo.dwNumberOfProcessors;
#elif (LINUX)
            physical_cores = static_cast<i32>(sysconf(_SC_NPROCESSORS_CONF));
#elif (APPLE)
            sysctlbyname("hw.physicalcpu", &physical_cores, sizeof(physical_cores), NULL, 0);
#else
            return false;
#endif
    
            return (logical_cores > physical_cores);
        }

        // get the CPU product
        [[nodiscard]] static std::string get_brand() {
            if (memo::is_cpu_brand_cached()) {
                return memo::fetch_cpu_brand();
            }

            if (!cpuid_supported) {
                return "Unknown";
            }

#if (!x86)
            return "Unknown";
#else
            if (!cpu::is_leaf_supported(cpu::leaf::brand3)) {
                return "Unknown";
            }

            std::array<u32, 4> buffer{};
            constexpr std::size_t buffer_size = sizeof(int32_t) * buffer.size();
            std::array<char, 64> charbuffer{};

            constexpr std::array<u32, 3> ids = { {
                cpu::leaf::brand1,
                cpu::leaf::brand2,
                cpu::leaf::brand3
            } };

            std::string brand = "";

            for (const u32& id : ids) {
                cpu::cpuid(buffer.at(0), buffer.at(1), buffer.at(2), buffer.at(3), id);

                std::memcpy(charbuffer.data(), buffer.data(), buffer_size);

                const char* convert = charbuffer.data();
                brand += convert;
            }

            debug("BRAND: ", "cpu brand = ", brand);

            memo::store_cpu_brand(brand);

            return brand;
#endif
        }

        struct model_struct {
            bool found;
            bool is_xeon;
            bool is_i_series;
            std::string string;
        };

        [[nodiscard]] static model_struct get_model() {
            const std::string brand = get_brand();

            constexpr const char* intel_i_series_regex    = "i[0-9]-[A-Z0-9]{1,7}";
            constexpr const char* intel_xeon_series_regex = "[DEW]-[A-Z0-9]{1,7}";

            std::string match_str = "";

            auto match = [&](const char* regex) -> bool {
                std::regex pattern(regex);

                auto words_begin  = std::sregex_iterator(brand.begin(), brand.end(), pattern);
                auto words_end    = std::sregex_iterator();

                for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                    std::smatch match = *i;
                    match_str = match.str();
                }

                if (!match_str.empty()) {
                    return true;
                }

                return false;
            };

            bool found        = false;
            bool is_xeon      = false;
            bool is_i_series  = false;
            std::string string = "";

            if (cpu::is_intel()) {
                if (match(intel_i_series_regex)) {
                    found       = true;
                    is_i_series = true;
                    string       = match_str;
                } else if (match(intel_xeon_series_regex)) {
                    found   = true;
                    is_xeon = true;
                    string   = match_str;
                }
            }

            // no AMD (for now)

            return model_struct{ found, is_xeon, is_i_series, string };
        };

#if (CPP >= 17)
        [[nodiscard]] static bool vmid_template(const u32 p_leaf, [[maybe_unused]] const char* technique_name) {
#else 
        [[nodiscard]] static bool vmid_template(const u32 p_leaf, const char* technique_name) {
#endif
#if (CPP >= 17)
            constexpr std::string_view
#else
            const std::string
#endif
                bhyve = "bhyve bhyve ",
                bhyve2 = "BHyVE BHyVE ",
                kvm = "KVMKVMKVM\0\0\0",
                kvm_hyperv = "Linux KVM Hv",
                qemu = "TCGTCGTCGTCG",
                hyperv = "Microsoft Hv",
                xta = "MicrosoftXTA",
                parallels = " prl hyperv ",
                parallels2 = " lrpepyh  vr",
                vmware = "VMwareVMware",
                vbox = "VBoxVBoxVBox",
                xen = "XenVMMXenVMM",
                acrn = "ACRNACRNACRN",
                qnx = " QNXQVMBSQG ",
                qnx2 = "QXNQSBMV",
                nvmm = "___ NVMM ___",
                openbsd_vmm = "OpenBSDVMM58",
                intel_haxm = "HAXMHAXMHAXM",
                virtapple = "VirtualApple",
                unisys = "UnisysSpar64",
                lmhs = "SRESRESRESRE";

            auto cpuid_thingy = [](const u32 p_leaf, u32* regs, std::size_t start = 0, std::size_t end = 4) -> bool {
                u32 x[4]{};
                cpu::cpuid(x[0], x[1], x[2], x[3], p_leaf);

                for (; start < end; start++) {
                    *regs++ = x[start];
                }

                return true;
            };

            std::string brand = "";
            u32 sig_reg[3] = {0};

            if (!cpuid_thingy(p_leaf, sig_reg, 1)) {
                return false;
            }

            auto strconvert = [](u64 n) -> std::string {
                const std::string& str(reinterpret_cast<char*>(&n));
                return str;
            };

            std::stringstream ss1;
            std::stringstream ss2;

            ss1 << strconvert(sig_reg[0]);
            ss1 << strconvert(sig_reg[1]);
            ss1 << strconvert(sig_reg[2]);

            ss2 << strconvert(sig_reg[0]);
            ss2 << strconvert(sig_reg[2]);
            ss2 << strconvert(sig_reg[1]);

            std::string brand1 = ss1.str();
            std::string brand2 = ss2.str();

            debug(technique_name, brand1);
            debug(technique_name, brand2);

#if (CPP < 17)
            // bypass compiler warning about unused parameter, ignore this
            UNUSED(technique_name);
#endif

            const std::vector<std::string> brand_streams = { brand1, brand2 };

            for (const auto &tmp_brand : brand_streams) {
                if (tmp_brand == qemu)        { return core::add(QEMU); }
                if (tmp_brand == vmware)      { return core::add(VMWARE); }
                if (tmp_brand == vbox)        { return core::add(VBOX); }
                if (tmp_brand == bhyve)       { return core::add(BHYVE); }
                if (tmp_brand == bhyve2)      { return core::add(BHYVE); }
                if (tmp_brand == kvm)         { return core::add(KVM); }
                if (tmp_brand == kvm_hyperv)  { return core::add(KVM_HYPERV); }
                if (tmp_brand == xta)         { return core::add(MSXTA); }
                if (tmp_brand == parallels)   { return core::add(PARALLELS); }
                if (tmp_brand == parallels2)  { return core::add(PARALLELS); }
                if (tmp_brand == xen)         { return core::add(XEN); }
                if (tmp_brand == acrn)        { return core::add(ACRN); }
                if (tmp_brand == qnx)         { return core::add(QNX); }
                if (tmp_brand == virtapple)   { return core::add(VAPPLE); }
                if (tmp_brand == nvmm)        { return core::add(NVMM); }
                if (tmp_brand == openbsd_vmm) { return core::add(BSD_VMM); }
                if (tmp_brand == intel_haxm)  { return core::add(INTEL_HAXM); }
                if (tmp_brand == unisys)      { return core::add(UNISYS); }
                if (tmp_brand == lmhs)        { return core::add(LMHS); }


                // both Hyper-V and VirtualPC have the same string value
                if (tmp_brand == hyperv)     { 
                    bool tmp = core::add(VPC);
                    UNUSED(tmp);
                    return core::add(HYPERV);
                }

                /**
                 * this is added because there are inconsistent string
                 * values for KVM's manufacturer ID. For example,
                 * it gives me "KVMKMVMKV" when I run it under QEMU
                 * but the Wikipedia article on CPUID says it's
                 * "KVMKVMKVM\0\0\0", like wtf????
                 */
                if (util::find(tmp_brand, "KVM")) {
                    return core::add(KVM);
                }

                /** 
                 * i'm honestly not sure about this one, 
                 * it's supposed to have 12 characters but
                 * Wikipedia tells me it has 8, so i'm just
                 * going to scan for the entire string ig
                 */ 
                #if (CPP >= 17)
                const char* qnx_sample = qnx2.data();
                #else
                const char* qnx_sample = qnx2.c_str();
                #endif

                if (util::find(tmp_brand, qnx_sample)) {
                    return core::add(QNX);
                }
            }

            return false;
        }
    };

    // memoization
    struct memo {
    private:
        using result_t = bool;
        using points_t = u8;

    public:
        struct data_t {
            result_t result;
            points_t points;
        };

    private:
        static std::map<u8, data_t> cache_table;
        static flagset cache_keys;
        static std::string brand_cache;
        static std::string multibrand_cache;
        static std::string cpu_brand_cache;

    public:
        static void cache_store(const u8 technique_macro, const result_t result, const points_t points) {
            cache_table[technique_macro] = { result, points };
            cache_keys.set(technique_macro);
        }

        static bool is_cached(const u8 technique_macro) {
            return cache_keys.test(technique_macro);
        }

        static data_t cache_fetch(const u8 technique_macro) {
            return cache_table.at(technique_macro);
        }

        static std::string fetch_brand() {
            return brand_cache;
        }

        static void store_brand(const std::string &p_brand) {
            brand_cache = p_brand;
        }

        static bool is_brand_cached() {
            return (!brand_cache.empty());
        }

        static std::string fetch_multibrand() {
            return multibrand_cache;
        }

        static void store_multibrand(const std::string &p_brand) {
            multibrand_cache = p_brand;
        }

        static bool is_multibrand_cached() {
            return (!multibrand_cache.empty());
        }
    
        static std::string fetch_cpu_brand() {
            return cpu_brand_cache;
        }

        static void store_cpu_brand(const std::string &p_brand) {
            cpu_brand_cache = p_brand;
        }

        static bool is_cpu_brand_cached() {
            return (!cpu_brand_cache.empty());
        }

        // basically checks whether all the techniques were cached
        static bool all_present() {
            return (cache_table.size() == technique_count);
        }
    };

    // miscellaneous functionalities
    struct util {
#if (LINUX)
        // fetch file data
        [[nodiscard]] static std::string read_file(const char* dir) {
            if (!exists(dir)) {
                return "";
            }

            std::ifstream file{};
            std::string data{};
            file.open(dir);

            if (file.is_open()) {
                file >> data;
            }

            file.close();
            return data;
        }
#endif

        [[nodiscard]] static bool exists(const char* path) {
#if (MSVC)
            return (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) || (GetLastError() != ERROR_FILE_NOT_FOUND);
#else 
    #if (CPP >= 17)
            return std::filesystem::exists(path);
    #elif (CPP >= 11)
            struct stat buffer;
            return (stat(path, &buffer) == 0);
    #endif
#endif
        }

#if (MSVC) && (_UNICODE)
        // handle TCHAR conversion
        [[nodiscard]] static bool exists(const TCHAR* path)
		{
			char c_szText[_MAX_PATH];
			wcstombs(c_szText, path, wcslen(path) + 1);
			return exists(c_szText);
		}
#endif

        // self-explanatory
        [[nodiscard]] static bool is_admin() noexcept {
#if (LINUX || APPLE)
            const uid_t uid = getuid();
            const uid_t euid = geteuid();

            return (
                (uid != euid) ||
                (euid == 0)
            );
#elif (MSVC)
            BOOL is_admin = FALSE;
            HANDLE hToken = NULL;
            if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
                DWORD dwSize = 0;
                if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    PTOKEN_MANDATORY_LABEL pTIL = (PTOKEN_MANDATORY_LABEL)malloc(dwSize);
                    if (pTIL != NULL) {
                        if (GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize, &dwSize)) {
                            SID* pSID = (SID*)GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));
                            DWORD dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));

                            if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID) {
                                is_admin = TRUE;
                            }

                            UNUSED(pSID);
                        }
                        free(pTIL);
                    }
                }
            }

            if (hToken) {
                CloseHandle(hToken);
            }

            return is_admin;
#endif
        }

        // scan for keyword in string
        [[nodiscard]] static bool find(const std::string& base_str, const char* keyword) noexcept {
            return (base_str.find(keyword) != std::string::npos);
        };

        // for debug output
#ifdef __VMAWARE_DEBUG__
        template <typename... Args>
        static inline void debug_msg(Args... message) noexcept {
#if (LINUX || APPLE)
            constexpr const char* black_bg = "\x1B[48;2;0;0;0m";
            constexpr const char* bold = "\033[1m";
            constexpr const char* blue = "\x1B[38;2;00;59;193m";
            constexpr const char* ansiexit = "\x1B[0m";

            std::cout.setf(std::ios::fixed, std::ios::floatfield);
            std::cout.setf(std::ios::showpoint);

            std::cout << black_bg << bold << "[" << blue << "DEBUG" << ansiexit << bold << black_bg << "]" << ansiexit << " ";
#else       
            std::cout << "[DEBUG] ";
#endif
            ((std::cout << message), ...);
            std::cout << "\n";
        }
#endif

        // basically std::system but it runs in the background with std::string output
        [[nodiscard]] static std::unique_ptr<std::string> sys_result(const TCHAR* cmd) try {
#if (CPP < 14)
            std::unique_ptr<std::string> tmp(nullptr);
            UNUSED(cmd);
            return tmp;
#else
    #if (LINUX || APPLE)
        #if (ARM)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wignored-attributes"
        #endif
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

        #if (ARM)
            #pragma GCC diagnostic pop
        #endif

            if (!pipe) {
                return nullptr;
            }

            std::string result{};
            std::array<char, 128> buffer{};

            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }

            result.pop_back();

            return std::make_unique<std::string>(result);
    #elif (MSVC)
            // Set up the structures for creating the process
            STARTUPINFO si = { 0 };
            PROCESS_INFORMATION pi = { 0 };
            si.cb = sizeof(si);

            // Create a pipe to capture the command output
            HANDLE hReadPipe, hWritePipe;
            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = NULL;

            if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
                debug("sys_result: ", "error creating pipe");

                return nullptr;
            }

            // Set up the startup information with the write end of the pipe as the standard output
            si.hStdError = hWritePipe;
            si.hStdOutput = hWritePipe;
            si.dwFlags |= STARTF_USESTDHANDLES;

            // Create the process
            if (!CreateProcess(NULL, const_cast<TCHAR*>(cmd), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                debug("sys_result: ", "error creating process");

                CloseHandle(hReadPipe);
                CloseHandle(hWritePipe);
                return nullptr;
            }

            // Close the write end of the pipe as it's not needed in this process
            CloseHandle(hWritePipe);

            // Read the output from the pipe
            char buffer[4096];
            DWORD bytesRead;
            std::string result;

            while (ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
                result.append(buffer, bytesRead);
            }

            // Close handles
            CloseHandle(hReadPipe);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            // Return the result as a unique_ptr<string>
            return std::make_unique<std::string>(result);
    #endif
#endif
        }
        catch (...) {
            debug("sys_result: ", "catched error, returning nullptr");
            std::unique_ptr<std::string> tmp(nullptr);
            return tmp;
        }

        // get disk size in GB
        [[nodiscard]] static u32 get_disk_size() {
            u32 size = 0;
            constexpr u64 GB = (static_cast<VM::u64>(1024 * 1024) * 1024);

#if (LINUX)
            struct statvfs stat;

            if (statvfs("/", &stat) != 0) {
                debug("private util::get_disk_size( function: ", "failed to fetch disk size");
                return false;
            }

            // in gigabytes
            size = static_cast<u32>((stat.f_blocks * stat.f_frsize) / GB);
#elif (MSVC)
            ULARGE_INTEGER totalNumberOfBytes;

            if (GetDiskFreeSpaceExW(
                L"C:",                      // Drive or directory path (use wide character string)
                nullptr,                    // Free bytes available to the caller (not needed for total size)
                reinterpret_cast<PULARGE_INTEGER>(&totalNumberOfBytes),  // Total number of bytes on the disk
                nullptr                     // Total number of free bytes on the disk (not needed for total size)
            )) {
                size = static_cast<u32>(totalNumberOfBytes.QuadPart) / GB;
            }

            else {
                debug("util::get_disk_size(: ", "failed to fetch size in GB");
            }
#endif

            if (size == 0) {
                return false;
            }

            // round to the nearest factor of 10
            const u32 result = static_cast<u32>(std::round((size / 10.0) * 10));

            debug("private util::get_disk_size( function: ", "disk size = ", result, "GB");

            return result;
        }

        // get physical RAM size in GB
        [[nodiscard]] static u64 get_physical_ram_size() {
#if (LINUX)
            if (!util::is_admin()) {
                debug("private get_physical_ram_size function: ", "not root, returned 0");
                return 0;
            }

            auto result = util::sys_result("dmidecode --type 19 | grep 'Size' | grep '[[:digit:]]*'");

            if (result == nullptr) {
                debug("private get_physical_ram_size function: ", "invalid system result from dmidecode, returned 0");
                return 0;
            }

            const bool MB = (std::regex_search(*result, std::regex("MB")));
            const bool GB = (std::regex_search(*result, std::regex("GB")));

            if (!(MB || GB)) {
                debug("private get_physical_ram_size function: ", "neither MB nor GB found, returned 0");
                return 0;
            }

            std::string number_str;
            bool in_number = false;

            for (char c : *result) {
                if (std::isdigit(c)) {
                    number_str += c;
                    in_number = true;
                }
                else if (in_number) {
                    break;
                }
            }

            if (number_str.empty()) {
                debug("private get_physical_ram_size_gb function: ", "string is empty, returned 0");
                return 0;
            }

            u64 number = 0;

            number = std::stoull(number_str);

            if (MB == true) {
                number = static_cast<u64>(std::round(number / 1024));
            }

            return number; // in GB
#elif (MSVC)
            if (!IsWindowsVistaOrGreater()) {
                return 0;
            }

            ULONGLONG total_memory_kb = 0;

            if (GetPhysicallyInstalledSystemMemory(&total_memory_kb) == ERROR_INVALID_DATA) {
                return 0;
            }

            return (total_memory_kb / (static_cast<unsigned long long>(1024) * 1024)); // MB
#else
            return 0;
#endif
        }

        // get available memory space
        [[nodiscard]] static u64 get_memory_space() {
#if (MSVC)
            MEMORYSTATUSEX statex = { 0 };
            statex.dwLength = sizeof(statex);
            GlobalMemoryStatusEx(&statex); // calls NtQuerySystemInformation
            return statex.ullTotalPhys;
#elif (LINUX)
            const i64 pages = sysconf(_SC_PHYS_PAGES);
            const i64 page_size = sysconf(_SC_PAGE_SIZE);
            return (pages * page_size);
#elif (APPLE)
            int32_t mib[2] = { CTL_HW, HW_MEMSIZE };
            u32 namelen = sizeof(mib) / sizeof(mib[0]);
            u64 size = 0;
            std::size_t len = sizeof(size);

            if (sysctl(mib, namelen, &size, &len, NULL, 0) < 0) {
                return 0;
            }

            return size; // in bytes
#endif
        }


        [[nodiscard]] static bool is_proc_running(const TCHAR* executable) {
#if (MSVC)
            DWORD processes[1024], bytesReturned;

            if (!EnumProcesses(processes, sizeof(processes), &bytesReturned))
                return false;

            DWORD numProcesses = bytesReturned / sizeof(DWORD);

            for (DWORD i = 0; i < numProcesses; ++i) {
                HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                if (process != nullptr) {
                    TCHAR processName[MAX_PATH];
                    if (GetModuleBaseName(process, nullptr, processName, sizeof(processName) / sizeof(TCHAR))) {
                        if (!_tcsicmp(processName, executable)) {
                            CloseHandle(process);
                            return true;
                        }
                    }
                    CloseHandle(process);
                }
            }

            return false;
#elif (LINUX)
#if (CPP >= 17)
            for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
                if (!(entry.is_directory())) {
                    continue;
                }

                const std::string filename = entry.path().filename().string();
#else
            DIR* dir = opendir("/proc");
            if (!dir) {
                debug("util::is_proc_running: ", "failed to open /proc directory");
                return false;
            }

            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string filename(entry->d_name);
                if (filename == "." || filename == "..") {
                    continue;
                }
#endif
                if (!(std::all_of(filename.begin(), filename.end(), ::isdigit))) {
                    continue;
                }

                const std::string cmdline_file = "/proc/" + filename + "/cmdline";
                std::ifstream cmdline(cmdline_file);
                if (!(cmdline.is_open())) {
                    continue;
                }

                std::string line;
                std::getline(cmdline, line);
                cmdline.close();

                if (
                    !line.empty() && \
                    line.find(executable) != std::string::npos
                ) {
                    const std::size_t slash_index = line.find_last_of('/');

                    if (slash_index == std::string::npos) {
                        continue;
                    }

                    line = line.substr(slash_index + 1);

                    const std::size_t space_index = line.find_first_of(' ');

                    if (space_index != std::string::npos) {
                        line = line.substr(0, space_index);
                    }

                    if (line != executable) {
                        continue;
                    }
#if (CPP < 17)
                    closedir(dir);
#endif
                    return true;
                }
            }

            return false;
#else
            return false;
#endif
            }

#if (MSVC)
        /**
         * @link: https://codereview.stackexchange.com/questions/249034/systeminfo-a-c-class-to-retrieve-system-management-data-from-the-bios
         * @author: arcomber
         */
        class sys_info {
        private:
#pragma pack(push) 
#pragma pack(1)
            /*
            SMBIOS Structure header (System Management BIOS) spec:
            https ://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
            */
            struct SMBIOSHEADER
            {
                uint8_t type;
                uint8_t length;
                uint16_t handle;
            };

            /*
            Structure needed to get the SMBIOS table using GetSystemFirmwareTable API.
            see https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
            */
            struct SMBIOSData {
                uint8_t  Used20CallingMethod;
                uint8_t  SMBIOSMajorVersion;
                uint8_t  SMBIOSMinorVersion;
                uint8_t  DmiRevision;
                uint32_t  Length;
                uint8_t  SMBIOSTableData[1];
            };

            // System Information (Type 1)
            struct SYSTEMINFORMATION {
                SMBIOSHEADER Header;
                uint8_t Manufacturer;
                uint8_t ProductName;
                uint8_t Version;
                uint8_t SerialNumber;
                uint8_t UUID[16];
                uint8_t WakeUpType;  // Identifies the event that caused the system to power up
                uint8_t SKUNumber;   // identifies a particular computer configuration for sale
                uint8_t Family;
            };
#pragma pack(pop) 

            // helper to retrieve string at string offset. Optional null string description can be set.
            const char* get_string_by_index(const char* str, int index, const char* null_string_text = "")
            {
                if (0 == index || 0 == *str) {
                    return null_string_text;
                }

                while (--index) {
                    str += strlen(str) + 1;
                }
                return str;
            }

            // retrieve the BIOS data block from the system
            SMBIOSData* get_bios_data() {
                SMBIOSData* bios_data = nullptr;

                // GetSystemFirmwareTable with arg RSMB retrieves raw SMBIOS firmware table
                // return value is either size of BIOS table or zero if function fails
                DWORD bios_size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);

                if (bios_size > 0) {
                    if (bios_data != nullptr) {
                        bios_data = (SMBIOSData*)malloc(bios_size);

                        // Retrieve the SMBIOS table
                        DWORD bytes_retrieved = GetSystemFirmwareTable('RSMB', 0, bios_data, bios_size);

                        if (bytes_retrieved != bios_size) {
                            free(bios_data);
                            bios_data = nullptr;
                        }
                    }
                }

                return bios_data;
            }


            // locates system information memory block in BIOS table
            SYSTEMINFORMATION* find_system_information(SMBIOSData* bios_data) {
                uint8_t* data = bios_data->SMBIOSTableData;

                while (data < bios_data->SMBIOSTableData + bios_data->Length)
                {
                    uint8_t* next;
                    SMBIOSHEADER* header = (SMBIOSHEADER*)data;

                    if (header->length < 4)
                        break;

                    //Search for System Information structure with type 0x01 (see para 7.2)
                    if (header->type == 0x01 && header->length >= 0x19)
                    {
                        return (SYSTEMINFORMATION*)header;
                    }

                    //skip over formatted area
                    next = data + header->length;

                    //skip over unformatted area of the structure (marker is 0000h)
                    while (next < bios_data->SMBIOSTableData + bios_data->Length && (next[0] != 0 || next[1] != 0)) {
                        next++;
                    }
                    next += 2;

                    data = next;
                }
                return nullptr;
            }

        public:
            // System information data retrieved on construction and string members populated
            sys_info() {
                SMBIOSData* bios_data = get_bios_data();

                if (bios_data) {
                    SYSTEMINFORMATION* sysinfo = find_system_information(bios_data);
                    if (sysinfo) {
                        const char* str = (const char*)sysinfo + sysinfo->Header.length;

                        manufacturer_ = get_string_by_index(str, sysinfo->Manufacturer);
                        productname_ = get_string_by_index(str, sysinfo->ProductName);
                        serialnumber_ = get_string_by_index(str, sysinfo->SerialNumber);
                        version_ = get_string_by_index(str, sysinfo->Version);

                        // for v2.1 and later
                        if (sysinfo->Header.length > 0x08)
                        {
                            static const int max_uuid_size{ 50 };
                            char uuid[max_uuid_size] = {};
                            _snprintf_s(uuid, max_uuid_size, static_cast<size_t>(max_uuid_size) - 1, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                                sysinfo->UUID[0], sysinfo->UUID[1], sysinfo->UUID[2], sysinfo->UUID[3],
                                sysinfo->UUID[4], sysinfo->UUID[5], sysinfo->UUID[6], sysinfo->UUID[7],
                                sysinfo->UUID[8], sysinfo->UUID[9], sysinfo->UUID[10], sysinfo->UUID[11],
                                sysinfo->UUID[12], sysinfo->UUID[13], sysinfo->UUID[14], sysinfo->UUID[15]);

                            uuid_ = uuid;
                        }

                        if (sysinfo->Header.length > 0x19)
                        {
                            // supported in v 2.4 spec
                            sku_ = get_string_by_index(str, sysinfo->SKUNumber);
                            family_ = get_string_by_index(str, sysinfo->Family);
                        }
                    }
                    free(bios_data);
                }
            }

            // get product family
            const std::string get_family() const {
                return family_;
            }

            // get manufacturer - generally motherboard or system assembler name
            const std::string get_manufacturer() const {
                return manufacturer_;
            }

            // get product name
            const std::string get_productname() const {
                return productname_;
            }

            // get BIOS serial number
            const std::string get_serialnumber() const {
                return serialnumber_;
            }

            // get SKU / system configuration
            const std::string get_sku() const {
                return sku_;
            }

            // get a universally unique identifier for system
            const std::string get_uuid() const {
                return uuid_;
            }

            // get version of system information
            const std::string get_version() const {
                return version_;
            }

            sys_info(sys_info const&) = delete;
            sys_info& operator=(sys_info const&) = delete;

        private:
            std::string family_;
            std::string manufacturer_;
            std::string productname_;
            std::string serialnumber_;
            std::string sku_;
            std::string uuid_;
            std::string version_;
        };

        [[nodiscard]] static bool is_wow64() {
            BOOL isWow64 = FALSE;
            BOOL tmp = IsWow64Process(GetCurrentProcess(), &isWow64);
            return (tmp && isWow64);
        }

        [[nodiscard]] static u8 get_windows_version() {
            u8 ret = 0;
            NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW) = nullptr;
            OSVERSIONINFOEXW osInfo{};

            HMODULE ntdllModule = GetModuleHandleA("ntdll");

            if (ntdllModule == nullptr) {
                return false;
            }

            *(FARPROC*)&RtlGetVersion = GetProcAddress(ntdllModule, "RtlGetVersion");

            if (RtlGetVersion == nullptr) {
                return false;
            }

            if (RtlGetVersion != nullptr) {
                osInfo.dwOSVersionInfoSize = sizeof(osInfo);
                RtlGetVersion(&osInfo);
                ret = static_cast<u8>(osInfo.dwMajorVersion);
            }

            return ret;
        }
#endif
    };

private: // START OF PRIVATE VM DETECTION TECHNIQUE DEFINITIONS
    /**
     * @brief Check CPUID output of manufacturer ID for known VMs/hypervisors
     * @category x86
     */
    [[nodiscard]] static bool vmid() try {
#if (!x86)
        return false;
#else
        if (!cpuid_supported || core::disabled(VMID)) {
            return false;
        }

        return cpu::vmid_template(0, "VMID: ");
#endif
    }
    catch (...) {
        debug("VMID: catched error, returned false");
        return false;
    }


    /**
     * @brief Check CPUID output of manufacturer ID for known VMs/hypervisors with leaf value 0x40000000
     * @category x86
     */
    [[nodiscard]] static bool vmid_0x4() try {
#if (!x86)
        return false;
#else
        if (!cpuid_supported || core::disabled(VMID_0X4)) {
            return false;
        }

        return cpu::vmid_template(cpu::leaf::hypervisor, "VMID_0x4: ");
#endif
    }
    catch (...) {
        debug("VMID_0x4: catched error, returned false");
        return false;
    }

    /**
     * @brief Check if CPU brand is a VM brand
     * @category x86
     */
    [[nodiscard]] static bool cpu_brand() try {
#if (!x86)
        return false;
#else
        if (!cpuid_supported || core::disabled(BRAND)) {
            return false;
        }

        std::string brand = cpu::get_brand();

        // TODO: might add more potential keywords, be aware that it could (theoretically) cause false positives
        constexpr std::array<const char*, 16> vmkeywords{ {
            "qemu", "kvm", "virtual", "vm",
            "vbox", "virtualbox", "vmm", "monitor",
            "bhyve", "hyperv", "hypervisor", "hvisor",
            "parallels", "vmware", "hvm", "qnx"
        } };

        u8 match_count = 0;

        for (auto it = vmkeywords.cbegin(); it != vmkeywords.cend(); it++) {
            const auto regex = std::regex(*it, std::regex::icase);
            const bool match = std::regex_search(brand, regex);

            if (match) {
                debug("BRAND_KEYWORDS: ", "match = ", *it);
                match_count++;
            }
        }

        debug("BRAND_KEYWORDS: ", "matches: ", static_cast<u32>(match_count));

        if (match_count > 0) {
            const auto qemu_regex = std::regex("QEMU", std::regex::icase);
            const bool qemu_match = std::regex_search(brand, qemu_regex);

            if (qemu_match) {
                return core::add(QEMU);
            }
        }

        return (match_count >= 1);
#endif
    }
    catch (...) {
        debug("BRAND_KEYWORDS: catched error, returned false");
        return false;
    }


    /**
     * @brief Match for QEMU CPU brand
     * @category x86
     */
    [[nodiscard]] static bool cpu_brand_qemu() try {
#if (!x86)
        return false;
#else
        if (!cpuid_supported || core::disabled(QEMU_BRAND)) {
            return false;
        }

        std::string brand = cpu::get_brand();

        std::regex pattern("QEMU Virtual CPU", std::regex_constants::icase);

        if (std::regex_match(brand, pattern)) {
            return core::add(QEMU);
        }

        return false;
#endif
    }
    catch (...) {
        debug("QEMU_BRAND: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if hypervisor feature bit in CPUID is enabled (always false for physical CPUs)
     * @category x86
     */
    [[nodiscard]] static bool hypervisor_bit() try {
#if (!x86)
        return false;
#else
        if (!cpuid_supported || core::disabled(HYPERVISOR_BIT)) {
            return false;
        }

        constexpr u8 hyperv_bit = 31;

        u32 unused, ecx = 0;
        cpu::cpuid(unused, unused, ecx, unused, 1);

        return (ecx & (1 << hyperv_bit));
#endif
    }
    catch (...) {
        debug("HYPERVISOR_BIT: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if 0x40000000~0x400000FF cpuid input is present (mostly present in VMs, according to VMware)
     * @link https://kb.vmware.com/s/article/1009458
     * @category x86
     */
    [[nodiscard]] static bool cpuid_0x4() try {
#if (!x86)
        return false;
#else
        if (!cpuid_supported || core::disabled(CPUID_0X4)) {
            return false;
        }

        u32 a, b, c, d = 0;

        for (u8 i = 0; i < 0xFF; ++i) {
            cpu::cpuid(a, b, c, d, (cpu::leaf::hypervisor + i));
            if ((a + b + c + d) != 0) {
                return true;
            }
        }

        return false;
#endif
    }
    catch (...) {
        debug("CPUID_0x4: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for hypervisor brand string length (would be around 2 characters in a host machine)
     * @category x86
     */
    [[nodiscard]] static bool hypervisor_brand() try {
#if (!x86)
        return false;
#else
        if (core::disabled(HYPERVISOR_STR)) {
            return false;
        }

        char out[sizeof(int32_t) * 4 + 1] = { 0 }; // e*x size + number of e*x registers + null terminator
        cpu::cpuid((int*)out, cpu::leaf::hypervisor);

        debug("HYPERV_STR: eax: ", static_cast<u32>(out[0]),
            "\nebx: ", static_cast<u32>(out[1]),
            "\necx: ", static_cast<u32>(out[2]),
            "\nedx: ", static_cast<u32>(out[3])
        );

        return (std::strlen(out + 4) >= 4);
#endif
    }
    catch (...) {
        debug("HYPERVISOR_STR: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if RDTSC is slow, if yes then it might be a VM
     * @category x86
     */
    [[nodiscard]] 
#if (LINUX)
    // this is added so no sanitizers can potentially cause unwanted delays while measuring rdtsc in a debug compilation
    __attribute__((no_sanitize("address", "leak", "thread", "undefined")))
#endif
    static bool rdtsc_check() try {
#if (!x86)
        return false;
#else
        if (core::disabled(RDTSC)) {
            return false;
        }

    #if (LINUX)
        u32 a, b, c, d = 0;

        // check if rdtsc is available
        if (!__get_cpuid(cpu::leaf::proc_ext, &a, &b, &c, &d)) {
            if (!(d & (1 << 27))) {
                return false;
            }
        }

        u64 s, acc = 0;
        int32_t out[4];

        for (std::size_t i = 0; i < 100; ++i) {
            s = __rdtsc();
            cpu::cpuid(out, 0, 0);
            acc += __rdtsc() - s;
        }

        debug("RDTSC: ", "acc = ", acc);
        debug("RDTSC: ", "acc/100 = ", acc / 100);

        return (acc / 100 > 350);
    #elif (MSVC)
#define LODWORD(_qw)    ((DWORD)(_qw))
        u64 tsc1 = 0;
        u64 tsc2 = 0;
        u64 tsc3 = 0;
        for (INT i = 0; i < 10; i++) {
            tsc1 = __rdtsc();
            GetProcessHeap();  // delay
            tsc2 = __rdtsc();
#pragma warning(push)
#pragma warning(disable: 6387)
            CloseHandle(0);
#pragma warning(pop)
            tsc3 = __rdtsc();
            const bool condition = ((LODWORD(tsc3) - LODWORD(tsc2)) / (LODWORD(tsc2) - LODWORD(tsc1)) >= 10);
            if (condition) {
                return false;
            }
        }

        return true;
    #else
        return false;
    #endif
#endif
    }
    catch (...) {
        debug("RDTSC: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if the 5th byte after sidt is null
     * @author Matteo Malvica
     * @link https://www.matteomalvica.com/blog/2018/12/05/detecting-vmware-on-64-bit-systems/
     * @category x86
     */
    [[nodiscard]] static bool sidt5() try {
#if (!x86 || !LINUX || GCC)
        return false;
#else
        if (core::disabled(SIDT5)) {
            return false;
        }

        u8 values[10];
        std::memset(values, 0, 10);

        fflush(stdout);
        __asm__ __volatile__("sidt %0" : "=m"(values));

    #ifdef __VMAWARE_DEBUG__
        u32 result = 0;

        for (u8 i = 0; i < 10; i++) {
            result <<= 8;
            result |= values[i];
        }

        debug("SIDT5: ", "values = 0x", std::hex, std::setw(16), std::setfill('0'), result);
    #endif

        return (values[5] == 0x00);
#endif
    }
    catch (...) {
        debug("SIDT5: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if processor count is 1 or 2 (some VMs only have a single core)
     * @category All systems
     */
    [[nodiscard]] static bool thread_count() try {
        if (core::disabled(THREADCOUNT)) {
            return false;
        }

        debug("THREADCOUNT: ", "threads = ", std::thread::hardware_concurrency());

        return (std::thread::hardware_concurrency() <= 2);
    }
    catch (...) {
        debug("THREADCOUNT: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if mac address starts with certain VM designated values
     * @category All systems (I think)
     */
    [[nodiscard]] static bool mac_address_check() try {
        if (core::disabled(MAC)) {
            return false;
        }

        // C-style array on purpose
        u8 mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#if (LINUX)
        struct ifreq ifr;
        struct ifconf ifc;
        char buf[1024];
        int32_t success = 0;

        int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

        if (sock == -1) {
            return false;
        };

        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;

        if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
            return false;
        }

        struct ifreq* it = ifc.ifc_req;
        const struct ifreq* end = it + (ifc.ifc_len / sizeof(struct ifreq));

        for (; it != end; ++it) {
            std::strcpy(ifr.ifr_name, it->ifr_name);

            if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) {
                return false;
            }

            if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }

        if (success) {
            std::memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        }
        else {
            debug("MAC: ", "not successful");
        }
#elif (MSVC)
        PIP_ADAPTER_INFO AdapterInfo;
        DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

        AdapterInfo = (IP_ADAPTER_INFO*)std::malloc(sizeof(IP_ADAPTER_INFO));

        if (AdapterInfo == NULL) {
            return false;
        }

        if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
            std::free(AdapterInfo);
            AdapterInfo = (IP_ADAPTER_INFO*)std::malloc(dwBufLen);
            if (AdapterInfo == NULL) {
                return false;
            }
        }

        if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
            PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
            for (std::size_t i = 0; i < 6; i++) {
                mac[i] = pAdapterInfo->Address[i];
            }
        }

        std::free(AdapterInfo);
#else
        return false;
#endif

#ifdef __VMAWARE_DEBUG__
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << std::hex
            << static_cast<int32_t>(mac[0]) << ":"
            << static_cast<int32_t>(mac[1]) << ":"
            << static_cast<int32_t>(mac[2]) << ":"
            << static_cast<int32_t>(mac[3]) << ":"
            << static_cast<int32_t>(mac[4]) << ":"
            << static_cast<int32_t>(mac[5]);
        debug("MAC: ", ss.str());
#endif

        // better expression to fix code duplication
        auto compare = [=](const u8 mac1, const u8 mac2, const u8 mac3) noexcept -> bool {
            return (mac[0] == mac1 && mac[1] == mac2 && mac[2] == mac3);
        };

        if (compare(0x08, 0x00, 0x27)) {
            return core::add(VBOX);
        }

        if (
            (compare(0x00, 0x0C, 0x29)) ||
            (compare(0x00, 0x1C, 0x14)) ||
            (compare(0x00, 0x50, 0x56)) ||
            (compare(0x00, 0x05, 0x69))
        ) {
            return core::add(VMWARE);
        }

        if (compare(0x00, 0x16, 0xE3)) {
            return core::add(XEN);
        }

        if (compare(0x00, 0x1C, 0x42)) {
            return core::add(PARALLELS);
        }

        if (compare(0x0A, 0x00, 0x27)) {
            return core::add(HYBRID);
        }

        return false;
    }
    catch (...) {
        debug("MAC: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if thermal directory is present, might not be present in VMs
     * @category Linux
     */
    [[nodiscard]] static bool temperature() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(TEMPERATURE)) {
            return false;
        }

        return (!util::exists("/sys/class/thermal/thermal_zone0/"));
#endif
    }
    catch (...) {
        debug("TEMPERATURE: catched error, returned false");
        return false;
    }


    /**
     * @brief Check result from systemd-detect-virt tool
     * @category Linux
     */
    [[nodiscard]] static bool systemd_virt() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(SYSTEMD)) {
            return false;
        }

        if (!(util::exists("/usr/bin/systemd-detect-virt") || util::exists("/bin/systemd-detect-virt"))) {
            debug("SYSTEMD: ", "binary doesn't exist");
            return false;
        }

        const std::unique_ptr<std::string> result = util::sys_result("systemd-detect-virt");

        if (result == nullptr) {
            debug("SYSTEMD: ", "invalid stdout output from systemd-detect-virt");
            return false;
        }

        debug("SYSTEMD: ", "output = ", *result);

        return (*result != "none");
#endif
    }
    catch (...) {
        debug("SYSTEMD: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if chassis vendor is a VM vendor
     * @category Linux
     */
    [[nodiscard]] static bool chassis_vendor() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(CVENDOR)) {
            return false;
        }

        const char* vendor_file = "/sys/devices/virtual/dmi/id/chassis_vendor";

        if (!util::exists(vendor_file)) {
            debug("CVENDOR: ", "file doesn't exist");
            return false;
        }

        const std::string vendor = util::read_file(vendor_file);

        // TODO: More can definitely be added, I only tried QEMU and VMware so far
        if (vendor == "QEMU") { return core::add(QEMU); }
        if (vendor == "Oracle Corporation") { return core::add(VMWARE); }

        debug("CVENDOR: ", "unknown vendor = ", vendor);

        return false;
#endif
    }
    catch (...) {
        debug("CVENDOR: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if the chassis type is valid (it's very often invalid in VMs)
     * @category Linux
     */
    [[nodiscard]] static bool chassis_type() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(CTYPE)) {
            return false;
        }

        const char* chassis = "/sys/devices/virtual/dmi/id/chassis_type";

        if (util::exists(chassis)) {
            return (stoi(util::read_file(chassis)) == 1);
        }
        else {
            debug("CTYPE: ", "file doesn't exist");
        }

        return false;
#endif
    }
    catch (...) {
        debug("CTYPE: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if /.dockerenv or /.dockerinit file is present
     * @category Linux
     */
    [[nodiscard]] static bool dockerenv() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(DOCKERENV)) {
            return false;
        }

        return (util::exists("/.dockerenv") || util::exists("/.dockerinit"));
#endif
    }
    catch (...) {
        debug("DOCKERENV: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if dmidecode output matches a VM brand
     * @category Linux
     */
    [[nodiscard]] static bool dmidecode() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(DMIDECODE) || (util::is_admin() == false)) {
            debug("DMIDECODE: ", "precondition return called (root = ", util::is_admin(), ")");
            return false;
        }

        if (!(util::exists("/bin/dmidecode") || util::exists("/usr/bin/dmidecode"))) {
            debug("DMIDECODE: ", "binary doesn't exist");
            return false;
        }

        const std::unique_ptr<std::string> result = util::sys_result("dmidecode -t system | grep 'Manufacturer|Product' | grep -c \"QEMU|VirtualBox|KVM\"");

        if (*result == "" || result == nullptr) {
            debug("DMIDECODE: ", "invalid output");
            return false;
        }
        else if (*result == "QEMU") {
            return core::add(QEMU);
        }
        else if (*result == "VirtualBox") {
            return core::add(VBOX);
        }
        else if (*result == "KVM") {
            return core::add(KVM);
        }
        else if (std::atoi(result->c_str()) >= 1) {
            return true;
        }
        else {
            debug("DMIDECODE: ", "output = ", *result);
        }

        return false;
#endif
    }
    catch (...) {
        debug("DMIDECODE: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if dmesg command output matches a VM brand
     * @category Linux
     */
    [[nodiscard]] static bool dmesg() try {
#if (!LINUX || CPP <= 11)
        return false;
#else
        if (core::disabled(DMESG) || !util::is_admin()) {
            return false;
        }

        if (!util::exists("/bin/dmesg") && !util::exists("/usr/bin/dmesg")) {
            debug("DMESG: ", "binary doesn't exist");
            return false;
        }

        const std::unique_ptr<std::string> result = util::sys_result("dmesg | grep -i hypervisor | grep -c \"KVM|QEMU\"");

        if (*result == "" || result == nullptr) {
            return false;
        }
        else if (*result == "KVM") {
            return core::add(KVM);
        }
        else if (*result == "QEMU") {
            return core::add(QEMU);
        }
        else if (std::atoi(result->c_str())) {
            return true;
        }
        else {
            debug("DMESG: ", "output = ", *result);
        }

        return false;
#endif
    }
    catch (...) {
        debug("DMESG: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if /sys/class/hwmon/ directory is present. If not, likely a VM
     * @category Linux
     */
    [[nodiscard]] static bool hwmon() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(HWMON)) {
            return false;
        }

        return (!util::exists("/sys/class/hwmon/"));
#endif
    }
    catch (...) {
        debug("HWMON: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for tons of VM-specific registry values
     * @category Windows
     */
    [[nodiscard]] static bool registry_key() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(REGISTRY)) {
            return false;
        }

        u8 score = 0;

        auto key = [&score](const char* p_brand, const char* regkey_s) -> void {
            HKEY regkey;
            LONG ret;

            if (util::is_wow64()) {
                wchar_t wRegKey[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, regkey_s, -1, wRegKey, MAX_PATH);

                ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, wRegKey, 0, KEY_READ | KEY_WOW64_64KEY, &regkey);
            }
            else {
                wchar_t wRegKey[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, regkey_s, -1, wRegKey, MAX_PATH);

                ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, wRegKey, 0, KEY_READ, &regkey);
            }

            if (ret == ERROR_SUCCESS) {
                RegCloseKey(regkey);
                score++;

                if (std::string(p_brand) != "") {
                    debug("REGISTRY: ", "detected = ", p_brand);
                    core::add(p_brand);
                }
            }
        };

        // general
        key("", "HKLM\\Software\\Classes\\Folder\\shell\\sandbox");

        // hyper-v
        key(HYPERV, "HKLM\\SOFTWARE\\Microsoft\\Hyper-V");
        key(HYPERV, "HKLM\\SOFTWARE\\Microsoft\\VirtualMachine");
        key(HYPERV, "HKLM\\SOFTWARE\\Microsoft\\Virtual Machine\\Guest\\Parameters");
        key(HYPERV, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmicheartbeat");
        key(HYPERV, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmicvss");
        key(HYPERV, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmicshutdown");
        key(HYPERV, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmicexchange");

        // parallels
        key(PARALLELS, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_1AB8*");

        // sandboxie
        key(SANDBOXIE, "HKLM\\SYSTEM\\CurrentControlSet\\Services\\SbieDrv");
        key(SANDBOXIE, "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Sandboxie");

        // virtualbox
        key(VBOX, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_80EE*");
        key(VBOX, "HKLM\\HARDWARE\\ACPI\\DSDT\\VBOX__");
        key(VBOX, "HKLM\\HARDWARE\\ACPI\\FADT\\VBOX__");
        key(VBOX, "HKLM\\HARDWARE\\ACPI\\RSDT\\VBOX__");
        key(VBOX, "HKLM\\SOFTWARE\\Oracle\\VirtualBox Guest Additions");
        key(VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxGuest");
        key(VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxMouse");
        key(VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxService");
        key(VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxSF");
        key(VBOX, "HKLM\\SYSTEM\\ControlSet001\\Services\\VBoxVideo");

        // virtualpc
        key(VPC, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_5333*");
        key(VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\vpcbus");
        key(VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\vpc-s3");
        key(VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\vpcuhub");
        key(VPC, "HKLM\\SYSTEM\\ControlSet001\\Services\\msvmmouf");

        // vmware
        key(VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\VEN_15AD*");
        key(VMWARE, "HKCU\\SOFTWARE\\VMware, Inc.\\VMware Tools");
        key(VMWARE, "HKLM\\SOFTWARE\\VMware, Inc.\\VMware Tools");
        key(VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmdebug");
        key(VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmmouse");
        key(VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\VMTools");
        key(VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\VMMEMCTL");
        key(VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmware");
        key(VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmci");
        key(VMWARE, "HKLM\\SYSTEM\\ControlSet001\\Services\\vmx86");
        key(VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\CdRomNECVMWar_VMware_IDE_CD*");
        key(VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\CdRomNECVMWar_VMware_SATA_CD*");
        key(VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\DiskVMware_Virtual_IDE_Hard_Drive*");
        key(VMWARE, "HKLM\\SYSTEM\\CurrentControlSet\\Enum\\IDE\\DiskVMware_Virtual_SATA_Hard_Drive*");

        // wine
        key(WINE, "HKCU\\SOFTWARE\\Wine");
        key(WINE, "HKLM\\SOFTWARE\\Wine");

        // xen
        key(XEN, "HKLM\\HARDWARE\\ACPI\\DSDT\\xen");
        key(XEN, "HKLM\\HARDWARE\\ACPI\\FADT\\xen");
        key(XEN, "HKLM\\HARDWARE\\ACPI\\RSDT\\xen");
        key(XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xenevtchn");
        key(XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xennet");
        key(XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xennet6");
        key(XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xensvc");
        key(XEN, "HKLM\\SYSTEM\\ControlSet001\\Services\\xenvdb");

        debug("REGISTRY: ", "score = ", static_cast<u32>(score));

        return (score >= 1);
#endif
    }
    catch (...) {
        debug("REGISTRY: catched error, returned false");
        return false;
    }


    /**
     * @brief checks for default usernames, often a sign of a VM
     * @author: Some guy in a russian underground forum from a screenshot I saw, idk I don't speak russian ¯\_(ツ)_/¯
     * @category Windows
     */
    [[nodiscard]] static bool user_check() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(USER)) {
            return false;
        }

        TCHAR user[UNLEN + 1]{};
        DWORD user_len = UNLEN + 1;
        GetUserName(user, &user_len);

        //TODO Ansi: debug("USER: ", "output = ", user);

        if (0 == _tcscmp(user, _T("username"))) {
            return core::add(THREADEXPERT);
        }

        if (0 == _tcscmp(user, _T("vmware"))) {
            return core::add(VMWARE);
        }

        if (0 == _tcscmp(user, _T("user"))) {
            return core::add(SANDBOXIE);
        }

        return (
            (0 == _tcscmp(user, _T("USER"))) ||      // Sandbox
            (0 == _tcscmp(user, _T("currentuser")))  // Normal
        );
#endif
    }
    catch (...) {
        debug("USER: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if CWSandbox-specific file exists
     * @author same russian guy as above. Whoever you are, ty
     * @category Windows
     */
    [[nodiscard]] static bool cwsandbox_check() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(CWSANDBOX_VM)) {
            return false;
        }

        if (util::exists(_T("C:\\analysis"))) {
            return core::add(CWSANDBOX);
        }

        return false;
#endif
    }
    catch (...) {
        debug("CWSANDBOX_VM: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for VM-specific DLLs
     * @category Windows
     */
    [[nodiscard]] static bool DLL_check() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(DLL)) {
            return false;
        }

        std::vector<const char*> real_dlls = {
            "kernel32.dll",
            "networkexplorer.dll",
            "NlsData0000.dll"
        };

        std::vector<const char*> false_dlls = {
            "NetProjW.dll",
            "Ghofr.dll",
            "fg122.dll"
        };

        HMODULE lib_inst;

        for (auto& dll : real_dlls) {
            lib_inst = LoadLibraryA(dll);
            if (lib_inst == nullptr) {
                debug("DLL: ", "LIB_INST detected true for real dll = ", dll);
                return true;
            }
            FreeLibrary(lib_inst);
        }

        for (auto& dll : false_dlls) {
            lib_inst = LoadLibraryA(dll);
            if (lib_inst != nullptr) {
                debug("DLL: ", "LIB_INST detected true for false dll = ", dll);
                return true;
            }
        }

        return false;
#endif
    }
    catch (...) {
        debug("DLL: catched error, returned false");
        return false;
    }


    /**
     * @brief Check VBox RdrDN
     * @category Windows
     */
    [[nodiscard]] static bool vbox_registry() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VBOX_REG)) {
            return false;
        }

        HANDLE handle = CreateFile(_T("\\\\.\\VBoxMiniRdrDN"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
            return core::add(VBOX);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VBOX_REG: catched error, returned false");
        return false;
    }


    /**
     * @brief Find VMware tools presence
     * @category Windows
     */
    [[nodiscard]] static bool vmware_registry() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VMWARE_REG)) {
            return false;
        }

        HKEY hKey;
        // Use wide string literal
        bool result = (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\VMware, Inc.\\VMware Tools", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS);

        debug("VMWARE_REG: result = ", result);

        if (result == true) {
            return core::add(VMWARE);
        }

        return result;
#endif
    }
    catch (...) {
        debug("VMWARE_REG: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if the mouse coordinates have changed after 5 seconds
     * @note Some VMs are automatic without a human due to mass malware scanning being a thing
     * @note Disabled by default due to performance reasons
     * @category Windows
     */
    [[nodiscard]] static bool cursor_check() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(CURSOR)) {
            return false;
        }

        POINT pos1, pos2;
        GetCursorPos(&pos1);

        debug("CURSOR: pos1.x = ", pos1.x);
        debug("CURSOR: pos1.y = ", pos1.y);

        Sleep(5000);
        GetCursorPos(&pos2);

        debug("CURSOR: pos1.x = ", pos1.x);
        debug("CURSOR: pos1.y = ", pos1.y);
        debug("CURSOR: pos2.x = ", pos2.x);
        debug("CURSOR: pos2.y = ", pos2.y);

        return ((pos1.x == pos2.x) && (pos1.y == pos2.y));
#endif
    }
    catch (...) {
        debug("CURSOR: catched error, returned false");
        return false;
    }


    /**
     * @brief Find for VMware and VBox specific files
     * @category Windows
     */
    [[nodiscard]] static bool vm_files() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VM_FILES)) {
            return false;
        }

        // points
        u8 vbox = 0;
        u8 vmware = 0;

        constexpr std::array<const TCHAR*, 26> files = { {
            // VMware
            _T("C:\\windows\\System32\\Drivers\\Vmmouse.sys"),
            _T("C:\\windows\\System32\\Drivers\\vm3dgl.dll"),
            _T("C:\\windows\\System32\\Drivers\\vmdum.dll"),
            _T("C:\\windows\\System32\\Drivers\\VmGuestLibJava.dll"),
            _T("C:\\windows\\System32\\Drivers\\vm3dver.dll"),
            _T("C:\\windows\\System32\\Drivers\\vmtray.dll"),
            _T("C:\\windows\\System32\\Drivers\\VMToolsHook.dll"),
            _T("C:\\windows\\System32\\Drivers\\vmGuestLib.dll"),
            _T("C:\\windows\\System32\\Drivers\\vmhgfs.dll"),

            // VBox
            _T("C:\\windows\\System32\\Drivers\\VBoxMouse.sys"),
            _T("C:\\windows\\System32\\Drivers\\VBoxGuest.sys"),
            _T("C:\\windows\\System32\\Drivers\\VBoxSF.sys"),
            _T("C:\\windows\\System32\\Drivers\\VBoxVideo.sys"),
            _T("C:\\windows\\System32\\vboxoglpackspu.dll"),
            _T("C:\\windows\\System32\\vboxoglpassthroughspu.dll"),
            _T("C:\\windows\\System32\\vboxservice.exe"),
            _T("C:\\windows\\System32\\vboxoglcrutil.dll"),
            _T("C:\\windows\\System32\\vboxdisp.dll"),
            _T("C:\\windows\\System32\\vboxhook.dll"),
            _T("C:\\windows\\System32\\vboxmrxnp.dll"),
            _T("C:\\windows\\System32\\vboxogl.dll"),
            _T("C:\\windows\\System32\\vboxtray.exe"),
            _T("C:\\windows\\System32\\VBoxControl.exe"),
            _T("C:\\windows\\System32\\vboxoglerrorspu.dll"),
            _T("C:\\windows\\System32\\vboxoglfeedbackspu.dll"),
            _T("c:\\windows\\system32\\vboxoglarrayspu.dll")
        } };

        for (const auto file : files) {
            if (util::exists(file)) {
                const auto regex = tregex(file, std::regex::icase);

                if (std::regex_search(_T("vbox"), regex)) {
                    //TODO Ansi: debug("VM_FILES: found vbox file = ", file);
                    vbox++;
                }
                else {
                    //TODO Ansi: debug("VM_FILES: found vmware file = ", file);
                    vmware++;
                }
            }
        }

        debug("VM_FILES: vmware score: ", static_cast<u32>(vmware));
        debug("VM_FILES: vbox score: ", static_cast<u32>(vbox));

        if (vbox > vmware) {
            return core::add(VBOX);
        }
        else if (vbox < vmware) {
            return core::add(VMWARE);
        }
        else if (
            vbox > 0 &&
            vmware > 0 &&
            vbox == vmware
            ) {
            return true;
        }

        return false;
#endif
    }
    catch (...) {
        debug("VM_FILES: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for sysctl hardware model
     * @author MacRansom ransomware
     * @category MacOS
     */
    [[nodiscard]] static bool hwmodel() try {
#if (!APPLE)
        return false;
#else
        if (core::disabled(HWMODEL)) {
            return false;
        }

        auto result = util::sys_result("sysctl -n hw.model");

        std::smatch match;

        if (result == nullptr) {
            debug("HWMODEL: ", "null result received");
            return false;
        }

        debug("HWMODEL: ", "output = ", *result);

        // if string contains "Mac" anywhere in the string, assume it's baremetal
        if (std::regex_search(*result, match, std::regex("Mac"))) {
            return false;
        }

        // not sure about the other VMs, more could potentially be added
        if (std::regex_search(*result, match, std::regex("VMware"))) {
            return core::add(VMWARE);
        }

        // assumed true since it doesn't contain "Mac" string
        return true;
#endif
    }
    catch (...) {
        debug("HWMODEL: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if disk size is too low
     * @category Linux (for now)
     */
    [[nodiscard]] static bool disk_size() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(DISK_SIZE)) {
            return false;
        }

        const u32 size = util::get_disk_size();

        debug("DISK_SIZE: size = ", size);

        return (size <= 60); // in GB
#endif
    }
    catch (...) {
        debug("DISK_SIZE: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for match with default RAM and disk size (VBOX-specific)
     * @note        RAM     DISK
     * WINDOWS 11:  4096MB, 80GB
     * WINDOWS 10:  2048MB, 50GB
     * ARCH, OPENSUSE, REDHAD, GENTOO, FEDORA, DEBIAN: 1024MB, 8GB
     * UBUNTU:      1028MB, 10GB
     * ORACLE:      1024MB, 12GB
     * OTHER LINUX: 512MB,  8GB

     * @todo: check if it still applies to host systems with larger RAM and disk size than what I have
     * @category Linux, Windows
     */
    [[nodiscard]] static bool vbox_default_specs() try {
#if (APPLE)
        return false;
#else
        if (core::disabled(VBOX_DEFAULT)) {
            return false;
        }

        const u32 disk = util::get_disk_size();
        const u64 ram = util::get_physical_ram_size();

        debug("VBOX_DEFAULT: disk = ", disk);
        debug("VBOX_DEFAULT: ram = ", ram);

        if ((disk > 80) || (ram > 4)) {
            debug("VBOX_DEFAULT: returned false due to lack of precondition spec comparisons");
            return false;
        }

    #if (LINUX)
        auto get_distro = []() -> std::string {
            std::ifstream osReleaseFile("/etc/os-release");
            std::string line;

            while (std::getline(osReleaseFile, line)) {
                if (line.find("ID=") != std::string::npos) {
                    const std::size_t start = line.find('"');
                    const std::size_t end = line.rfind('"');
                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        return line.substr(start + 1, end - start - 1);
                    }
                }
            }

            return "unknown";
            };

        const std::string distro = get_distro();

        debug("VBOX_DEFAULT: linux, detected distro: ", distro);

        // yoda notation ftw
        if ("unknown" == distro) {
            return false;
        }

        if (
            "arch" == distro ||
            "opensuse" == distro ||
            "redhat" == distro ||
            "gentoo" == distro ||
            "fedora" == distro ||
            "debian" == distro
            ) {
            return ((8 == disk) && (1 == ram));
        }

        if ("ubuntu" == distro) {
            return ((10 == disk) && (1 == ram));
        }

        if ("ol" == distro) { // ol = oracle
            return ((12 == disk) && (1 == ram));
        }
    #elif (MSVC)
        const u16 ret = util::get_windows_version();
        // less than windows 10
        if (ret < 10) {
            debug("VBOX_DEFAULT: less than windows 10 detected");
            return false;
        }

        // windows 10
        if (10 == ret) {
            debug("VBOX_DEFAULT: windows 10 detected");
            return ((50 == disk) && (2 == ram));
        }

        // windows 11
        if (11 == ret) {
            debug("VBOX_DEFAULT: windows 11 detected");
            return ((80 == disk) && (4 == ram));
        }
    #endif
#endif
        return false;
    }
    catch (...) {
        debug("VBOX_DEFAULT: catched error, returned false");
        return false;
    }


    /**
     * @brief Check VBox network provider string
     * @category Windows
     */
    [[nodiscard]] static bool vbox_network_share() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VBOX_NETWORK)) {
            return false;
        }

        u32 pnsize = 0x1000;
        TCHAR* provider = new TCHAR[pnsize];
        
        u32 retv = WNetGetProviderName(WNNC_NET_RDR2SAMPLE, provider, reinterpret_cast<LPDWORD>(&pnsize));
        
        if (retv == NO_ERROR) {
            bool result = (lstrcmpi(provider, _T("VirtualBox Shared Folders")) == 0);
            delete[] provider;
            return result;
        }
        
        delete[] provider;
        return false;
#endif
    }
    catch (...) {
        debug("VBOX_NETWORK: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for any VM processes that are active
     * @category Windows
     */
    [[nodiscard]] static bool vm_processes() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VM_PROCESSES)) {
            return false;
        }

        auto check_proc = [](const TCHAR* proc) -> bool {
            DWORD processes[1024], bytesReturned;

            // Retrieve the list of process identifiers
            if (!EnumProcesses(processes, sizeof(processes), &bytesReturned))
                return false;

            // Calculate how many process identifiers were returned
            DWORD numProcesses = bytesReturned / sizeof(DWORD);

            for (DWORD i = 0; i < numProcesses; ++i) {
                // Open the process
                HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                if (process != nullptr) {
                    // Get the process name
                    TCHAR processName[MAX_PATH];
                    if (GetModuleBaseName(process, nullptr, processName, sizeof(processName) / sizeof(TCHAR))) {
                        // Check if the process name matches the desired executable
                        if (_tcscmp(processName, proc) == 0) {
                            CloseHandle(process);
                            return true;
                        }
                    }
                    CloseHandle(process);
                }
            }

            return false;
        };

        if (check_proc(_T("joeboxserver.exe")) || check_proc(_T("joeboxcontrol.exe"))) {
            return core::add(JOEBOX);
        }

        if (check_proc(_T("prl_cc.exe")) || check_proc(_T("prl_tools.exe"))) {
            return core::add(PARALLELS);
        }

        if (check_proc(_T("vboxservice.exe")) || check_proc(_T("vboxtray.exe"))) {
            return core::add(VBOX);
        }

        if (check_proc(_T("vmsrvc.exe")) || check_proc(_T("vmusrvc.exe"))) {
            return core::add(VPC);
        }

        if (
            check_proc(_T("vmtoolsd.exe")) ||
            check_proc(_T("vmwaretrat.exe")) ||
            check_proc(_T("vmacthlp.exe")) ||
            check_proc(_T("vmwaretray.exe")) ||
            check_proc(_T("vmwareuser.exe")) ||
            check_proc(_T("vmware.exe")) ||
            check_proc(_T("vmount2.exe"))
        ) {
            return core::add(VMWARE);
        }

        if (check_proc(_T("xenservice.exe")) || check_proc(_T("xsvc_depriv.exe"))) {
            return core::add(XEN);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VM_PROCESSES: caught error, returned false");
        return false;
    }


    /**
     * @brief Check for default VM username and hostname for linux
     * @category Linux
     */
    [[nodiscard]] static bool linux_user_host() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(LINUX_USER_HOST)) {
            return false;
        }

        if (util::is_admin()) {
            return false;
        }

        const char* username = std::getenv("USER");
        const char* hostname = std::getenv("HOSTNAME");

        debug("LINUX_USER_HOST: user = ", username);
        debug("LINUX_USER_HOST: host = ", hostname);

        return (
            (strcmp(username, "liveuser") == 0) &&
            (strcmp(hostname, "localhost-live") == 0)
            );
#endif
    }
    catch (...) {
        debug("LINUX_USER_HOST: catched error, returned false");
        return false;
    }


    /**
     * @brief Gamarue ransomware check
     * @category Windows
     */
    [[nodiscard]] static bool gamarue() try {
#if (!MSVC) 
        return false;
#else
        if (core::disabled(GAMARUE)) {
            return false;
        }

        HKEY hOpen;
        char* szBuff;
        int iBuffSize;
        HANDLE hMod;
        LONG nRes;

        szBuff = (char*)calloc(512, sizeof(char));

        hMod = GetModuleHandleW(L"SbieDll.dll"); // Sandboxie
        if (hMod != 0) {
            free(szBuff);
            return core::add(SANDBOXIE);
        }

        hMod = GetModuleHandleW(L"dbghelp.dll"); // Thread Expert
        if (hMod != 0) {
            free(szBuff);
            return core::add(THREADEXPERT);
        }

        nRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion", 0L, KEY_QUERY_VALUE, &hOpen);
        if (nRes == ERROR_SUCCESS) {
            iBuffSize = sizeof(szBuff);
            nRes = RegQueryValueExW(hOpen, L"ProductId", NULL, NULL, (unsigned char*)szBuff, reinterpret_cast<LPDWORD>(&iBuffSize));
            if (nRes == ERROR_SUCCESS) {
                // Check if szBuff is not NULL before using strcmp
                if (szBuff == NULL) {
                    RegCloseKey(hOpen);
                    return false;
                }

                if (strcmp(szBuff, "55274-640-2673064-23950") == 0) { // joebox
                    free(szBuff);
                    return core::add(JOEBOX);
                }
                else if (strcmp(szBuff, "76487-644-3177037-23510") == 0) { // CW Sandbox
                    free(szBuff);
                    return core::add(CWSANDBOX);
                }
                else if (strcmp(szBuff, "76487-337-8429955-22614") == 0) { // anubis
                    free(szBuff);
                    return core::add(ANUBIS);
                }
                else {
                    free(szBuff);
                    return false;
                }
            }
            RegCloseKey(hOpen);
        }
        // Set szBuff to NULL after freeing to avoid double free issues
        free(szBuff);
        return false;
#endif
    }
    catch (...) {
        debug("GAMARUE: catched error, returned false");
        return false;
    }


    /**
     * @brief Check if the BIOS serial is valid
     * @category Linux
     */
    [[nodiscard]] static bool bios_serial() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(BIOS_SERIAL)) {
            return false;
        }

        std::unique_ptr<util::sys_info> info = std::make_unique<util::sys_info>();

        const std::string str = info->get_serialnumber();
        const std::size_t nl_pos = str.find('\n');

        if (nl_pos == std::string::npos) {
            return false;
        }

        debug("BIOS_SERIAL: ", str);

        const std::string extract = str.substr(nl_pos + 1);

        const bool all_digits = std::all_of(extract.cbegin(), extract.cend(), [](const char c) {
            return std::isdigit(c);
            });

        if (all_digits) {
            if (extract == "0") {
                return true;
            }
        }

        return false;
#endif
    }
    catch (...) {
        debug("BIOS_SERIAL: catched error, returned false");
        return false;
    }


    /**
     * @brief check for any indication of parallels through BIOS stuff
     * @link https://stackoverflow.com/questions/1370586/detect-if-windows-is-running-from-within-parallels
     * @category Windows
     */
    [[nodiscard]] static bool parallels() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(PARALLELS_VM)) {
            return false;
        }

        std::unique_ptr<util::sys_info> info = std::make_unique<util::sys_info>();

#ifdef __VMAWARE_DEBUG__
        std::cout << std::left << ::std::setw(14) << "Manufacturer: " << info->get_manufacturer() << '\n'
            << std::left << ::std::setw(14) << "Product Name: " << info->get_productname() << '\n'
            << std::left << ::std::setw(14) << "Serial No: " << info->get_serialnumber() << '\n'
            << std::left << ::std::setw(14) << "UUID: " << info->get_uuid() << '\n'
            << std::left << ::std::setw(14) << "Version: " << info->get_version() << std::endl;

        if (!info->get_family().empty()) {
            std::cout << std::left << ::std::setw(14) << "Product family: " << info->get_family() << std::endl;
        }

        if (!info->get_sku().empty()) {
            std::cout << std::left << ::std::setw(14) << "SKU/Configuration: " << info->get_sku() << std::endl;
        }
#endif

        auto compare = [](const std::string& str) -> bool {
            std::regex pattern("Parallels", std::regex_constants::icase);
            return std::regex_match(str, pattern);
            };

        if (
            compare(info->get_manufacturer()) ||
            compare(info->get_productname()) ||
            compare(info->get_family())
            ) {
            return core::add(PARALLELS);
        }

        return false;
#endif
    }
    catch (...) {
        debug("PARALLELS_VM:", "catched error, returned false");
        return false;
    }


    /**
     * @brief check VM through alternative RDTSC technique with VMEXIT
     * @category x86
     */
    [[nodiscard]] 
#if (LINUX)
    // this is added so no sanitizers can potentially cause unwanted delays while measuring rdtsc in a debug compilation
    __attribute__((no_sanitize("address", "leak", "thread", "undefined")))
#endif
    static bool rdtsc_vmexit() try {

#if (!x86)
        return false;
#else
        if (core::disabled(RDTSC_VMEXIT)) {
            return false;
        }

        u64 tsc1 = 0;
        u64 tsc2 = 0;
        u64 avg = 0;
        i32 reg[4] = {};

        for (std::size_t i = 0; i < 10; i++) {
            tsc1 = __rdtsc();
            cpu::cpuid(reg, 0);
            tsc2 = __rdtsc();
            avg += (tsc2 - tsc1);
        }

        avg /= 10;

        return (avg >= 1500 || avg == 0);
#endif
    }
    catch (...) {
        debug("RDTSC_VMEXIT:", "catched error, returned false");
        return false;
    }


    /**
     * @brief Do various Bochs-related CPU stuff
     * @category x86
     * @note Discovered by Peter Ferrie, Senior Principal Researcher, Symantec Advanced Threat Research peter_ferrie@symantec.com
     */
    [[nodiscard]] static bool bochs_cpu() try {
#if (!x86)
        return false;
#else
        if (!cpuid_supported || core::disabled(BOCHS_CPU)) {
            return false;
        }

        const bool intel = cpu::is_intel();
        const bool amd = cpu::is_amd();

        // if neither amd or intel, return false
        if (!(intel ^ amd)) {
            return false;
        }

        const std::string brand = cpu::get_brand();

        if (intel) {
            // technique 1: not a valid brand 
            if (brand == "              Intel(R) Pentium(R) 4 CPU        ") {
                return core::add(BOCHS);
            }
        } else if (amd) {
            // technique 2: "processor" should have a capital P
            if (brand == "AMD Athlon(tm) processor") {
                return core::add(BOCHS);
            }

            // technique 3: Check for absence of AMD easter egg for K7 and K8 CPUs
            u32 unused, eax = 0;
            cpu::cpuid(eax, unused, unused, unused, 1);

            constexpr u8 AMD_K7 = 6;
            constexpr u8 AMD_K8 = 15;

            const u32 family = ((eax >> 8) & 0xF);

            if (family != AMD_K7 && family != AMD_K8) {
                return false;
            }

            u32 ecx_bochs = 0;
            cpu::cpuid(unused, unused, ecx_bochs, unused, cpu::leaf::amd_easter_egg);

            if (ecx_bochs == 0) {
                return core::add(BOCHS);
            }
        }

        return false;
#endif
    }
    catch (...) {
        debug("BOCHS_CPU:", "catched error, returned false");
        return false;
    }


    /**
     * @brief Go through the motherboard and match for VPC-specific string
     * @category Windows
     */
    [[nodiscard]] static bool vpc_board() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VPC_BOARD)) {
            return false;
        }

        HRESULT hres;

        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to initialize COM library. Error code: ", hres);
            return false;
        }

        hres = CoInitializeSecurity(
            NULL,
            -1,                          // use the default authentication service
            NULL,                        // use the default authorization service
            NULL,                        // reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // authentication
            RPC_C_IMP_LEVEL_IMPERSONATE, // impersonation
            NULL,                        // authentication info
            EOAC_NONE,                   // additional capabilities
            NULL                         // reserved
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to initialize security. Error code: ", hres);
            CoUninitialize();
            return false;
        }

        IWbemLocator* pLoc = NULL;
        IWbemServices* pSvc = NULL;

        hres = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            (LPVOID*)&pLoc
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to create IWbemLocator object. Error code: ", hres);
            CoUninitialize();
            return false;
        }

        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), // Namespace
            NULL,                    // User name
            NULL,                    // User password
            0,                       // Locale
            NULL,                    // Security flags
            0,                       // Authority
            0,                       // Context object pointer
            &pSvc
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to connect to WMI. Error code: ", hres);
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        hres = CoSetProxyBlanket(
            pSvc,                        // Indicates the proxy to set
            RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
            RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
            NULL,                        // Server principal name
            RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
            NULL,                        // client identity
            EOAC_NONE                    // proxy capabilities
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to set proxy blanket. Error code: ", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        IEnumWbemClassObject* enumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_BaseBoard"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &enumerator
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Query for Win32_BaseBoard failed. Error code: ", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        bool is_vm = false;

        while (enumerator) {
            HRESULT hr = enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0) {
                break;
            }

            VARIANT vtProp;
            VariantInit(&vtProp);
            hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);

            if (SUCCEEDED(hr)) {
                if (vtProp.vt == VT_BSTR && _wcsicmp(vtProp.bstrVal, L"Microsoft Corporation") == 0) {
                    is_vm = true;
                    VariantClear(&vtProp);
                    break;
                }

                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }

        enumerator->Release();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();

        if (is_vm) {
            return core::add(VPC);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VPC_BOARD:", "catched error, returned false");
        return false;
    }


    /**
     * @brief get WMI query for HYPERV name
     * @category Windows
     * @note idea is from nettitude
     * @link https://labs.nettitude.com/blog/vm-detection-tricks-part-3-hyper-v-raw-network-protocol/
     */
    [[nodiscard]] static bool hyperv_wmi() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(HYPERV_WMI)) {
            return false;
        }

        HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            debug("HYPERV_WMI: Failed to initialize COM library. Error code = ", hres);
            return false;
        }

        hres = CoInitializeSecurity(
            NULL,
            -1,                          // COM authentication
            NULL,                        // Authentication services
            NULL,                        // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
            NULL,                        // Authentication info
            EOAC_NONE,                   // Additional capabilities
            NULL                         // Reserved
        );

        if (FAILED(hres)) {
            debug("HYPERV_WMI: Failed to initialize security. Error code = ", hres);
            CoUninitialize();
            return false;
        }

        // Connect to WMI
        IWbemLocator* pLoc = NULL;
        hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

        if (FAILED(hres)) {
            debug("HYPERV_WMI: Failed to create IWbemLocator object. Error code = ", hres);
            CoUninitialize();
            return false;
        }

        IWbemServices* pSvc = NULL;
        hres = pLoc->ConnectServer(
            _bstr_t(L"\\\\.\\root\\CIMV2"),   // Object path of WMI namespace
            NULL,                             // User name. NULL = current user
            NULL,                             // User password. NULL = current
            0,                                // Locale. NULL indicates current
            NULL,                             // Security flags.
            0,                                // Authority (e.g. Kerberos)
            0,                                // Context object
            &pSvc                             // pointer to IWbemServices proxy
        );

        if (FAILED(hres)) {
            debug("HYPERV_WMI: Could not connect. Error code = ", hres);
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        hres = CoSetProxyBlanket(
            pSvc,                        // Indicates the proxy to set
            RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
            RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
            NULL,                        // Server principal name
            RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
            NULL,                        // client identity
            EOAC_NONE                    // proxy capabilities
        );

        if (FAILED(hres)) {
            debug("HYPERV_WMI: Could not set proxy blanket. Error code = ", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            _bstr_t(L"WQL"),
            _bstr_t(L"SELECT * FROM Win32_NetworkProtocol"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );

        if (FAILED(hres)) {
            debug("HYPERV_WMI: ExecQuery failed. Error code = ", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        bool is_vm = false;

        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0 || FAILED(hr)) {
                break;
            }

            VARIANT vtProp;
            VariantInit(&vtProp);
            hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);

            if (!FAILED(hr)) {
                if (vtProp.vt == VT_BSTR) {
                    is_vm = (wcscmp(vtProp.bstrVal, L"Hyper-V RAW") == 0);
                }
            }

            VariantClear(&vtProp);
            pclsObj->Release();
            pclsObj = NULL;
        }

        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        CoUninitialize();

        return is_vm;
#endif
    }
    catch (...) {
        debug("HYPERV_WMI: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief compare for hyperv-specific string in registry
     * @category Windows
     * @note idea is from nettitude
     * @link https://labs.nettitude.com/blog/vm-detection-tricks-part-3-hyper-v-raw-network-protocol/
     */
    [[nodiscard]] static bool hyperv_registry() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(HYPERV_REG)) {
            return false;
        }

        constexpr const char* registryPath = "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries";

        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, reinterpret_cast<LPCWSTR>(registryPath), 0, KEY_READ, &hKey);

        if (result != ERROR_SUCCESS) {
            debug("HYPERV_WMI: Error opening registry key. Code: ", result);
            return false;
        }

        bool is_vm = false;

        DWORD index = 0;
        wchar_t subkeyName[256];
        DWORD subkeyNameSize = sizeof(subkeyName) / sizeof(subkeyName[0]);

        while (RegEnumKeyExW(hKey, index++, subkeyName, &subkeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            HKEY subkey;
            result = RegOpenKeyExW(hKey, subkeyName, 0, KEY_READ, &subkey);

            if (result == ERROR_SUCCESS) {
                wchar_t protocolName[256]{};
                DWORD dataSize = sizeof(protocolName);

                // Check if the "ProtocolName" value exists
                result = RegQueryValueExW(subkey, L"ProtocolName", NULL, NULL, reinterpret_cast<LPBYTE>(protocolName), &dataSize);

                if (result == ERROR_SUCCESS) {
                    if (wcscmp(protocolName, L"Hyper-V RAW") == 0) {
                        is_vm = true;
                        break;
                    }
                }

                RegCloseKey(subkey);
            }

            subkeyNameSize = sizeof(subkeyName) / sizeof(subkeyName[0]);
        }

        RegCloseKey(hKey);

        return is_vm;
#endif 
    }
    catch (...) {
        debug("HYPERV_WMI: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for VirtualBox-specific string for shared folder ID
     * @category Windows
     * @note slightly modified code from original
     * @author @waleedassar
     * @link https://pastebin.com/xhFABpPL
     */
    [[nodiscard]] static bool vbox_shared_folders() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VBOX_FOLDERS)) {
            return false;
        }

        DWORD pnsize = 0;  // Initialize to 0 to query the required size
        wchar_t* provider = nullptr;

        // Query the required size
        DWORD retv = WNetGetProviderNameW(WNNC_NET_RDR2SAMPLE, nullptr, &pnsize);

        if (retv == ERROR_MORE_DATA) {
            // Allocate a buffer of the required size
            provider = static_cast<wchar_t*>(LocalAlloc(LMEM_ZEROINIT, pnsize));

            if (provider != nullptr) {
                // Retrieve the actual data
                retv = WNetGetProviderNameW(WNNC_NET_RDR2SAMPLE, provider, &pnsize);
            }
        }

        if (retv == NO_ERROR && provider != nullptr) {
            if (lstrcmpiW(provider, L"VirtualBox Shared Folders") == 0) {
                LocalFree(provider);
                return core::add(VBOX);
            }
        }

        // Clean up the allocated buffer
        LocalFree(provider);

        return false;

#endif
    }
    catch (...) {
        debug("VBOX_FOLDERS: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check MSSMBIOS registry for VM-specific strings
     * @category Windows
     * @note slightly modified from original code
     * @author @waleedassar
     * @link https://pastebin.com/fPY4MiYq
     */
    [[nodiscard]] static bool mssmbios() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(MSSMBIOS)) {
            return false;
        }

        HKEY hk = 0;
        int ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\mssmbios\\data", 0, KEY_ALL_ACCESS, &hk);
        if (ret != ERROR_SUCCESS) {
            return false;
        }

        bool is_vm = false;
        unsigned long type = 0;
        unsigned long length = 0;
        ret = RegQueryValueExW(hk, L"SMBiosData", 0, &type, 0, &length);

        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hk);
            return false;
        }

        if (length == 0) {
            RegCloseKey(hk);
            return false;
        }

        char* p = static_cast<char*>(LocalAlloc(LMEM_ZEROINIT, length));

        if (p == nullptr) {
            RegCloseKey(hk);
            return false;
        }

        ret = RegQueryValueExW(hk, L"SMBiosData", 0, &type, (unsigned char*)p, &length);

        if (ret != ERROR_SUCCESS) {
            LocalFree(p);
            RegCloseKey(hk);
            return false;
        }

        auto ScanDataForString = [](unsigned char* data, unsigned long data_length, unsigned char* string2) -> unsigned char* {
            std::size_t string_length = strlen(reinterpret_cast<char*>(string2));

            for (std::size_t i = 0; i <= (data_length - string_length); i++) {
                if (strncmp(reinterpret_cast<char*>(&data[i]), reinterpret_cast<char*>(string2), string_length) == 0) {
                    return &data[i];
                }
            }

            return 0;
        };

        auto AllToUpper = [](char* str, std::size_t len) {
            for (std::size_t i = 0; i < len; ++i) {
                str[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[i])));
            }
        };

        AllToUpper(p, length);

        // cleaner and better shortcut than typing reinterpret_cast<unsigned char*> a million times
        auto cast = [](char* p) -> unsigned char* {
            return reinterpret_cast<unsigned char*>(p);
        };

        unsigned char* x1 = ScanDataForString(cast(p), length, (unsigned char*)("INNOTEK GMBH"));
        unsigned char* x2 = ScanDataForString(cast(p), length, (unsigned char*)("VIRTUALBOX"));
        unsigned char* x3 = ScanDataForString(cast(p), length, (unsigned char*)("SUN MICROSYSTEMS"));
        unsigned char* x4 = ScanDataForString(cast(p), length, (unsigned char*)("VBOXVER"));
        unsigned char* x5 = ScanDataForString(cast(p), length, (unsigned char*)("VIRTUAL MACHINE"));

        if (x1 || x2 || x3 || x4 || x5) {
            is_vm = true;
#ifdef __VMAWARE_DEBUG__
            if (x1) { debug("VBOX_MSSMBIOS: x1 = ", x1); }
            if (x2) { debug("VBOX_MSSMBIOS: x2 = ", x2); }
            if (x3) { debug("VBOX_MSSMBIOS: x3 = ", x3); }
            if (x4) { debug("VBOX_MSSMBIOS: x4 = ", x4); }
            if (x5) { debug("VBOX_MSSMBIOS: x5 = ", x5); }
#endif
        }

        LocalFree(p);
        RegCloseKey(hk);

        if (is_vm) {
            if (x5) {
                return true;
            }

            return core::add(VBOX);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VBOX_MSSMBIOS: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check if memory is too low for MacOS system
     * @category MacOS
     * @link https://evasions.checkpoint.com/techniques/macos.html
     */
    [[nodiscard]] static bool hw_memsize() try {
#if (!APPLE)
        return false;
#else
        if (core::disabled(MAC_MEMSIZE)) {
            return false;
        }

        std::unique_ptr<std::string> result = util::sys_result("sysctl -n hw.memsize");
        const std::string ram = *result;

        if (ram == "0") {
            return false;
        }

        debug("MAC_MEMSIZE: ", "ram size = ", ram);

        for (const char c : ram) {
            if (!std::isdigit(c)) {
                debug("MAC_MEMSIZE: ", "found non-digit character, returned false");
                return false;
            }
        }

        const u64 ram_u64 = std::stoull(ram);

        debug("MAC_MEMSIZE: ", "ram size in u64 = ", ram_u64);

        constexpr u64 limit = 4000000000; // 4GB 

        return (ram_u64 <= limit);
#endif
    }
    catch (...) {
        debug("MAC_MEMSIZE: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check MacOS' IO kit registry for VM-specific strings
     * @category MacOS
     * @link https://evasions.checkpoint.com/techniques/macos.html
     */
    [[nodiscard]] static bool io_kit() try {
#if (!APPLE)
        return false;
#else
        if (core::disabled(MAC_IOKIT)) {
            return false;
        }

        // board_ptr and manufacturer_ptr empty
        std::unique_ptr<std::string> platform_ptr = util::sys_result("ioreg -rd1 -c IOPlatformExpertDevice");
        std::unique_ptr<std::string> board_ptr = util::sys_result("ioreg -rd1 -c board-id");
        std::unique_ptr<std::string> manufacturer_ptr = util::sys_result("ioreg -rd1 -c manufacturer");

        const std::string platform = *platform_ptr;
        const std::string board = *board_ptr;
        const std::string manufacturer = *manufacturer_ptr;
        
        auto check_platform = [&]() -> bool {
            debug("IO_KIT: ", "platform = ", platform);

            if (platform.empty()) {
                return false;
            }

            for (const char c : platform) {
                if (!std::isdigit(c)) {
                    return false;
                }
            }

            return (platform == "0");
        };

        auto check_board = [&]() -> bool {
            debug("IO_KIT: ", "board = ", board);

            if (board.empty()) {
                return false;
            }

            if (util::find(board, "Mac")) {
                return false;
            }

            if (util::find(board, "VirtualBox")) {
                return core::add(VBOX);
            }

            if (util::find(board, "VMware")) {
                return core::add(VMWARE);
            }

            return false;
        };

        auto check_manufacturer = [&]() -> bool {
            debug("IO_KIT: ", "manufacturer = ", manufacturer);

            if (manufacturer.empty()) {
                return false;
            }

            if (util::find(manufacturer, "Apple")) {
                return false;
            }

            if (util::find(manufacturer, "innotek")) {
                return core::add(VBOX);
            }

            return false;
        };

        return (
            check_platform() ||
            check_board() ||
            check_manufacturer()
        );

        return false;
#endif            
    }
    catch (...) {
        debug("MAC_IOKIT: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for VM-strings in ioreg commands for MacOS
     * @category MacOS
     * @link https://evasions.checkpoint.com/techniques/macos.html
     */
    [[nodiscard]] static bool ioreg_grep() try {
#if (!APPLE)
        return false;
#else
        if (core::disabled(IOREG_GREP)) {
            return false;
        }

        auto check_usb = []() -> bool {
            std::unique_ptr<std::string> result = util::sys_result("ioreg -rd1 -c IOUSBHostDevice | grep \"USB Vendor Name\"");
            const std::string usb = *result;

            if (util::find(usb, "Apple")) {
                return false;
            }

            if (util::find(usb, "VirtualBox")) {
                return core::add(VBOX);
            }

            return false;
            };

        auto check_general = []() -> bool {
            std::unique_ptr<std::string> sys_vbox = util::sys_result("ioreg -l | grep -i -c -e \"virtualbox\" -e \"oracle\"");

            if (std::stoi(*sys_vbox) > 0) {
                return core::add(VBOX);
            }

            std::unique_ptr<std::string> sys_vmware = util::sys_result("ioreg -l | grep -i -c -e \"vmware\"");

            if (std::stoi(*sys_vmware) > 0) {
                return core::add(VMWARE);
            }

            return false;
            };

        auto check_rom = []() -> bool {
            std::unique_ptr<std::string> sys_rom = util::sys_result("system_profiler SPHardwareDataType | grep \"Boot ROM Version\"");
            const std::string rom = *sys_rom;

            if (util::find(rom, "VirtualBox")) {
                return core::add(VBOX);
            }

            return false;
            };

        return (
            check_usb() ||
            check_general() ||
            check_rom()
        );
#endif
    }
    catch (...) {
        debug("IOREG_GREP: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check if System Integrity Protection is disabled (likely a VM if it is)
     * @category MacOS
     * @link https://evasions.checkpoint.com/techniques/macos.html
     */
    [[nodiscard]] static bool mac_sip() try {
#if (!APPLE)
        return false;
#else
        if (core::disabled(MAC_SIP)) {
            return false;
        }

        std::unique_ptr<std::string> result = util::sys_result("csrutil status");
        const std::string tmp = *result;

        debug("MAC_SIP: ", "result = ", tmp);

        return (util::find(tmp, "disabled") || (!util::find(tmp, "enabled")));
#endif
    }
    catch (...) {
        debug("MAC_SIP: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Fetch HKLM registries for specific VM strings
     * @category Windows
     */
    [[nodiscard]] static bool hklm_registries() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(HKLM_REGISTRIES)) {
            return false;
        }

        u8 count = 0;

        auto check_key = [&count](const char* p_brand, const char* subKey, const char* valueName, const char* comp_string) {
            HKEY hKey;
            DWORD dwType = REG_SZ;
            char buffer[1024]{};
            DWORD bufferSize = sizeof(buffer);

            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                if (RegQueryValueExA(hKey, valueName, NULL, &dwType, reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
                    if (strcmp(buffer, comp_string) == 0) {
                        core::add(p_brand);
                        count++;
                    }
                }
                else {
                    debug("Failed to query value for \"", subKey, "\"");
                }

                RegCloseKey(hKey);
            }
            else {
                debug("Failed to open registry key for \"", subKey, "\"");
            }
        };

        check_key(BOCHS, "HARDWARE\\Description\\System", "SystemBiosVersion", "BOCHS");
        check_key(BOCHS, "HARDWARE\\Description\\System", "VideoBiosVersion", "BOCHS");

        check_key(ANUBIS, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductID", "76487-337-8429955-22614");
        check_key(ANUBIS, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductID", "76487-337-8429955-22614");

        check_key(CWSANDBOX, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductID", "76487-644-3177037-23510");
        check_key(CWSANDBOX, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductID", "76487-644-3177037-23510");

        check_key(JOEBOX, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductID", "55274-640-2673064-23950");
        check_key(JOEBOX, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductID", "55274-640-2673064-23950");

        check_key(PARALLELS, "HARDWARE\\Description\\System", "SystemBiosVersion", "PARALLELS");
        check_key(PARALLELS, "HARDWARE\\Description\\System", "VideoBiosVersion", "PARALLELS");

        check_key(QEMU, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "QEMU");
        check_key(QEMU, "HARDWARE\\Description\\System", "SystemBiosVersion", "QEMU");
        check_key(QEMU, "HARDWARE\\Description\\System", "VideoBiosVersion", "QEMU");
        check_key(QEMU, "HARDWARE\\Description\\System\\BIOS", "SystemManufacturer", "QEMU");

        check_key(VBOX, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VBOX");
        check_key(VBOX, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VBOX");
        check_key(VBOX, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 2\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VBOX");
        check_key(VBOX, "HARDWARE\\Description\\System", "SystemBiosVersion", "VBOX");
        check_key(VBOX, "HARDWARE\\Description\\System", "VideoBiosVersion", "VIRTUALBOX");
        check_key(VBOX, "HARDWARE\\Description\\System\\BIOS", "SystemProductName", "VIRTUAL");
        check_key(VBOX, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "DeviceDesc", "VBOX");
        check_key(VBOX, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "FriendlyName", "VBOX");
        check_key(VBOX, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "DeviceDesc", "VBOX");
        check_key(VBOX, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "FriendlyName", "VBOX");
        check_key(VBOX, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "DeviceDesc", "VBOX");
        check_key(VBOX, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "FriendlyName", "VBOX");
        check_key(VBOX, "SYSTEM\\CurrentControlSet\\Control\\SystemInformation", "SystemProductName", "VIRTUAL");
        check_key(VBOX, "SYSTEM\\CurrentControlSet\\Control\\SystemInformation", "SystemProductName", "VIRTUALBOX");

        check_key(VMWARE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE");
        check_key(VMWARE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE");
        check_key(VMWARE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 2\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", "Identifier", "VMWARE");
        check_key(VMWARE, "HARDWARE\\Description\\System", "SystemBiosVersion", "VMWARE");
        check_key(VMWARE, "HARDWARE\\Description\\System", "SystemBiosVersion", "INTEL - 6040000");
        check_key(VMWARE, "HARDWARE\\Description\\System", "VideoBiosVersion", "VMWARE");
        check_key(VMWARE, "HARDWARE\\Description\\System\\BIOS", "SystemProductName", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "0", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "1", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "DeviceDesc", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Services\\Disk\\Enum", "FriendlyName", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "DeviceDesc", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet002\\Services\\Disk\\Enum", "FriendlyName", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "DeviceDesc", "VMware");
        check_key(VMWARE, "SYSTEM\\ControlSet003\\Services\\Disk\\Enum", "FriendlyName", "VMware");
        //check_key(HKCR\Installer\Products 	ProductName 	vmware tools
        //check_key(HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall 	DisplayName 	vmware tools
        check_key(VMWARE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", "DisplayName", "vmware tools");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "CoInstallers32", "*vmx*");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "DriverDesc", "VMware*");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "InfSection", "vmx*");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000", "ProviderName", "VMware*");
        check_key(VMWARE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000\\Settings", "Device Description", "VMware*");
        check_key(VMWARE, "SYSTEM\\CurrentControlSet\\Control\\SystemInformation", "SystemProductName", "VMWARE");
        check_key(VMWARE, "SYSTEM\\CurrentControlSet\\Control\\Video\\{GUID}\\Video", "Service", "vm3dmp");
        check_key(VMWARE, "SYSTEM\\CurrentControlSet\\Control\\Video\\{GUID}\\Video", "Service", "vmx_svga");
        check_key(VMWARE, "SYSTEM\\CurrentControlSet\\Control\\Video\\{GUID}\\0000", "Device Description", "VMware SVGA*");

        check_key(XEN, "HARDWARE\\Description\\System\\BIOS", "SystemProductName", "Xen");

        return (count > 0);
#endif
    }
    catch (...) {
        debug("KHLM_REGISTRIES: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for qemu-ga process
     * @category Linux
     */
    [[nodiscard]] static bool qemu_ga() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(QEMU_GA)) {
            return false;
        }

        constexpr const char* process = "qemu-ga";

        if (util::is_proc_running(process)) {
            return core::add(QEMU);
        }

        return false;
#endif
    }
    catch (...) {
        debug("QEMU_GA: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief check for valid MSR value
     * @category Windows
     * @author LukeGoule
     * @link https://github.com/LukeGoule/compact_vm_detector/tree/main
     * @copyright MIT
     */
    [[nodiscard]] static bool valid_msr() {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VALID_MSR)) {
            return false;
        }

        __try
        {
            __readmsr(cpu::leaf::hypervisor);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }

        return true;
#endif
    }


    /**
     * @brief Check for QEMU processes
     * @category Windows
     */
    [[nodiscard]] static bool qemu_processes() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(QEMU_PROC)) {
            return false;
        }

        constexpr std::array<const TCHAR*, 3> qemu_proc_strings = { {
            _T("qemu-ga.exe"),
            _T("vdagent.exe"),
            _T("vdservice.exe")
        } };

        for (const auto str : qemu_proc_strings) {
            if (util::is_proc_running(str)) {
                return core::add(QEMU);
            }
        }

        return false;
#endif
    }
    catch (...) {
        debug("QEMU_PROC: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for VPC processes
     * @category Windows
     */
    [[nodiscard]] static bool vpc_proc() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VPC_PROC)) {
            return false;
        }

        constexpr std::array<const TCHAR*, 2> vpc_proc_strings = { {
            _T("VMSrvc.exe"),
            _T("VMUSrvc.exe")
        } };

        for (const auto str : vpc_proc_strings) {
            if (util::is_proc_running(str)) {
                return core::add(VPC);
            }
        }

        return false;
#endif
    }
    catch (...) {
        debug("VPC_PROC: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for official VPC method
     * @category Windows, x86
     */
    [[nodiscard]] static bool vpc_invalid() {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(VPC_INVALID)) {
            return false;
        }

        bool rc = false;

        auto IsInsideVPC_exceptionFilter = [](PEXCEPTION_POINTERS ep) -> DWORD {
            PCONTEXT ctx = ep->ContextRecord;

            ctx->Ebx = static_cast<DWORD>(-1); // Not running VPC
            ctx->Eip += 4; // skip past the "call VPC" opcodes
            return static_cast<DWORD>(EXCEPTION_CONTINUE_EXECUTION);
            // we can safely resume execution since we skipped faulty instruction
        };

        __try {
            __asm {
                push eax
                push ebx
                push ecx
                push edx

                mov ebx, 0h
                mov eax, 01h

                __emit 0Fh
                __emit 3Fh
                __emit 07h
                __emit 0Bh

                test ebx, ebx
                setz[rc]

                pop edx
                pop ecx
                pop ebx
                pop eax
            }
        }
        __except (IsInsideVPC_exceptionFilter(GetExceptionInformation())) {
            rc = false;
        }

        return rc;
#else
        return false;
#endif
    }


    /**
     * @brief Check for sidt method
     * @category Linux, Windows, x86
     */
    [[nodiscard]] static bool sidt() try {
        if (core::disabled(SIDT)) {
            return false;
        }

        // gcc/g++ causes a stack smashing error at runtime for some reason
        if (GCC) {
            return false;
        }

        u8 idtr[10]{};
        u32 idt_entry = 0;

#if (MSVC)
#if (x86_32)
        _asm sidt idtr
#elif (x86)
#pragma pack(1)
        struct IDTR {
            u16 limit;
            u64 base;
        };
#pragma pack()

        IDTR idtrStruct;
        __sidt(&idtrStruct);
        std::memcpy(idtr, &idtrStruct, sizeof(IDTR));
#else
        return false;
#endif

        idt_entry = *reinterpret_cast<unsigned long*>(&idtr[2]);
#elif (LINUX)
        // false positive with root for some reason
        if (util::is_admin()) {
            return false;
        }

        struct IDTR {
            u16 limit;
            u32 base;
        } __attribute__((packed));

        IDTR idtr_struct;

        __asm__ __volatile__(
            "sidt %0"
            : "=m" (idtr_struct)
        );

        std::ifstream mem("/dev/mem", std::ios::binary);
        mem.seekg(idtr_struct.base + 8, std::ios::beg);
        mem.read(reinterpret_cast<char*>(&idt_entry), sizeof(idt_entry));
        mem.close();
        UNUSED(idtr);
#else
        UNUSED(idtr);
        UNUSED(idt_entry);
        return false;
#endif

        if ((idt_entry >> 24) == 0xFF) {
            return core::add(VMWARE);
        }

        return false;
    }
    catch (...) {
        debug("SIDT: ", "caught error, returned false");
        return false;
    }


    /**
     * @brief Check for sldt
     * @category Windows, x86
     */
    [[nodiscard]] static bool sldt() try {
#if (x86_32 && MSVC)
        if (core::disabled(SLDT)) {
            return false;
        }

        unsigned short ldtr[5] = { 0xEF, 0xBE, 0xAD, 0xDE };
        unsigned int ldt = 0;

        _asm sldt ldtr;
        ldt = *((u32*)&ldtr[0]);

        return (ldt != 0xDEAD0000);
#else
        return false;
#endif
    }
    catch (...) {
        debug("SLDT: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for sgdt
     * @category Windows, x86
     */
    [[nodiscard]] static bool sgdt() try {
#if (x86_32 && MSVC)
        if (core::disabled(SGDT)) {
            return false;
        }

        u8 gdtr[6]{};
        u32 gdt = 0;

        _asm sgdt gdtr
        gdt = *((unsigned long*)&gdtr[2]);

        return ((gdt >> 24) == 0xFF);
#else
        return false;
#endif
    }
    catch (...) {
        debug("SGDT: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Go through the motherboard and match for Hyper-V string
     * @category Windows
     */
    [[nodiscard]] static bool hyperv_board() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(HYPERV_BOARD)) {
            return false;
        }

        HRESULT hres;

        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to initialize COM library. Error code: ", hres);
            return false;
        }

        hres = CoInitializeSecurity(
            NULL,
            -1,                          // use the default authentication service
            NULL,                        // use the default authorization service
            NULL,                        // reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // authentication
            RPC_C_IMP_LEVEL_IMPERSONATE, // impersonation
            NULL,                        // authentication info
            EOAC_NONE,                   // additional capabilities
            NULL                         // reserved
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to initialize security. Error code: ", hres);
            CoUninitialize();
            return false;
        }

        IWbemLocator* pLoc = NULL;
        IWbemServices* pSvc = NULL;

        hres = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            (LPVOID*)&pLoc
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to create IWbemLocator object. Error code: ", hres);
            CoUninitialize();
            return false;
        }

        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), // Namespace
            NULL,                    // User name
            NULL,                    // User password
            0,                       // Locale
            NULL,                    // Security flags
            0,                       // Authority
            0,                       // Context object pointer
            &pSvc
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to connect to WMI. Error code: ", hres);
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        hres = CoSetProxyBlanket(
            pSvc,                        // Indicates the proxy to set
            RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
            RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
            NULL,                        // Server principal name
            RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
            NULL,                        // client identity
            EOAC_NONE                    // proxy capabilities
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Failed to set proxy blanket. Error code: ", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        IEnumWbemClassObject* enumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_BaseBoard"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &enumerator
        );

        if (FAILED(hres)) {
            debug("VPC_BOARD: Query for Win32_BaseBoard failed. Error code: ", hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        bool is_vm = false;

        while (enumerator) {
            HRESULT hr = enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0) {
                break;
            }

            VARIANT vtProp;
            VariantInit(&vtProp);
            hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);

            if (SUCCEEDED(hr)) {
                if (vtProp.vt == VT_BSTR && _wcsicmp(vtProp.bstrVal, L"Microsoft Corporation Virtual Machine") == 0) {
                    is_vm = true;
                    VariantClear(&vtProp);
                    break;
                }

                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }

        enumerator->Release();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();

        if (is_vm) {
            return core::add(HYPERV);
        }

        return false;
#endif
    }
    catch (...) {
        debug("HYPERV_BOARD:", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for offensive security sidt method
     * @category Windows, x86
     * @author Danny Quist (chamuco@gmail.com)
     * @author Val Smith (mvalsmith@metasploit.com)
     * @note code documentation paper in /papers/www.offensivecomputing.net_vm.pdf
     */
    [[nodiscard]] static bool offsec_sidt() try {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(OFFSEC_SIDT)) {
            return false;
        }

        unsigned char m[6]{};
        __asm sidt m;

        return (m[5] > 0xD0);
#else
        return false;
#endif
    }
    catch (...) {
        debug("OFFSEC_SIDT: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for offensive security sgdt method
     * @category Windows, x86
     * @author Danny Quist (chamuco@gmail.com)
     * @author Val Smith (mvalsmith@metasploit.com)
     * @note code documentation paper in /papers/www.offensivecomputing.net_vm.pdf
     */
    [[nodiscard]] static bool offsec_sgdt() try {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(OFFSEC_SGDT)) {
            return false;
        }

        unsigned char m[6]{};
        __asm sgdt m;

        return (m[5] > 0xD0);
#else
        return false;
#endif
    }
    catch (...) {
        debug("OFFSEC_SGDT: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for Offensive Security sldt method
     * @category Windows, x86
     * @author Danny Quist (chamuco@gmail.com)
     * @author Val Smith (mvalsmith@metasploit.com)
     * @note code documentation paper in /papers/www.offensivecomputing.net_vm.pdf
     */
    [[nodiscard]] static bool offsec_sldt() try {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(OFFSEC_SLDT)) {
            return false;
        }

        unsigned short m[6]{};
        __asm sldt m;

        return (m[0] != 0x00 && m[1] != 0x00);
#else
        return false;
#endif
    }
    catch (...) {
        debug("OFFSEC_SLDT: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Check for sidt method with VPC's 0xE8XXXXXX range
     * @category Windows, x86
     * @note Idea from Tom Liston and Ed Skoudis' paper "On the Cutting Edge: Thwarting Virtual Machine Detection"
     * @note Paper situated at /papers/ThwartingVMDetection_Liston_Skoudis.pdf
     */
    [[nodiscard]] static bool vpc_sidt() try {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(VPC_SIDT)) {
            return false;
        }

        u8	idtr[6]{};
        u32	idt = 0;

        _asm sidt idtr
        idt = *((unsigned long*)&idtr[2]);

        if ((idt >> 24) == 0xE8) {
            return core::add(VPC);
        }

        return false;
#else
        return false;
#endif
    }
    catch (...) {
        debug("VPC_SIDT: ", "catched error, returned false");
        return false;
    }


    /**
     * @brief Find for VPC and Parallels specific VM files
     * @category Windows
     */
    [[nodiscard]] static bool vm_files_extra() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(VM_FILES_EXTRA)) {
            return false;
        }

        constexpr std::array<std::pair<const char*, const char*>, 9> files = { {
            { VPC, "c:\\windows\\system32\\drivers\\vmsrvc.sys" },
            { VPC, "c:\\windows\\system32\\drivers\\vpc-s3.sys" },
            { PARALLELS, "c:\\windows\\system32\\drivers\\prleth.sys" },
            { PARALLELS, "c:\\windows\\system32\\drivers\\prlfs.sys" },
            { PARALLELS, "c:\\windows\\system32\\drivers\\prlmouse.sys" },
            { PARALLELS, "c:\\windows\\system32\\drivers\\prlvideo.sys" },
            { PARALLELS, "c:\\windows\\system32\\drivers\\prltime.sys" },
            { PARALLELS, "c:\\windows\\system32\\drivers\\prl_pv32.sys" },
            { PARALLELS, "c:\\windows\\system32\\drivers\\prl_paravirt_32.sys" }
        } };

        for (const auto &file_pair : files) {
            if (util::exists(file_pair.second)) {
                return core::add(file_pair.first);
            }
        }
        
        return false;
#endif
    }
    catch (...) {
        debug("VM_FILES_EXTRA: catched error, returned false");
        return false;
    }


    /**
     * @brief Find for VMware string in /proc/iomem
     * @category Linux
     * @note idea from ScoopyNG by Tobias Klein
     */
    [[nodiscard]] static bool vmware_iomem() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(VMWARE_IOMEM)) {
            return false;
        }

        const std::string iomem_file = util::read_file("/proc/iomem");

        if (util::find(iomem_file, "VMware")) {
            return core::add(VMWARE);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VMWARE_IOMEM: catched error, returned false");
        return false;
    }


    /**
     * @brief Find for VMware string in /proc/ioports
     * @category Windows
     * @note idea from ScoopyNG by Tobias Klein
     */
    [[nodiscard]] static bool vmware_ioports() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(VMWARE_IOPORTS)) {
            return false;
        }

        const std::string ioports_file = util::read_file("/proc/ioports");

        if (util::find(ioports_file, "VMware")) {
            return core::add(VMWARE);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VMWARE_IOPORTS: catched error, returned false");
        return false;
    }


    /**
     * @brief Find for VMware string in /proc/scsi/scsi
     * @category Windows
     * @note idea from ScoopyNG by Tobias Klein
     */
    [[nodiscard]] static bool vmware_scsi() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(VMWARE_SCSI)) {
            return false;
        }

        const std::string scsi_file = util::read_file("/proc/scsi/scsi");

        if (util::find(scsi_file, "VMware")) {
            return core::add(VMWARE);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VMWARE_SCSI: catched error, returned false");
        return false;
    }


    /**
     * @brief Find for VMware-specific device name in dmesg output
     * @category Windows
     * @note idea from ScoopyNG by Tobias Klein
     */
    [[nodiscard]] static bool vmware_dmesg() try {
#if (!LINUX)
        return false;
#else
        if (core::disabled(VMWARE_DMESG)) {
            return false;
        }

        if (!util::is_admin()) {
            return false;
        }

        auto dmesg_output = util::sys_result("dmesg");
        const std::string dmesg = *dmesg_output;

        if (dmesg.empty()) {
            return false;
        }

        if (util::find(dmesg, "BusLogic BT-958")) {
            return core::add(VMWARE);
        }

        if (util::find(dmesg, "pcnet32")) {
            return core::add(VMWARE);
        }

        return false;
#endif
    }
    catch (...) {
        debug("VMWARE_DMESG: catched error, returned false");
        return false;
    }


    /**
     * @brief Check using str assembly instruction
     * @note Alfredo Omella's (S21sec) STR technique
     * @note paper describing this technique is located at /papers/www.s21sec.com_vmware-eng.pdf (2006)
     * @category Windows
     */ 
    [[nodiscard]] static bool vmware_str() try {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(VMWARE_STR)) {
            return false;
        }

        unsigned short mem[4] = {0, 0, 0, 0};

        __asm str mem;

        if ((mem[0] == 0x00) && (mem[1] == 0x40)) {
            return core::add(VMWARE);
        }
#else
        return false;
#endif
    }
    catch (...) {
        debug("VMWARE_STR: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for official VMware io port backdoor technique
     * @category Windows, x86
     * @note Code from ScoopyNG by Tobias Klein
     * @note Technique founded by Ken Kato
     * @copyright BSD clause 2
     */ 
    [[nodiscard]] static bool vmware_backdoor() {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(VMWARE_BACKDOOR)) {
            return false;
        }

        u32 a = 0;
        u32 b = 0;

        constexpr std::array<i16, 2> ioports = { 'VX' , 'VY' };
        i16 ioport;
        bool is_vm = false;

        for (u8 i = 0; i < ioports.size(); ++i) {
            ioport = ioports[i];
            for (u8 cmd = 0; cmd < 0x2c; ++cmd) {
                __try {
                    __asm {
                        push eax
                        push ebx
                        push ecx
                        push edx
        
                        mov eax, 'VMXh'
                        movzx ecx, cmd
                        mov dx, ioport
                        in eax, dx      // <- key point is here

                        mov a, ebx
                        mov b, ecx

                        pop edx
                        pop ecx
                        pop ebx
                        pop eax
                    }

                    is_vm = true;
                    break;
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        }

        if (is_vm) {
            switch (b) {
                case 1:  return core::add(VMWARE_EXPRESS);
                case 2:  return core::add(VMWARE_ESX);
                case 3:  return core::add(VMWARE_GSX);
                case 4:  return core::add(VMWARE_WORKSTATION);
                default: return core::add(VMWARE);
            }
        }
    
        return false;
#else
        return false;
#endif
    }


    /**
     * @brief Check for VMware memory using IO port backdoor
     * @category Windows, x86
     * @note Code from ScoopyNG by Tobias Klein
     * @copyright BSD clause 2
     */ 
    [[nodiscard]] static bool vmware_port_memory() {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(VMWARE_PORT_MEM)) {
            return false;
        }

        unsigned int a = 0;

        __try {
            __asm {
                push eax
                push ebx
                push ecx
                push edx
                
                mov eax, 'VMXh'
                mov ecx, 14h
                mov dx, 'VX'
                in eax, dx
                mov a, eax 

                pop edx
                pop ecx
                pop ebx
                pop eax
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}

        if (a > 0) {
            return core::add(VMWARE);
        }

        return false;
#else
        return false;
#endif
    }


    /**
     * @brief Check for SMSW assembly instruction technique
     * @category Windows, x86
     * @author Danny Quist from Offensive Computing
     */ 
    [[nodiscard]] static bool smsw() try {
#if (!MSVC || !x86)
        return false;
#elif (x86_32)
        if (core::disabled(SMSW)) {
            return false;
        }

        unsigned int reax = 0;

        __asm
        {
            mov eax, 0xCCCCCCCC;
            smsw eax;
            mov DWORD PTR [reax], eax;
        }

        return (
            (((reax >> 24) & 0xFF) == 0xCC) && 
            (((reax >> 16) & 0xFF) == 0xCC)
        );
#else
        return false;
#endif
    }
    catch (...) {
        debug("SMSW: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for mutex strings of VM brands
     * @category Windows, x86
     * @note from VMDE project 
     * @author hfiref0x
     * @copyright MIT
     */ 
    [[nodiscard]] static bool mutex() try {
#if (!MSVC)
        return false;
#else
        if (core::disabled(MUTEX)) {
            return false;
        }

        auto supMutexExist = [](const char* lpMutexName) -> bool {
            DWORD dwError;
            HANDLE hObject = NULL;
            if (lpMutexName == NULL) {
                return false;
            }

            SetLastError(0);
            hObject = CreateMutexA(NULL, FALSE, lpMutexName);
            dwError = GetLastError();

            if (hObject) {
                CloseHandle(hObject);
            }

            return (dwError == ERROR_ALREADY_EXISTS);
        };

        if (
            supMutexExist("Sandboxie_SingleInstanceMutex_Control") ||
            supMutexExist("SBIE_BOXED_ServiceInitComplete_Mutex1")
        ) { 
            return core::add(SANDBOXIE);
        }

        if (supMutexExist("MicrosoftVirtualPC7UserServiceMakeSureWe'reTheOnlyOneMutex")) {
            return core::add(VPC);
        }

        if (supMutexExist("Frz_State")) {
            return true;
        }

        return false;
#endif
    }
    catch (...) {
        debug("MUTEX: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for uptime of less than or equal to 2 minutes
     * @category Windows, Linux
     * @note https://stackoverflow.com/questions/30095439/how-do-i-get-system-up-time-in-milliseconds-in-c
     */ 
    [[nodiscard]] static bool uptime() try {
        if (core::disabled(UPTIME)) {
            return false;
        }

        constexpr u32 uptime_ms = 1000 * 60 * 2;
        constexpr u32 uptime_s = 60 * 2;

#if (MSVC)
        UNUSED(uptime_s);
        return (GetTickCount64() <= uptime_ms);
#elif (LINUX)
        UNUSED(uptime_ms);
        struct sysinfo info;

        if (sysinfo(&info) != 0) {
            debug("UPTIME: sysinfo failed");
            return false;
        }

        return (info.uptime < uptime_s);
#elif (APPLE)
        UNUSED(uptime_s);
        std::chrono::milliseconds uptime(0u);

        struct timeval ts;
        std::size_t len = sizeof(ts);

        int mib[2] = { CTL_KERN, KERN_BOOTTIME };

        if (sysctl(mib, 2, &ts, &len, NULL, 0) != 0) {
            return false;
        }

        uptime = std::chrono::milliseconds(
            (static_cast<u64>(ts.tv_sec) * 1000ULL) + 
            (static_cast<u64>(ts.tv_usec) / 1000ULL)
        );

        return (uptime < uptime_ms);
#else
        return false;
#endif
    }
    catch (...) {
        debug("UPTIME: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for odd CPU threads, usually a sign of modification through VM setting because 99% of CPUs have even numbers of threads
     * @category All, x86
     */ 
    [[nodiscard]] static bool odd_cpu_threads() try {
#if (!x86)
        return false;
#else
        if (core::disabled(ODD_CPU_THREADS)) {
            return false;
        }

        const u32 threads = std::thread::hardware_concurrency();

        struct stepping_struct {
            u8 model;
            u8 family;
            u8 extmodel;
        };

        struct stepping_struct steps {};

        u32 unused, eax = 0;
        cpu::cpuid(eax, unused, unused, unused, 1);
        UNUSED(unused);

        steps.model    = ((eax >> 4)  & 0b1111);
        steps.family   = ((eax >> 8)  & 0b1111);
        steps.extmodel = ((eax >> 16) & 0b1111);
        
        debug("ODD_CPU_THREADS: model    = ", static_cast<u32>(steps.model));
        debug("ODD_CPU_THREADS: family   = ", static_cast<u32>(steps.family));
        debug("ODD_CPU_THREADS: extmodel = ", static_cast<u32>(steps.extmodel));

        // check if the CPU is an intel celeron
        auto is_celeron = [&steps]() -> bool {
            if (!cpu::is_intel()) {
                return false;
            }

            constexpr u8 celeron_family   = 0x6;
            constexpr u8 celeron_extmodel = 0x2;
            constexpr u8 celeron_model    = 0xA;

            return (
                steps.family   == celeron_family   &&
                steps.extmodel == celeron_extmodel && 
                steps.model    == celeron_model
            );
        };

        // check if the microarchitecture was made before 2006, which was around the time multi-core processors were implemented
        auto old_microarchitecture = [&steps]() -> bool {
            constexpr std::array<std::array<u8, 3>, 32> old_archs = {{
                // 80486
                {{ 0x4, 0x0, 0x1 }},
                {{ 0x4, 0x0, 0x2 }},
                {{ 0x4, 0x0, 0x3 }},
                {{ 0x4, 0x0, 0x4 }},
                {{ 0x4, 0x0, 0x5 }},
                {{ 0x4, 0x0, 0x7 }},
                {{ 0x4, 0x0, 0x8 }},
                {{ 0x4, 0x0, 0x9 }},

                // P5
                {{ 0x5, 0x0, 0x1 }},
                {{ 0x5, 0x0, 0x2 }},
                {{ 0x5, 0x0, 0x4 }},
                {{ 0x5, 0x0, 0x7 }},
                {{ 0x5, 0x0, 0x8 }},

                // P6
                {{ 0x6, 0x0, 0x1 }},
                {{ 0x6, 0x0, 0x3 }},
                {{ 0x6, 0x0, 0x5 }},
                {{ 0x6, 0x0, 0x6 }},
                {{ 0x6, 0x0, 0x7 }},
                {{ 0x6, 0x0, 0x8 }},
                {{ 0x6, 0x0, 0xA }},
                {{ 0x6, 0x0, 0xB }},

                // Netburst
                {{ 0xF, 0x0, 0x6 }},
                {{ 0xF, 0x0, 0x4 }},
                {{ 0xF, 0x0, 0x3 }},
                {{ 0xF, 0x0, 0x2 }},
                {{ 0xF, 0x0, 0x10 }},

                {{ 0x6, 0x1, 0x5 }}, // Pentium M (Talopai)
                {{ 0x6, 0x1, 0x6 }}, // Core (Client)
                {{ 0x6, 0x0, 0x9 }}, // Pentium M
                {{ 0x6, 0x0, 0xD }}, // Pentium M
                {{ 0x6, 0x0, 0xE }}, // Modified Pentium M
                {{ 0x6, 0x0, 0xF }}  // Core (Client)
            }};

            constexpr u8 FAMILY   = 0;
            constexpr u8 EXTMODEL = 1;
            constexpr u8 MODEL    = 2;

            for (const auto& arch : old_archs) {
                if (
                    steps.family   == arch.at(FAMILY)   &&
                    steps.extmodel == arch.at(EXTMODEL) &&
                    steps.model    == arch.at(MODEL)
                ) {
                    return true;
                }
            }

            return false;
        };

        // self-explanatory
        if (!(cpu::is_intel() || cpu::is_amd())) {
            return false;
        }

        // intel celeron CPUs are relatively modern, but they can contain a single or odd thread count
        if (is_celeron()) {
            return false;
        }

        // CPUs before 2006 had no official multi-core processors
        if (old_microarchitecture()) {
            return false;
        }

        // is the count odd?
        return (threads & 1);
#endif
    }
    catch (...) {
        debug("ODD_CPU_THREADS: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for CPUs that don't match their thread count
     * @category All, x86
     * @link https://en.wikipedia.org/wiki/List_of_Intel_Core_processors
     */ 
    [[nodiscard]] static bool intel_thread_mismatch() try {
#if (!x86)
        return false;
#else
        if (core::disabled(INTEL_THREAD_MISMATCH)) {
            return false;
        }
    
        if (!cpu::is_intel()) {
            return false;
        }

        if (cpu::has_hyperthreading()) {
            return false;
        }

        const cpu::model_struct model = cpu::get_model();

        if (!model.found) {
            return false;
        }

        if (!model.is_i_series) {
            return false;
        }

        debug("INTEL_THREAD_MISMATCH: CPU model = ", model.string);

        auto thread_database = std::make_unique<std::unordered_map<std::string, u8>>();

        #define push thread_database->emplace

        // i3 series
        push("i3-1000G1", 4);
        push("i3-1000G4", 4);
        push("i3-1000NG4", 4);
        push("i3-1005G1", 4);
        push("i3-10100", 8);
        push("i3-10100E", 8);
        push("i3-10100F", 8);
        push("i3-10100T", 8);
        push("i3-10100TE", 8);
        push("i3-10100Y", 4);
        push("i3-10105", 8);
        push("i3-10105F", 8);
        push("i3-10105T", 8);
        push("i3-10110U", 4);
        push("i3-10110Y", 4);
        push("i3-10300", 8);
        push("i3-10300T", 8);
        push("i3-10305", 8);
        push("i3-10305T", 8);
        push("i3-10320", 8);
        push("i3-10325", 8);
        push("i3-11100B", 8);
        push("i3-11100HE", 8);
        push("i3-1110G4", 4 );
        push("i3-1115G4E", 4);
        push("i3-1115GRE", 4);
        push("i3-1120G4", 8);
        push("i3-12100", 8);
        push("i3-12100F", 8);
        push("i3-12100T", 8);
        push("i3-1210U", 4);
        push("i3-1215U", 4);
        push("i3-1215UE", 4);
        push("i3-1215UL", 4);
        push("i3-12300", 8);
        push("i3-12300T", 8);
        push("i3-13100", 8);
        push("i3-13100F", 8 );
        push("i3-13100T", 8 );
        push("i3-1315U", 4);
        push("i3-1315UE", 4);
        push("i3-14100", 8);
        push("i3-14100F", 8);
        push("i3-14100T", 8);
        push("i3-2100", 4);
        push("i3-2100T", 4);
        push("i3-2102", 4);
        push("i3-2105", 4);
        push("i3-2120", 4);
        push("i3-2120T", 4);
        push("i3-2125", 4);
        push("i3-2130", 4);
        push("i3-2308M", 4);
        push("i3-2310E", 4);
        push("i3-2310M", 4);
        push("i3-2312M", 4);
        push("i3-2328M", 4);
        push("i3-2330E", 4);
        push("i3-2330M", 4);
        push("i3-2332M", 4);
        push("i3-2340UE", 4);
        push("i3-2348M", 4);
        push("i3-2350LM", 4);
        push("i3-2350M", 4);
        push("i3-2355M", 4);
        push("i3-2357M", 4);
        push("i3-2365M", 4);
        push("i3-2367M", 4);
        push("i3-2370LM", 4);
        push("i3-2370M", 4);
        push("i3-2375M", 4);
        push("i3-2377M", 4);
        push("i3-2390M", 4);
        push("i3-2393M", 4);
        push("i3-2394M", 4);
        push("i3-2395M", 4);
        push("i3-2397M", 4);
        push("i3-3110M", 4);
        push("i3-3115C", 4);
        push("i3-3120M", 4);
        push("i3-3120ME", 4);
        push("i3-3130M", 4);
        push("i3-3210", 4);
        push("i3-3217U", 4);
        push("i3-3217UE", 4);
        push("i3-3220", 4);
        push("i3-3220T", 4);
        push("i3-3225", 4);
        push("i3-3227U", 4);
        push("i3-3229Y", 4);
        push("i3-3240", 4);
        push("i3-3240T", 4);
        push("i3-3245", 4);
        push("i3-3250", 4);
        push("i3-3250T", 4);
        push("i3-330E", 4);
        push("i3-330M", 4);
        push("i3-330UM", 4);
        push("i3-350M", 4);
        push("i3-370M", 4);
        push("i3-380M", 4);
        push("i3-380UM", 4);
        push("i3-390M", 4);
        push("i3-4000M", 4);
        push("i3-4005U", 4);
        push("i3-4010M", 4);
        push("i3-4010U", 4);
        push("i3-4010Y", 4);
        push("i3-4012Y", 4);
        push("i3-4020Y", 4);
        push("i3-4025U", 4);
        push("i3-4030U", 4);
        push("i3-4030Y", 4);
        push("i3-4100E", 4);
        push("i3-4100M", 4);
        push("i3-4100U", 4);
        push("i3-4102E", 4);
        push("i3-4110E", 4);
        push("i3-4110M", 4);
        push("i3-4112E", 4);
        push("i3-4120U", 4);
        push("i3-4130", 4);
        push("i3-4130T", 4);
        push("i3-4150", 4);
        push("i3-4150T", 4);
        push("i3-4158U", 4);
        push("i3-4160", 4);
        push("i3-4160T", 4);
        push("i3-4170", 4);
        push("i3-4170T", 4);
        push("i3-4330", 4);
        push("i3-4330T", 4);
        push("i3-4330TE", 4);
        push("i3-4340", 4);
        push("i3-4340TE", 4);
        push("i3-4350", 4);
        push("i3-4350T", 4);
        push("i3-4360", 4);
        push("i3-4360T", 4);
        push("i3-4370", 4);
        push("i3-4370T", 4);
        push("i3-5005U", 4);
        push("i3-5010U", 4);
        push("i3-5015U", 4);
        push("i3-5020U", 4);
        push("i3-5157U", 4);
        push("i3-530", 4);
        push("i3-540", 4);
        push("i3-550", 4);
        push("i3-560", 4);
        push("i3-6006U", 4);
        push("i3-6098P", 4);
        push("i3-6100", 4);
        push("i3-6100E", 4);
        push("i3-6100H", 4);
        push("i3-6100T", 4);
        push("i3-6100TE", 4);
        push("i3-6100U", 4);
        push("i3-6102E", 4);
        push("i3-6120T", 4);
        push("i3-6157U", 4);
        push("i3-6167U", 4);
        push("i3-6300", 4);
        push("i3-6300T", 4);
        push("i3-6320", 4);
        push("i3-6320T", 4);
        push("i3-7007U", 4);
        push("i3-7020U", 4);
        push("i3-7100", 4);
        push("i3-7100E", 4);
        push("i3-7100H", 4);
        push("i3-7100T", 4);
        push("i3-7100U", 4);
        push("i3-7101E", 4);
        push("i3-7101TE", 4);
        push("i3-7102E", 4);
        push("i3-7110U", 4);
        push("i3-7120", 4);
        push("i3-7120T", 4);
        push("i3-7130U", 4);
        push("i3-7167U", 4);
        push("i3-7300", 4);
        push("i3-7300T", 4);
        push("i3-7310T", 4);
        push("i3-7310U", 4);
        push("i3-7320", 4);
        push("i3-7320T", 4);
        push("i3-7340", 4);
        push("i3-7350K", 4);
        push("i3-8000", 4);
        push("i3-8000T", 4);
        push("i3-8020", 4);
        push("i3-8020T", 4);
        push("i3-8100", 4);
        push("i3-8100B", 4);
        push("i3-8100F", 4);
        push("i3-8100H", 4);
        push("i3-8100T", 4);
        push("i3-8109U", 4);
        push("i3-8120", 4);
        push("i3-8120T", 4);
        push("i3-8121U", 4);
        push("i3-8130U", 4);
        push("i3-8130U", 4);
        push("i3-8140U", 4);
        push("i3-8145U", 4);
        push("i3-8145UE", 4);
        push("i3-8300", 4);
        push("i3-8300T", 4);
        push("i3-8320", 4);
        push("i3-8320T", 4);
        push("i3-8350K", 4);
        push("i3-9100", 4);
        push("i3-9100E", 4);
        push("i3-9100F", 4);
        push("i3-9100HL", 4);
        push("i3-9100T", 4);
        push("i3-9100TE", 4);
        push("i3-9300", 4);
        push("i3-9300T", 4);
        push("i3-9320", 4);
        push("i3-9350K", 4);
        push("i3-9350KF", 4);
        push("i3-N300", 8);
        push("i3-N305", 8);

            // i5 series
        push("i5-10200H", 8);
        push("i5-10210U", 4);
        push("i5-10210Y", 8);
        push("i5-10300H", 8);
        push("i5-1030G4", 8);
        push("i5-1030G7", 8);
        push("i5-1030NG7", 8);
        push("i5-10310U", 4);
        push("i5-10310Y", 8);
        push("i5-1035G1", 8);
        push("i5-1035G4", 8);
        push("i5-1035G7", 8);
        push("i5-1038NG7", 8);
        push("i5-10400", 12);
        push("i5-10400F", 12);
        push("i5-10400H", 8);
        push("i5-10400T", 12);
        push("i5-10500", 12);
        push("i5-10500E", 12);
        push("i5-10500H", 12);
        push("i5-10500T", 12);
        push("i5-10500TE", 12);
        push("i5-10505", 12);
        push("i5-10600", 12);
        push("i5-10600K", 12);
        push("i5-10600KF", 12);
        push("i5-10600T", 12);
        push("i5-1115G4", 4);
        push("i5-1125G4", 8);
        push("i5-11260H", 12);
        push("i5-11300H", 8);
        push("i5-1130G7", 8);
        push("i5-11320H", 8);
        push("i5-1135G7", 8);
        push("i5-11400", 12);
        push("i5-11400F", 12);
        push("i5-11400H", 12);
        push("i5-11400T", 12);
        push("i5-1140G7", 8);
        push("i5-1145G7", 8);
        push("i5-1145G7E", 8);
        push("i5-1145GRE", 8);
        push("i5-11500", 12);
        push("i5-11500B", 12);
        push("i5-11500H", 12);
        push("i5-11500HE", 12);
        push("i5-11500T", 12);
        push("i5-1155G7", 8);
        push("i5-11600", 12);
        push("i5-11600K", 12);
        push("i5-11600KF", 12);
        push("i5-11600T", 12);
        push("i5-1230U", 4);
        push("i5-1235U", 4);
        push("i5-12400", 12 );
        push("i5-12400F", 12 );
        push("i5-12400T", 12);
        push("i5-1240P", 8);
        push("i5-1240U", 4);
        push("i5-1245U", 4);
        push("i5-12490F", 12);
        push("i5-12500", 12 );
        push("i5-12500H", 8);
        push("i5-12500HL", 8);
        push("i5-12500T", 12 );
        push("i5-1250P", 8);
        push("i5-1250PE", 8);
        push("i5-12600", 12);
        push("i5-12600H", 8);
        push("i5-12600HE", 8);
        push("i5-12600HL", 8);
        push("i5-12600HX", 8);
        push("i5-12600K", 12);
        push("i5-12600KF", 12);
        push("i5-12600T", 12 );
        push("i5-13400", 12);
        push("i5-13400F", 12);
        push("i5-13400T", 12);
        push("i5-1340P", 8);
        push("i5-1340PE", 8);
        push("i5-13490F", 12);
        push("i5-13500", 12);
        push("i5-13500H", 8 );
        push("i5-13500T", 12);
        push("i5-13505H", 8);
        push("i5-1350P", 8);
        push("i5-1350PE", 8);
        push("i5-13600", 12);
        push("i5-13600H", 8 );
        push("i5-13600HE", 8);
        push("i5-13600K", 12);
        push("i5-13600K", 20);
        push("i5-13600KF", 12 );
        push("i5-13600KF", 20);
        push("i5-13600T", 12);
        push("i5-2300", 4);
        push("i5-2310", 4);
        push("i5-2320", 4);
        push("i5-2380P", 4);
        push("i5-2390T", 4);
        push("i5-2400", 4);
        push("i5-2400S", 4);
        push("i5-2405S", 4);
        push("i5-2410M", 4);
        push("i5-2415M", 4);
        push("i5-2430M", 4);
        push("i5-2435M", 4);
        push("i5-2450M", 4);
        push("i5-2450P", 4);
        push("i5-2467M", 4);
        push("i5-2475M", 4);
        push("i5-2477M", 4);
        push("i5-2487M", 4);
        push("i5-2490M", 4);
        push("i5-2497M", 4);
        push("i5-2500", 4);
        push("i5-2500K", 4);
        push("i5-2500S", 4);
        push("i5-2500T", 4);
        push("i5-2510E", 4);
        push("i5-2515E", 4);
        push("i5-2520M", 4);
        push("i5-2537M", 4);
        push("i5-2540LM", 4);
        push("i5-2540M", 4);
        push("i5-2547M", 4);
        push("i5-2550K", 4);
        push("i5-2557M", 4);
        push("i5-2560LM", 4);
        push("i5-2560M", 4);
        push("i5-2580M", 4);
        push("i5-3210M", 4);
        push("i5-3230M", 4);
        push("i5-3317U", 4);
        push("i5-3320M", 4);
        push("i5-3330", 4);
        push("i5-3330S", 4);
        push("i5-3335S", 4);
        push("i5-3337U", 4);
        push("i5-3339Y", 4);
        push("i5-3340", 4);
        push("i5-3340M", 4);
        push("i5-3340S", 4);
        push("i5-3350P", 4);
        push("i5-3360M", 4);
        push("i5-3380M", 4);
        push("i5-3427U", 4);
        push("i5-3437U", 4);
        push("i5-3439Y", 4);
        push("i5-3450", 4);
        push("i5-3450S", 4);
        push("i5-3470", 4);
        push("i5-3470S", 4);
        push("i5-3470T", 4);
        push("i5-3475S", 4);
        push("i5-3550", 4);
        push("i5-3550S", 4);
        push("i5-3570", 4);
        push("i5-3570K", 4);
        push("i5-3570S", 4);
        push("i5-3570T", 4);
        push("i5-3610ME", 4);
        push("i5-4200H", 4);
        push("i5-4200M", 4);
        push("i5-4200U", 4);
        push("i5-4200Y", 4);
        push("i5-4202Y", 4);
        push("i5-4210H", 4);
        push("i5-4210M", 4);
        push("i5-4210U", 4);
        push("i5-4210Y", 4);
        push("i5-4220Y", 4);
        push("i5-4250U", 4);
        push("i5-4258U", 4);
        push("i5-4260U", 4);
        push("i5-4278U", 4);
        push("i5-4288U", 4);
        push("i5-4300M", 4);
        push("i5-4300U", 4);
        push("i5-4300Y", 4);
        push("i5-4302Y", 4);
        push("i5-4308U", 4);
        push("i5-430M", 4);
        push("i5-430UM", 4);
        push("i5-4310M", 4);
        push("i5-4310U", 4);
        push("i5-4330M", 4);
        push("i5-4340M", 4);
        push("i5-4350U", 4);
        push("i5-4360U", 4);
        push("i5-4400E", 4);
        push("i5-4402E", 4);
        push("i5-4402EC", 4);
        push("i5-4410E", 4);
        push("i5-4422E", 4);
        push("i5-4430", 4);
        push("i5-4430S", 4);
        push("i5-4440", 4);
        push("i5-4440S", 4);
        push("i5-4460", 4);
        push("i5-4460S", 4);
        push("i5-4460T", 4);
        push("i5-4470", 4);
        push("i5-450M", 4);
        push("i5-4570", 4);
        push("i5-4570R", 4);
        push("i5-4570S", 4);
        push("i5-4570T", 4);
        push("i5-4570TE", 4);
        push("i5-4590", 4);
        push("i5-4590S", 4);
        push("i5-4590T", 4);
        push("i5-460M", 4);
        push("i5-4670", 4);
        push("i5-4670K", 4);
        push("i5-4670R", 4);
        push("i5-4670S", 4);
        push("i5-4670T", 4);
        push("i5-4690", 4);
        push("i5-4690K", 4);
        push("i5-4690S", 4);
        push("i5-4690T", 4);
        push("i5-470UM", 4);
        push("i5-480M", 4);
        push("i5-5200U", 4);
        push("i5-520E", 4);
        push("i5-520M", 4);
        push("i5-520UM", 4);
        push("i5-5250U", 4);
        push("i5-5257U", 4);
        push("i5-5287U", 4);
        push("i5-5300U", 4);
        push("i5-5350H", 4 );
        push("i5-5350U", 4);
        push("i5-540M", 4);
        push("i5-540UM", 4);
        push("i5-5575R", 4);
        push("i5-560M", 4);
        push("i5-560UM", 4);
        push("i5-5675C", 4);
        push("i5-5675R", 4);
        push("i5-580M", 4);
        push("i5-6198DU", 4);
        push("i5-6200U", 4);
        push("i5-6260U", 4);
        push("i5-6267U", 4);
        push("i5-6287U", 4);
        push("i5-6300HQ", 4);
        push("i5-6300U", 4);
        push("i5-6350HQ", 4);
        push("i5-6360U", 4);
        push("i5-6400", 4);
        push("i5-6400T", 4);
        push("i5-6402P", 4);
        push("i5-6440EQ", 4);
        push("i5-6440HQ", 4);
        push("i5-6442EQ", 4);
        push("i5-650", 4);
        push("i5-6500", 4);
        push("i5-6500T", 4);
        push("i5-6500TE", 4);
        push("i5-655K", 4);
        push("i5-6585R", 4);
        push("i5-660", 4);
        push("i5-6600", 4);
        push("i5-6600K", 4);
        push("i5-6600T", 4);
        push("i5-661", 4);
        push("i5-6685R", 4);
        push("i5-670", 4);
        push("i5-680", 4);
        push("i5-7200U", 4);
        push("i5-7210U", 4);
        push("i5-7260U", 4);
        push("i5-7267U", 4);
        push("i5-7287U", 4);
        push("i5-7300HQ", 4);
        push("i5-7300U", 4);
        push("i5-7360U", 4);
        push("i5-7400", 4);
        push("i5-7400T", 4);
        push("i5-7440EQ", 4);
        push("i5-7440HQ", 4);
        push("i5-7442EQ", 4);
        push("i5-750", 4);
        push("i5-7500", 4);
        push("i5-7500T", 4);
        push("i5-750S", 4);
        push("i5-760", 4);
        push("i5-7600", 4);
        push("i5-7600K", 4);
        push("i5-7600T", 4);
        push("i5-7640X", 4 );
        push("i5-7Y54", 4);
        push("i5-7Y57", 4);
        push("i5-8200Y", 4);
        push("i5-8210Y", 4);
        push("i5-8250U", 8);
        push("i5-8257U", 8);
        push("i5-8259U", 8);
        push("i5-8260U", 8);
        push("i5-8265U", 8);
        push("i5-8269U", 8);
        push("i5-8279U", 8);
        push("i5-8300H", 8);
        push("i5-8305G", 8);
        push("i5-8310Y", 4);
        push("i5-8350U", 8);
        push("i5-8365U", 8);
        push("i5-8365UE", 8);
        push("i5-8400", 6);
        push("i5-8400B", 6);
        push("i5-8400H", 8);
        push("i5-8400T", 6);
        push("i5-8420", 6);
        push("i5-8420T", 6);
        push("i5-8500", 6);
        push("i5-8500B", 6);
        push("i5-8500T", 6);
        push("i5-8550", 6);
        push("i5-8600", 6);
        push("i5-8600K", 6);
        push("i5-8600T", 6);
        push("i5-8650", 6);
        push("i5-9300H", 8);
        push("i5-9300HF", 8);
        push("i5-9400", 6);
        push("i5-9400F", 6);
        push("i5-9400H", 8);
        push("i5-9400T", 6);
        push("i5-9500", 6);
        push("i5-9500E", 6);
        push("i5-9500F", 6);
        push("i5-9500T", 6);
        push("i5-9500TE", 6);
        push("i5-9600", 6);
        push("i5-9600K", 6);
        push("i5-9600KF", 6);
        push("i5-9600T", 6);

        // i7 series
        push("i7-10510U", 8);
        push("i7-10510Y", 8);
        push("i7-1060G7", 8);
        push("i7-10610U", 8);
        push("i7-1065G7", 8);
        push("i7-1068G7", 8);
        push("i7-1068NG7", 8);
        push("i7-10700", 16);
        push("i7-10700E", 16);
        push("i7-10700F", 16);
        push("i7-10700K", 16);
        push("i7-10700KF", 16);
        push("i7-10700T", 16);
        push("i7-10700TE", 16);
        push("i7-10710U", 8);
        push("i7-10750H", 12);
        push("i7-10810U", 12);
        push("i7-10850H", 12);
        push("i7-10870H", 16);
        push("i7-10875H", 16);
        push("i7-11370H", 8);
        push("i7-11375H", 8);
        push("i7-11390H", 8);
        push("i7-11600H", 12);
        push("i7-1160G7", 8);
        push("i7-1165G7", 8);
        push("i7-11700", 16);
        push("i7-11700B", 16);
        push("i7-11700F", 16);
        push("i7-11700K", 16);
        push("i7-11700KF", 16);
        push("i7-11700T", 16);
        push("i7-11800H", 16);
        push("i7-1180G7", 8);
        push("i7-11850H", 16);
        push("i7-11850HE", 16);
        push("i7-1185G7", 8);
        push("i7-1185G7E", 8);
        push("i7-1185GRE", 8);
        push("i7-1195G7", 8);
        push("i7-1250U", 4);
        push("i7-1255U", 4);
        push("i7-1260P", 8);
        push("i7-1260U", 4);
        push("i7-1265U", 4);
        push("i7-12700", 16);
        push("i7-12700F", 16);
        push("i7-12700KF", 16);
        push("i7-12700T", 16);
        push("i7-1270P", 8);
        push("i7-1270PE", 8);
        push("i7-1360P", 8);
        push("i7-13700", 16);
        push("i7-13700F", 16);
        push("i7-13700K", 16);
        push("i7-13700KF", 16);
        push("i7-13700T", 16);
        push("i7-13790F", 16);
        push("i7-2535QM", 8);
        push("i7-2570QM", 8);
        push("i7-2600", 8);
        push("i7-2600K", 8);
        push("i7-2600S", 8);
        push("i7-2610UE", 4);
        push("i7-2617M", 4);
        push("i7-2620M", 4);
        push("i7-2627M", 4);
        push("i7-2629M", 4);
        push("i7-2630QM", 8);
        push("i7-2635QM", 8);
        push("i7-2637M", 4);
        push("i7-2640M", 4);
        push("i7-2649M", 4);
        push("i7-2655LE", 4);
        push("i7-2655QM", 8);
        push("i7-2657M", 4);
        push("i7-2660M", 4);
        push("i7-2667M", 4);
        push("i7-2669M", 4);
        push("i7-2670QM", 8);
        push("i7-2675QM", 8);
        push("i7-2677M", 4);
        push("i7-2685QM", 8);
        push("i7-2689M", 4);
        push("i7-2700K", 8);
        push("i7-2710QE", 8);
        push("i7-2715QE", 8);
        push("i7-2720QM", 8);
        push("i7-2740QM", 8);
        push("i7-2760QM", 8);
        push("i7-2820QM", 8);
        push("i7-2840QM", 8);
        push("i7-2860QM", 8);
        push("i7-2920XM", 8);
        push("i7-2960XM", 8);
        push("i7-3517U", 4);
        push("i7-3517UE", 4);
        push("i7-3520M", 4);
        push("i7-3537U", 4);
        push("i7-3540M", 4);
        push("i7-3555LE", 4);
        push("i7-3610QE", 8);
        push("i7-3610QM", 8);
        push("i7-3612QE", 8);
        push("i7-3612QM", 8);
        push("i7-3615QE", 8);
        push("i7-3615QM", 8);
        push("i7-3630QM", 8);
        push("i7-3632QM", 8);
        push("i7-3635QM", 8);
        push("i7-3667U", 4);
        push("i7-3687U", 4);
        push("i7-3689Y", 4);
        push("i7-3720QM", 8);
        push("i7-3740QM", 8);
        push("i7-3770", 8);
        push("i7-3770K", 8);
        push("i7-3770S", 8);
        push("i7-3770T", 8);
        push("i7-3820", 8);
        push("i7-3820QM", 8);
        push("i7-3840QM", 8);
        push("i7-3920XM", 8);
        push("i7-3930K", 12);
        push("i7-3940XM", 8);
        push("i7-3960X", 12);
        push("i7-3970X", 12);
        push("i7-4500U", 4);
        push("i7-4510U", 4);
        push("i7-4550U", 4);
        push("i7-4558U", 4);
        push("i7-4578U", 4);
        push("i7-4600M", 4);
        push("i7-4600U", 4);
        push("i7-4610M", 8);
        push("i7-4610Y", 4);
        push("i7-4650U", 4);
        push("i7-4700EC", 8);
        push("i7-4700EQ", 8);
        push("i7-4700HQ", 8);
        push("i7-4700MQ", 8);
        push("i7-4701EQ", 8);
        push("i7-4702EC", 8);
        push("i7-4702HQ", 8);
        push("i7-4702MQ", 8);
        push("i7-4710HQ", 8);
        push("i7-4710MQ", 8);
        push("i7-4712HQ", 8);
        push("i7-4712MQ", 8);
        push("i7-4720HQ", 8);
        push("i7-4722HQ", 8);
        push("i7-4750HQ", 8);
        push("i7-4760HQ", 8);
        push("i7-4765T", 8);
        push("i7-4770", 8);
        push("i7-4770HQ", 8);
        push("i7-4770K", 8);
        push("i7-4770R", 8);
        push("i7-4770S", 8);
        push("i7-4770T", 8);
        push("i7-4770TE", 8);
        push("i7-4771", 8);
        push("i7-4785T", 8);
        push("i7-4790", 8);
        push("i7-4790K", 8);
        push("i7-4790S", 8);
        push("i7-4790T", 8);
        push("i7-4800MQ", 8);
        push("i7-4810MQ", 8);
        push("i7-4820K", 8);
        push("i7-4850EQ", 8);
        push("i7-4850HQ", 8);
        push("i7-4860EQ", 8);
        push("i7-4860HQ", 8);
        push("i7-4870HQ", 8);
        push("i7-4900MQ", 8);
        push("i7-4910MQ", 8);
        push("i7-4930K", 12);
        push("i7-4930MX", 8);
        push("i7-4940MX", 8);
        push("i7-4950HQ", 8);
        push("i7-4960HQ", 8);
        push("i7-4960X", 12);
        push("i7-4980HQ", 8);
        push("i7-5500U", 4);
        push("i7-5550U", 4);
        push("i7-5557U", 4);
        push("i7-5600U", 4);
        push("i7-5650U", 4);
        push("i7-5700EQ", 8);
        push("i7-5700HQ", 8);
        push("i7-5750HQ", 8);
        push("i7-5775C", 8);
        push("i7-5775R", 8);
        push("i7-5820K", 12);
        push("i7-5850EQ", 8);
        push("i7-5850HQ", 8);
        push("i7-5930K", 12);
        push("i7-5950HQ", 8);
        push("i7-5960X", 16);
        push("i7-610E", 4);
        push("i7-620LE", 4);
        push("i7-620LM", 4);
        push("i7-620M", 4);
        push("i7-620UE", 4);
        push("i7-620UM", 4);
        push("i7-640LM", 4);
        push("i7-640M", 4);
        push("i7-640UM", 4);
        push("i7-6498DU", 4);
        push("i7-6500U", 4);
        push("i7-6560U", 4);
        push("i7-6567U", 4);
        push("i7-6600U", 4);
        push("i7-660LM", 4);
        push("i7-660UE", 4);
        push("i7-660UM", 4);
        push("i7-6650U", 4);
        push("i7-6660U", 4);
        push("i7-6700", 8);
        push("i7-6700HQ", 8);
        push("i7-6700K", 8);
        push("i7-6700T", 8);
        push("i7-6700TE", 8);
        push("i7-6770HQ", 8);
        push("i7-6785R", 8);
        push("i7-6800K", 12);
        push("i7-680UM", 4);
        push("i7-6820EQ", 8);
        push("i7-6820HK", 8);
        push("i7-6820HQ", 8);
        push("i7-6822EQ", 8);
        push("i7-6850K", 12);
        push("i7-6870HQ", 8);
        push("i7-6900K", 16);
        push("i7-6920HQ", 8);
        push("i7-6950X", 20);
        push("i7-6970HQ", 8);
        push("i7-720QM", 8);
        push("i7-740QM", 8);
        push("i7-7500U", 4);
        push("i7-7510U", 4);
        push("i7-7560U", 4);
        push("i7-7567U", 4);
        push("i7-7600U", 4);
        push("i7-7660U", 4);
        push("i7-7700", 8);
        push("i7-7700HQ", 8);
        push("i7-7700K", 8);
        push("i7-7700T", 8);
        push("i7-7740X", 8);
        push("i7-7800X", 12);
        push("i7-7820EQ", 8);
        push("i7-7820HK", 8);
        push("i7-7820HQ", 8);
        push("i7-7820X", 16);
        push("i7-7920HQ", 8);
        push("i7-7Y75", 4);
        push("i7-8086K", 12);
        push("i7-820QM", 8);
        push("i7-840QM", 8);
        push("i7-8500Y", 4);
        push("i7-8550U", 8);
        push("i7-8557U", 8);
        push("i7-8559U", 8);
        push("i7-8565U", 8);
        push("i7-8569U", 8);
        push("i7-860", 8);
        push("i7-860S", 8);
        push("i7-8650U", 8);
        push("i7-8665U", 8);
        push("i7-8665UE", 8);
        push("i7-8670", 12);
        push("i7-8670T", 12);
        push("i7-870", 8);
        push("i7-8700", 12);
        push("i7-8700B", 12);
        push("i7-8700K", 12);
        push("i7-8700T", 12);
        push("i7-8705G", 8);
        push("i7-8706G", 8);
        push("i7-8709G", 8);
        push("i7-870S", 8);
        push("i7-8750H", 12);
        push("i7-875K", 8);
        push("i7-880", 8);
        push("i7-8809G", 8);
        push("i7-8850H", 12);
        push("i7-920", 8);
        push("i7-920XM", 8);
        push("i7-930", 8);
        push("i7-940", 8);
        push("i7-940XM", 8);
        push("i7-950", 8);
        push("i7-960", 8);
        push("i7-965", 8);
        push("i7-970", 12);
        push("i7-9700", 8);
        push("i7-9700E", 8);
        push("i7-9700F", 8);
        push("i7-9700K", 8);
        push("i7-9700KF", 8);
        push("i7-9700T", 8);
        push("i7-9700TE", 8);
        push("i7-975", 8);
        push("i7-9750H", 12);
        push("i7-9750HF", 12);
        push("i7-980", 12);
        push("i7-9800X", 16);
        push("i7-980X", 12);
        push("i7-9850H", 12);
        push("i7-9850HE", 12);
        push("i7-9850HL", 12);
        push("i7-990X", 12);

        // i9 series
        push("i9-10850K", 20);
        push("i9-10885H", 16);
        push("i9-10900", 20);
        push("i9-10900E", 20);
        push("i9-10900F ", 20);
        push("i9-10900K", 20);
        push("i9-10900KF", 20);
        push("i9-10900T", 20);
        push("i9-10900TE", 20);
        push("i9-10900X", 20);
        push("i9-10910", 20);
        push("i9-10920X", 24);
        push("i9-10940X", 28);
        push("i9-10980HK", 16);
        push("i9-10980XE", 36);
        push("i9-11900", 16);
        push("i9-11900F", 16);
        push("i9-11900H", 16);
        push("i9-11900K", 16);
        push("i9-11900KB", 16);
        push("i9-11900KF", 16);
        push("i9-11900T", 16);
        push("i9-11950H", 16);
        push("i9-11980HK", 16);
        push("i9-12900", 16);
        push("i9-12900F", 16);
        push("i9-12900K", 16);
        push("i9-12900KF", 16);
        push("i9-12900KS", 16);
        push("i9-12900T", 16);
        push("i9-13900", 16);
        push("i9-13900E", 16);
        push("i9-13900F", 16);
        push("i9-13900HX", 16);
        push("i9-13900K", 16);
        push("i9-13900KF", 16);
        push("i9-13900KS", 16);
        push("i9-13900T", 16);
        push("i9-13900TE", 16);
        push("i9-13950HX", 16);
        push("i9-13980HX", 16);
        push("i9-14900", 16);
        push("i9-14900F", 16);
        push("i9-14900HX", 16);
        push("i9-14900K", 16);
        push("i9-14900KF", 16);
        push("i9-14900KS", 16);
        push("i9-14900T", 16);
        push("i9-7900X", 20);
        push("i9-7920X", 24);
        push("i9-7940X", 28);
        push("i9-7960X", 32);
        push("i9-7980XE", 36);
        push("i9-8950HK", 12);
        push("i9-9820X", 20);
        push("i9-9880H", 16);
        push("i9-9900", 16);
        push("i9-9900K", 16);
        push("i9-9900KF", 16);
        push("i9-9900KS", 16);
        push("i9-9900T", 16);
        push("i9-9900X", 20);
        push("i9-9920X", 24);
        push("i9-9940X", 28);
        push("i9-9960X", 32);
        push("i9-9980HK", 16);
        push("i9-9980XE", 36);
        push("i9-9990XE", 28);

        // basically means "if there's 0 matches in the database, return false"
        if (thread_database->count(model.string) == 0) {
            return false;
        }

        const u8 threads = thread_database->at(model.string);

        debug("INTEL_THREAD_MISMATCH: thread in database = ", static_cast<u32>(threads));

        return (std::thread::hardware_concurrency() != threads);
#endif
    }
    catch (...) {
        debug("INTEL_THREAD_MISMATCH: catched error, returned false");
        return false;
    }


    /**
     * @brief Check for Intel Xeon CPUs that don't match their thread count
     * @category All, x86
     * @link https://en.wikipedia.org/wiki/List_of_Intel_Core_processors
     */ 
    [[nodiscard]] static bool xeon_thread_mismatch() try {
#if (!x86)
        return false;
#else
        if (core::disabled(XEON_THREAD_MISMATCH)) {
            return false;
        }

        if (!cpu::is_intel()) {
            return false;
        }

        if (cpu::has_hyperthreading()) {
            return false;
        }

        const cpu::model_struct model = cpu::get_model();

        if (!model.found) {
            return false;
        }

        if (!model.is_i_series) {
            return false;
        }

        debug("XEON_THREAD_MISMATCH: CPU model = ", model.string);

        auto xeon_thread_database = std::make_unique<std::unordered_map<std::string, u8>>();

        #define xeon_push xeon_thread_database->emplace

        // Xeon D
        xeon_push("D-1518", 8);
        xeon_push("D-1520", 8);
        xeon_push("D-1521", 8);
        xeon_push("D-1527", 8);
        xeon_push("D-1528", 12);
        xeon_push("D-1529", 8);
        xeon_push("D-1531", 12);
        xeon_push("D-1537", 16);
        xeon_push("D-1539", 16);
        xeon_push("D-1540", 16);
        xeon_push("D-1541", 16);
        xeon_push("D-1548", 16);
        xeon_push("D-1557", 24);
        xeon_push("D-1559", 24);
        xeon_push("D-1567", 24);
        xeon_push("D-1571", 32);
        xeon_push("D-1577", 32);
        xeon_push("D-1581", 32);
        xeon_push("D-1587", 32);
        xeon_push("D-1513N", 8);
        xeon_push("D-1523N", 8);
        xeon_push("D-1533N", 12);
        xeon_push("D-1543N", 16);
        xeon_push("D-1553N", 16);
        xeon_push("D-1602", 4);
        xeon_push("D-1612", 8);
        xeon_push("D-1622", 8);
        xeon_push("D-1627", 8);
        xeon_push("D-1632", 16);
        xeon_push("D-1637", 12);
        xeon_push("D-1623N", 8);
        xeon_push("D-1633N", 12);
        xeon_push("D-1649N", 16);
        xeon_push("D-1653N", 16);
        xeon_push("D-2141I", 16);
        xeon_push("D-2161I", 24);
        xeon_push("D-2191", 36);
        xeon_push("D-2123IT", 8);
        xeon_push("D-2142IT", 16);
        xeon_push("D-2143IT", 16);
        xeon_push("D-2163IT", 24);
        xeon_push("D-2173IT", 28);
        xeon_push("D-2183IT", 32);
        xeon_push("D-2145NT", 16);
        xeon_push("D-2146NT", 16);
        xeon_push("D-2166NT", 24);
        xeon_push("D-2177NT", 28);
        xeon_push("D-2187NT", 32);

        // Xeon E
        xeon_push("E-2104G", 4);
        xeon_push("E-2124", 4);
        xeon_push("E-2124G", 4);
        xeon_push("E-2126G", 6);
        xeon_push("E-2134", 8);
        xeon_push("E-2136", 12);
        xeon_push("E-2144G", 8);
        xeon_push("E-2146G", 12);
        xeon_push("E-2174G", 8);
        xeon_push("E-2176G", 12);
        xeon_push("E-2186G", 12);
        xeon_push("E-2176M", 12);
        xeon_push("E-2186M", 12);
        xeon_push("E-2224", 4);
        xeon_push("E-2224G", 4);
        xeon_push("E-2226G", 6);
        xeon_push("E-2234", 8);
        xeon_push("E-2236", 12);
        xeon_push("E-2244G", 8);
        xeon_push("E-2246G", 12);
        xeon_push("E-2274G", 8);
        xeon_push("E-2276G", 12);
        xeon_push("E-2278G", 16);
        xeon_push("E-2286G", 12);
        xeon_push("E-2288G", 16);
        xeon_push("E-2276M", 12);
        xeon_push("E-2286M", 16);

        // Xeon W
        xeon_push("W-2102", 4);
        xeon_push("W-2104", 4);
        xeon_push("W-2123", 8);
        xeon_push("W-2125", 8);
        xeon_push("W-2133", 12);
        xeon_push("W-2135", 12);
        xeon_push("W-2140B", 16);
        xeon_push("W-2145", 16);
        xeon_push("W-2150B", 20);
        xeon_push("W-2155", 20);
        xeon_push("W-2170B", 28);
        xeon_push("W-2175", 28);
        xeon_push("W-2191B", 36);
        xeon_push("W-2195", 36);
        xeon_push("W-3175X", 56);
        xeon_push("W-3223", 16);
        xeon_push("W-3225", 16);
        xeon_push("W-3235", 24);
        xeon_push("W-3245", 32);
        xeon_push("W-3245M", 32);
        xeon_push("W-3265", 48);
        xeon_push("W-3265M", 48);
        xeon_push("W-3275", 56);
        xeon_push("W-3275M", 56);

        if (xeon_thread_database->count(model.string) == 0) {
            return false;
        }

        const u8 threads = xeon_thread_database->at(model.string);

        debug("XEON_THREAD_MISMATCH: thread in database = ", static_cast<u32>(threads));

        return (std::thread::hardware_concurrency() != threads);
#endif
    }
    catch (...) {
        debug("XEON_THREAD_MISMATCH: catched error, returned false");
        return false;
    }





// TODO: DO AMD
//https://en.wikipedia.org/wiki/List_of_AMD_Ryzen_processors





















    struct core {
        MSVC_DISABLE_WARNING(PADDING)
        struct technique {
            u8 points;
            std::function<bool()> run; // this is the technique function
        };
        MSVC_ENABLE_WARNING(PADDING)

        static const std::map<u8, technique> table;

        static std::vector<technique> custom_table;

        // VM scoreboard table specifically for VM::brand()
        static std::map<const char*, brand_score_t> brand_scoreboard;

        // directly return when adding a brand to the scoreboard for a more succint expression
#if (MSVC)
        __declspec(noalias)
#elif (LINUX)
        [[gnu::const]]
#endif
        static inline bool add(const char* p_brand) noexcept {
            core::brand_scoreboard.at(p_brand)++;
            return true;
        }

        // assert if the flag is enabled, far better expression than typing std::bitset member functions
#if (LINUX && __has_cpp_attribute(gnu::pure))
        [[gnu::pure]]
#endif
        [[nodiscard]] static inline bool disabled(const u8 flag_bit) noexcept {
            return (!flags.test(flag_bit));
        }

        // same as above but for checking enabled flags
#if (LINUX && __has_cpp_attribute(gnu::pure))
        [[gnu::pure]]
#endif
        [[nodiscard]] static inline bool enabled(const u8 flag_bit) noexcept {
            return (flags.test(flag_bit));
        }

        static u16 run_all(flagset p_flags = DEFAULT) {
            u16 points = 0;
            VM::flags = p_flags;
            const bool memo_disabled = core::enabled(VM::NO_MEMO);

            // for main technique table
            for (const auto& tmp : table) {
                const u8 macro = tmp.first;
                technique pair = tmp.second;

                // check if the technique is cached already
                if (memo::is_cached(macro)) {
                    const memo::data_t data = memo::cache_fetch(macro);

                    if (data.result) {
                        points += data.points;
                    }

                    continue;
                }

                const bool result = pair.run();

                if (result) {
                    points += pair.points;
                }
    
                if (!memo_disabled) {
                    memo::cache_store(macro, result, pair.points);
                }
            }

            if (custom_table.empty()) {
                return points;
            }

            // for custom VM techniques
            for (const auto& pair : custom_table) {
                if (pair.run()) {
                    points += pair.points;
                }
            }


            return points;
        }


        static bool hyperv_default_check() {
            if (MSVC && core::disabled(WIN_HYPERV_DEFAULT)) {
                std::string tmp_brand;

                if (memo::is_brand_cached()) {
                    tmp_brand = memo::fetch_brand();
                } else {
                    tmp_brand = brand();
                }

                if (
                    tmp_brand == VM::HYPERV ||
                    tmp_brand == VM::VPC ||
                    tmp_brand == "Microsoft Virtual PC/Hyper-V"
                ) {
                    return true;
                }
            }
            
            return false;
        }
    };


public: // START OF PUBLIC FUNCTIONS
    /**
     * @brief Check for a specific technique based on flag argument
     * @param u8 (flags from VM wrapper)
     * @return bool
     * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmcheck
     */
#if (CPP >= 20 && !CLANG) // not sure why clang doesn't support this lol
    [[nodiscard]] static bool check(const u8 p_flag = 0, const std::source_location& loc = std::source_location::current()) {
#else
    [[nodiscard]] static bool check(const u8 p_flag = 0) {
#endif
        auto throw_error = [&](const char* text) -> void {
            std::stringstream ss;
#if (CPP >= 20 && !CLANG)
            ss << ", error in " << loc.function_name() << " at " << loc.file_name() << ":" << loc.line() << ")";
#endif
            ss << ". Consult the documentation's flag handler for VM::check()";
            throw std::invalid_argument(std::string(text) + ss.str());
        };

        if (p_flag > enum_size) {
            throw_error("Flag argument must be a valid");
        }

        if (p_flag == 0) {
            throw_error("Flag argument must contain at least a single option");
        }

        if (
            (p_flag == NO_MEMO) || \
            (p_flag == EXTREME) || \
            (p_flag == WIN_HYPERV_DEFAULT) || \
            (p_flag == MULTIPLE)
        ) {
            throw_error("Flag argument must be a technique flag and not a settings flag");
        }

        // count should only have a single flag at this stage
        assert(p_flag > 0 && p_flag <= enum_size);

        if (memo::is_cached(p_flag)) {
            const memo::data_t data = memo::cache_fetch(p_flag);
            return data.result;
        }

        // temporarily enable all flags so that every technique is enabled
        const flagset tmp_flags = VM::flags;
        VM::flags = ALL;

        bool result = false;

        auto it = core::table.find(p_flag);

        if ((it == core::table.end())) {
            throw_error("Flag is not known");
        }

        const core::technique& pair = it->second;
        result = pair.run();

        VM::flags = tmp_flags;

        memo::cache_store(p_flag, result, pair.points);

        return result;
    }


    /**
     * @brief Fetch the VM brand
     * @param either nothing or VM::MULTIPLE
     * @return std::string
     * @returns VMware, VirtualBox, KVM, bhyve, QEMU, Microsoft Hyper-V, Microsoft x86-to-ARM, Parallels, Xen HVM, ACRN, QNX hypervisor, Hybrid Analysis, Sandboxie, Docker, Wine, Virtual Apple, Virtual PC, Unknown
     * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmbrand
     */
    [[nodiscard]] static std::string brand(u8 is_multiple = false) {
        // this is added to set the brand scoreboard table in case all techniques were not ran
        if (!memo::all_present()) {
            u16 tmp = core::run_all(DEFAULT);
            UNUSED(tmp);
        }

        #define brands core::brand_scoreboard

#ifdef __VMAWARE_DEBUG__
        for (const auto p : brands) {
            debug("scoreboard: ", (int)p.second, " : ", p.first);
        }
#endif

        if (is_multiple == VM::MULTIPLE) {
            is_multiple = true;
        } else if (is_multiple != 0) {
            throw std::invalid_argument("Flag for VM::brand() must either be empty or VM::MULTIPLE. Consult the documentation's flag handler for VM::check()");
        }

        if (is_multiple) {
            if (memo::is_multibrand_cached()) {
                return memo::fetch_multibrand();
            }
        } else {
            if (memo::is_brand_cached()) {
                return memo::fetch_brand();
            }
        }

        std::string current_brand = "";
        i32 max = 0;

        // both do the same thing
#if (CPP >= 17)
        for (const auto& [brand, points] : brands) {
            if (points > max) {
                current_brand = brand;
                max = points;
            }
        }
#else
        for (auto it = brands.cbegin(); it != brands.cend(); ++it) {
            if (it->second > max) {
                current_brand = it->first;
                max = it->second;
            }
        }
#endif

        if (max == 0) {
            return "Unknown";
        }

        // goofy ass C++11 and C++14 linker error workaround
#if (CPP <= 14)
        constexpr const char* TMP_QEMU        = "QEMU";
        constexpr const char* TMP_KVM         = "KVM";
        constexpr const char* TMP_KVM_HYPERV  = "KVM Hyper-V Enlightenment";
    
        constexpr const char* TMP_VMWARE      = "VMware";
        constexpr const char* TMP_EXPRESS     = "VMware Express";
        constexpr const char* TMP_ESX         = "VMware ESX";
        constexpr const char* TMP_GSX         = "VMware GSX";
        constexpr const char* TMP_WORKSTATION = "VMware Workstation";

        constexpr const char* TMP_VPC         = "Virtual PC";
        constexpr const char* TMP_HYPERV      = "Microsoft Hyper-V";
#else
        constexpr const char* TMP_QEMU        = VM::QEMU;
        constexpr const char* TMP_KVM         = VM::KVM;
        constexpr const char* TMP_KVM_HYPERV  = VM::KVM_HYPERV;

        constexpr const char* TMP_VMWARE      = VM::VMWARE;
        constexpr const char* TMP_EXPRESS     = VM::VMWARE_EXPRESS;
        constexpr const char* TMP_ESX         = VM::VMWARE_ESX;
        constexpr const char* TMP_GSX         = VM::VMWARE_GSX;
        constexpr const char* TMP_WORKSTATION = VM::VMWARE_WORKSTATION;

        constexpr const char* TMP_VPC         = VM::VPC;
        constexpr const char* TMP_HYPERV      = VM::HYPERV;
#endif

        if (
            brands.at(TMP_KVM) > 0 &&
            brands.at(TMP_KVM_HYPERV) > 0
        ) {
            current_brand = TMP_KVM_HYPERV;
        } else if (
            brands.at(TMP_QEMU) > 0 &&
            brands.at(TMP_KVM) > 0
        ) {
            current_brand = "QEMU+KVM";
        } else if (
            brands.at(TMP_VPC) > 0 &&
            brands.at(TMP_HYPERV) > 0
        ) {
#if (MSVC)
            if (util::get_windows_version() < 10) {
                current_brand = TMP_VPC;
            } else {
                current_brand = TMP_HYPERV;
            }
#else
            current_brand = "Microsoft Virtual PC/Hyper-V";
#endif
        } else if (brands.at(TMP_VMWARE) > 0) {
            if (brands.at(TMP_EXPRESS) > 0) {
                current_brand = TMP_EXPRESS;
            } else if (brands.at(TMP_ESX) > 0) {
                current_brand = TMP_ESX;
            } else if (brands.at(TMP_GSX) > 0) {
                current_brand = TMP_GSX;
            } else if (brands.at(TMP_WORKSTATION) > 0) {
                current_brand = TMP_WORKSTATION;
            }
        } else if (is_multiple) {
            std::vector<std::string> potential_brands;

            for (auto it = brands.cbegin(); it != brands.cend(); ++it) {
                const brand_score_t points = it->second;
                const std::string brand = it->first;

                if (points > 0) {
                    potential_brands.push_back(brand);
                }
            }

            std::stringstream ss;
            u8 i = 1;

            ss << potential_brands.front();
            for (; i < potential_brands.size(); i++) {
                ss << " or ";
                ss << potential_brands.at(i);
            }
            current_brand = ss.str();
        }

        if (is_multiple) {
            memo::store_multibrand(current_brand);
        } else {
            memo::store_brand(current_brand);
        }

        return current_brand;
    }


    /**
     * @brief Detect if running inside a VM
     * @param any combination of flags, can be optional
     * @return bool
     * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmdetect
     */
    static bool detect(flagset p_flags = DEFAULT) {
        bool result = false;

        const u16 points = core::run_all(p_flags);

        if (core::enabled(EXTREME)) {
            result = (points > 0);
        } else {
            result = (points >= 100);
        }

        if (core::hyperv_default_check()) {
            return false;
        }

        return result;
    }


    /**
     * @brief Get the percentage of how likely it's a VM
     * @param any combination of flags, can be optional
     * @return std::uint8_t
     * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmpercentage
     */
    static u8 percentage(flagset p_flags = DEFAULT) {
        const u16 points = core::run_all(p_flags);
        u8 percent = 0;

        if (points > 100) {
            percent = 100;
        } else {
            percent = static_cast<u8>(points);
        }

        if (core::hyperv_default_check()) {
            return 0;
        }

        return percent;
    }


    /**
     * @brief Add a custom technique to the VM detection technique collection
     * @param either a function pointer, lambda function, or std::function<bool()>
     * @link https://github.com/kernelwernel/VMAware/blob/main/docs/documentation.md#vmaddcustom
     * @return void
     */
    static void add_custom(
        const std::uint8_t percent,
#if (CPP >= 20 && !CLANG)
        std::function<bool()> detection_func,
        const std::source_location & loc = std::source_location::current()
#else
        std::function<bool()> detection_func
#endif
    ) {
        auto throw_error = [&](const char* text) -> void {
            std::stringstream ss;
#if (CPP >= 20 && !CLANG)
            ss << ", error in " << loc.function_name() << " at " << loc.file_name() << ":" << loc.line() << ")";
#endif
            ss << ". Consult the documentation's parameters for VM::add_custom()";
            throw std::invalid_argument(std::string(text) + ss.str());
            };

        if (percent > 100) {
            throw_error("Percentage parameter must be between 0 and 100");
        }

        core::technique query{
            percent,
            detection_func
        };

        core::custom_table.emplace_back(query);
    }
};

MSVC_ENABLE_WARNING(ASSIGNMENT_OPERATOR NO_INLINE_FUNC SPECTRE)


// ============= EXTERNAL DEFINITIONS =============
// These are added here due to warnings related to C++17 inline variables for C++ standards that are under 17.
// It's easier to just group them together rather than having C++17<= preprocessors with inline stuff

std::map<const char*, VM::brand_score_t> VM::core::brand_scoreboard {
    { VM::VBOX, 0 },
    { VM::VMWARE, 0 },
    { VM::VMWARE_EXPRESS, 0 },
    { VM::VMWARE_ESX, 0 },
    { VM::VMWARE_GSX, 0 },
    { VM::VMWARE_WORKSTATION, 0 },
    { VM::BHYVE, 0 },
    { VM::QEMU, 0 },
    { VM::KVM, 0 },
    { VM::KVM_HYPERV, 0 },
    { VM::HYPERV, 0 },
    { VM::MSXTA, 0 },
    { VM::PARALLELS, 0 },
    { VM::XEN, 0 },
    { VM::ACRN, 0 },
    { VM::QNX, 0 },
    { VM::HYBRID, 0 },
    { VM::SANDBOXIE, 0 },
    { VM::DOCKER, 0 },
    { VM::WINE, 0 },
    { VM::VAPPLE, 0 },
    { VM::VPC, 0 },
    { VM::ANUBIS, 0 },
    { VM::JOEBOX, 0 },
    { VM::THREADEXPERT, 0 },
    { VM::CWSANDBOX, 0 },
    { VM::COMODO, 0 },
    { VM::BOCHS, 0 },
    { VM::NVMM, 0},
    { VM::BSD_VMM, 0 },
    { VM::INTEL_HAXM, 0 },
    { VM::UNISYS, 0 },
    { VM::LMHS, 0 }
};

std::map<VM::u8, VM::memo::data_t> VM::memo::cache_table;
VM::flagset VM::memo::cache_keys = 0;
std::string VM::memo::brand_cache = "";
std::string VM::memo::multibrand_cache = "";
std::string VM::memo::cpu_brand_cache = "";


VM::flagset VM::flags = 0;

VM::flagset VM::DEFAULT = []() -> flagset {
    flagset tmp;

    // set all bits to 1
    tmp.set();

    // disable all the non-default flags
    tmp.flip(EXTREME);
    tmp.flip(NO_MEMO);
    tmp.flip(CURSOR);
    tmp.flip(WIN_HYPERV_DEFAULT);
    tmp.flip(MULTIPLE);
    return tmp;
}();

VM::flagset VM::ALL = []() -> flagset {
    flagset tmp = DEFAULT;
    tmp.set(CURSOR);
    return tmp;
}();


bool VM::cpuid_supported = []() -> bool {
#if (x86)
    #if (MSVC)
        int32_t info[4];
        __cpuid(info, 0);
        return (info[0] > 0);
    #elif (LINUX)
        u32 ext = 0;
        return (__get_cpuid_max(ext, nullptr) > 0);
    #else
        return false;
    #endif
#else
    return false;
#endif
}();


// this is initialised as empty, because this is where custom techniques can be added at runtime 
std::vector<VM::core::technique> VM::core::custom_table = {

};


// the 0~100 points are debatable, but I think it's fine how it is. Feel free to disagree.
const std::map<VM::u8, VM::core::technique> VM::core::table = {
    { VM::VMID, { 100, VM::vmid }},
    { VM::BRAND, { 50, VM::cpu_brand }},
    { VM::HYPERVISOR_BIT, { 100, VM::hypervisor_bit }},
    { VM::CPUID_0X4, { 70, VM::cpuid_0x4 }},
    { VM::HYPERVISOR_STR, { 45, VM::hypervisor_brand }},
    { VM::RDTSC, { 20, VM::rdtsc_check }},
    { VM::THREADCOUNT, { 35, VM::thread_count }},
    { VM::MAC, { 90, VM::mac_address_check }},
    { VM::TEMPERATURE, { 15, VM::temperature }},
    { VM::SYSTEMD, { 70, VM::systemd_virt }},
    { VM::CVENDOR, { 65, VM::chassis_vendor }},
    { VM::CTYPE, { 10, VM::chassis_type }},
    { VM::DOCKERENV, { 80, VM::dockerenv }},
    { VM::DMIDECODE, { 55, VM::dmidecode }},
    { VM::DMESG, { 55, VM::dmesg }},
    { VM::HWMON, { 75, VM::hwmon }},
    { VM::SIDT5, { 45, VM::sidt5 }},
    { VM::CURSOR, { 5, VM::cursor_check }},
    { VM::VMWARE_REG, { 65, VM::vmware_registry }},
    { VM::VBOX_REG, { 65, VM::vbox_registry }},
    { VM::USER, { 35, VM::user_check }},
    { VM::DLL, { 50, VM::DLL_check }},
    { VM::REGISTRY, { 75, VM::registry_key }},
    { VM::CWSANDBOX_VM, { 10, VM::cwsandbox_check }},
    { VM::VM_FILES, { 60, VM::vm_files }},
    { VM::HWMODEL, { 75, VM::hwmodel }},
    { VM::DISK_SIZE, { 60, VM::disk_size }},
    { VM::VBOX_DEFAULT, { 55, VM::vbox_default_specs }},
    { VM::VBOX_NETWORK, { 70, VM::vbox_network_share }},
    { VM::VM_PROCESSES, { 30, VM::vm_processes }},
    { VM::LINUX_USER_HOST, { 25, VM::linux_user_host }},
    { VM::GAMARUE, { 40, VM::gamarue }},
    { VM::VMID_0X4, { 90, VM::vmid_0x4 }},
    { VM::PARALLELS_VM, { 50, VM::parallels }},
    { VM::RDTSC_VMEXIT, { 35, VM::rdtsc_vmexit }},
    { VM::QEMU_BRAND, { 100, VM::cpu_brand_qemu }},
    { VM::BOCHS_CPU, { 95, VM::bochs_cpu }},
    { VM::VPC_BOARD, { 20, VM::vpc_board }},
    { VM::HYPERV_WMI, { 80, VM::hyperv_wmi }},
    { VM::HYPERV_REG, { 80, VM::hyperv_registry }},
    { VM::BIOS_SERIAL, { 60, VM::bios_serial }},
    { VM::VBOX_FOLDERS, { 45, VM::vbox_shared_folders }},
    { VM::MSSMBIOS, { 75, VM::mssmbios }},
    { VM::MAC_MEMSIZE, { 30, VM::hw_memsize }},
    { VM::MAC_IOKIT, { 80, VM::io_kit }},
    { VM::IOREG_GREP, { 75, VM::ioreg_grep }},
    { VM::MAC_SIP, { 85, VM::mac_sip }},
    { VM::HKLM_REGISTRIES, { 70, VM::hklm_registries }},
    { VM::QEMU_GA, { 20, VM::qemu_ga }},
    { VM::VALID_MSR, { 35, VM::valid_msr }},
    { VM::QEMU_PROC, { 30, VM::qemu_processes }},
    { VM::VPC_PROC, { 30, VM::vpc_proc }},
    { VM::VPC_INVALID, { 75, VM::vpc_invalid }},
    { VM::SIDT, { 30, VM::sidt }},
    { VM::SGDT, { 30, VM::sgdt }},
    { VM::SLDT, { 15, VM::sldt }},
    { VM::OFFSEC_SIDT, { 60, VM::offsec_sidt }},
    { VM::OFFSEC_SGDT, { 60, VM::offsec_sgdt }},
    { VM::OFFSEC_SLDT, { 20, VM::offsec_sldt }},
    { VM::VPC_SIDT, { 15, VM::vpc_sidt }},
    { VM::HYPERV_BOARD, { 45, VM::hyperv_board }},
    { VM::VM_FILES_EXTRA, { 70, VM::vm_files_extra }},
    { VM::VMWARE_IOMEM, { 65, VM::vmware_iomem }},
    { VM::VMWARE_IOPORTS, { 70, VM::vmware_ioports }},
    { VM::VMWARE_SCSI, { 40, VM::vmware_scsi }},
    { VM::VMWARE_DMESG, { 65, VM::vmware_dmesg }},
    { VM::VMWARE_STR, { 35, VM::vmware_str }},
    { VM::VMWARE_BACKDOOR, { 100, VM::vmware_backdoor }},
    { VM::VMWARE_PORT_MEM, { 85, VM::vmware_port_memory }},
    { VM::SMSW, { 30, VM::smsw }},
    { VM::MUTEX, { 85, VM::mutex }},
    { VM::UPTIME, { 10, VM::uptime }},
    { VM::ODD_CPU_THREADS, { 80, VM::odd_cpu_threads }},
    { VM::INTEL_THREAD_MISMATCH, { 85, VM::intel_thread_mismatch }},
    { VM::XEON_THREAD_MISMATCH, { 85, VM::xeon_thread_mismatch }}

    // __TABLE_LABEL, add your technique above
    // { VM::FUNCTION, { POINTS, FUNCTION_POINTER }}
    // ^ template 
};
