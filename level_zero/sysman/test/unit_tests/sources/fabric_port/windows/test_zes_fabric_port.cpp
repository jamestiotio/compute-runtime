/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "level_zero/sysman/source/fabric_port/sysman_fabric_port_imp.h"
#include "level_zero/sysman/test/unit_tests/sources/windows/mock_sysman_fixture.h"

namespace L0 {
namespace Sysman {
namespace ult {

class SysmanFabricPortFixture : public SysmanDeviceFixture {
  protected:
    std::unique_ptr<L0::Sysman::FabricDeviceImp> pFabricDevice;
    std::unique_ptr<L0::Sysman::FabricPortImp> pFabricPortImp;
    void SetUp() override {
        SysmanDeviceFixture::SetUp();
        pFabricDevice.reset(new L0::Sysman::FabricDeviceImp(pOsSysman));
        pFabricPortImp.reset(new L0::Sysman::FabricPortImp(pFabricDevice.get(), 0u));
    }

    void TearDown() override {
        SysmanDeviceFixture::TearDown();
    }
};

TEST_F(SysmanFabricPortFixture, GivenValidFabricPortHandleWhenCallingFabricPortGetPropertiesThenFailureIsReturned) {
    zes_fabric_port_properties_t pProperties = {};
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, pFabricPortImp->fabricPortGetProperties(&pProperties));
}

TEST_F(SysmanFabricPortFixture, GivenValidFabricPortHandleWhenCallingFabricPortGetConfigThenFailureIsReturned) {
    zes_fabric_port_config_t pConfig = {};
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, pFabricPortImp->fabricPortGetConfig(&pConfig));
}

TEST_F(SysmanFabricPortFixture, GivenValidFabricPortHandleWhenCallingFabricPortSetConfigThenFailureIsReturned) {
    zes_fabric_port_config_t pConfig = {};
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, pFabricPortImp->fabricPortSetConfig(&pConfig));
}

TEST_F(SysmanFabricPortFixture, GivenValidFabricPortHandleWhenCallingFabricPortGetStateThenFailureIsReturned) {
    zes_fabric_port_state_t pState = {};
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, pFabricPortImp->fabricPortGetState(&pState));
}

TEST_F(SysmanFabricPortFixture, GivenValidFabricPortHandleWhenCallingFabricPortGetThroughputThenFailureIsReturned) {
    zes_fabric_port_throughput_t pThroughput = {};
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, pFabricPortImp->fabricPortGetThroughput(&pThroughput));
}

TEST_F(SysmanFabricPortFixture, GivenValidFabricPortHandleWhenCallingFabricPortGetLinkTupeThenFailureIsReturned) {
    zes_fabric_link_type_t pLinkType = {};
    EXPECT_EQ(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE, pFabricPortImp->fabricPortGetLinkType(&pLinkType));
}

} // namespace ult
} // namespace Sysman
} // namespace L0
