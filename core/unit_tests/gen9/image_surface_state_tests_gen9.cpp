/*
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "core/unit_tests/image/image_surface_state_fixture.h"

using ImageSurfaceStateTestsGen9 = ImageSurfaceStateTests;

GEN9TEST_F(ImageSurfaceStateTestsGen9, givenGmmWithMediaCompressedWhenSetFlagsForMediaCompressionThenAuxiliarySurfaceNoneIsSet) {
    auto size = sizeof(typename FamilyType::RENDER_SURFACE_STATE);
    auto surfaceState = std::make_unique<char[]>(size);
    auto castSurfaceState = reinterpret_cast<typename FamilyType::RENDER_SURFACE_STATE *>(surfaceState.get());
    castSurfaceState->setAuxiliarySurfaceMode(FamilyType::RENDER_SURFACE_STATE::AUXILIARY_SURFACE_MODE::AUXILIARY_SURFACE_MODE_AUX_CCS_E);

    mockGmm.gmmResourceInfo->getResourceFlags()->Info.MediaCompressed = false;
    setFlagsForMediaCompression<FamilyType>(castSurfaceState, &mockGmm);
    EXPECT_EQ(castSurfaceState->getAuxiliarySurfaceMode(), FamilyType::RENDER_SURFACE_STATE::AUXILIARY_SURFACE_MODE::AUXILIARY_SURFACE_MODE_AUX_CCS_E);
    mockGmm.gmmResourceInfo->getResourceFlags()->Info.MediaCompressed = true;
    setFlagsForMediaCompression<FamilyType>(castSurfaceState, &mockGmm);
    EXPECT_EQ(castSurfaceState->getAuxiliarySurfaceMode(), FamilyType::RENDER_SURFACE_STATE::AUXILIARY_SURFACE_MODE::AUXILIARY_SURFACE_MODE_AUX_NONE);
}
