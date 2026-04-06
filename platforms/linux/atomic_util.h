// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// No interrupt protection needed on Linux userspace
#define ATOMIC_BLOCK(type) if (1)
#define ATOMIC_BLOCK_RESTORESTATE
#define ATOMIC_BLOCK_FORCEON
