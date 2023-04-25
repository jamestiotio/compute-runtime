/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/xe_hpc_core/pvc/device_ids_configs_pvc.h"
#include "shared/test/common/libult/linux/drm_mock.h"
#include "shared/test/common/test_macros/hw_test.h"

#include "level_zero/tools/source/metrics/os_interface_metric.h"
#include "level_zero/tools/test/unit_tests/sources/metrics/mock_metric_oa.h"

namespace L0 {
namespace ult {

class DrmTipMock : public DrmMock {
  public:
    DrmTipMock(RootDeviceEnvironment &rootDeviceEnvironment) : DrmTipMock(rootDeviceEnvironment, defaultHwInfo.get()) {}
    DrmTipMock(RootDeviceEnvironment &rootDeviceEnvironment, HardwareInfo *inputHwInfo) : DrmMock(rootDeviceEnvironment) {
        rootDeviceEnvironment.setHwInfoAndInitHelpers(inputHwInfo);
        setupIoctlHelper(rootDeviceEnvironment.getHardwareInfo()->platform.eProductFamily);
    }

    void getPrelimVersion(std::string &prelimVersion) override {
        prelimVersion = "";
    }
};

class MetricIpSamplingLinuxTestUpstream : public MetricContextFixture,
                                          public ::testing::Test {
  public:
    void SetUp() override {
        MetricContextFixture::setUp();
        neoDevice->getExecutionEnvironment()->rootDeviceEnvironments[device->getRootDeviceIndex()]->osInterface = std::make_unique<NEO::OSInterface>();
        auto &osInterface = device->getOsInterface();
        osInterface.setDriverModel(std::make_unique<DrmTipMock>(const_cast<NEO::RootDeviceEnvironment &>(neoDevice->getRootDeviceEnvironment())));
        metricIpSamplingOsInterface = MetricIpSamplingOsInterface::create(static_cast<L0::Device &>(*device));
    }

    void TearDown() override {
        MetricContextFixture::tearDown();
    }
    std::unique_ptr<MetricIpSamplingOsInterface> metricIpSamplingOsInterface = nullptr;
};

HWTEST2_F(MetricIpSamplingLinuxTestUpstream, GivenSupportedProductFamilyAndSupportedDeviceIdIsUsedForUpstreamWhenIsDependencyAvailableIsCalledThenReturnFailure, IsPVC) {

    auto hwInfo = neoDevice->getRootDeviceEnvironment().getMutableHardwareInfo();
    hwInfo->platform.eProductFamily = productFamily;

    for (const auto &deviceId : NEO::pvcXtDeviceIds) {
        hwInfo->platform.usDeviceID = deviceId;
        EXPECT_FALSE(metricIpSamplingOsInterface->isDependencyAvailable());
    }
}

} // namespace ult
} // namespace L0
