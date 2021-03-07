#pragma once
namespace Cyrex {
    enum class DialogResult {
        None   = 0x0,
        OK     = 0x1,
        Cancel = 0x2,
        Abort  = 0x3,
        Retry  = 0x4,
        Ignore = 0x5,
        Yes    = 0x6,
        No     = 0x7,
    };
}
