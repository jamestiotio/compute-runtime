/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/helpers/compiler_product_helper.h"
#include "shared/source/helpers/compiler_product_helper_base.inl"
#include "shared/source/helpers/compiler_product_helper_bdw_and_later.inl"
#include "shared/source/helpers/compiler_product_helper_before_xe_hpc.inl"
#include "shared/source/helpers/compiler_product_helper_enable_subgroup_local_block_io.inl"
#include "shared/source/helpers/compiler_product_helper_mtl_and_later.inl"
#include "shared/source/helpers/compiler_product_helper_tgllp_and_later.inl"
#include "shared/source/helpers/compiler_product_helper_xe_hp_and_later.inl"
#include "shared/source/xe_hpg_core/hw_cmds_mtl.h"

#include "compiler_product_helper_mtl.inl"
#include "platforms.h"

constexpr auto gfxProduct = IGFX_METEORLAKE;

namespace NEO {
template <>
uint32_t CompilerProductHelperHw<gfxProduct>::getDefaultHwIpVersion() const {
    return AOT::MTL_M_A0;
}

template <>
bool CompilerProductHelperHw<gfxProduct>::isSplitMatrixMultiplyAccumulateSupported(const HardwareInfo &hwInfo) const {
    return (MTL::isLpg(hwInfo) == false);
}

template <>
bool CompilerProductHelperHw<gfxProduct>::isBFloat16ConversionSupported(const HardwareInfo &hwInfo) const {
    return (MTL::isLpg(hwInfo) == false);
}

static EnableCompilerProductHelper<gfxProduct> enableCompilerProductHelperMTL;

} // namespace NEO
