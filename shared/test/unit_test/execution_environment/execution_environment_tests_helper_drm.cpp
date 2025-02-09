/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/source/os_interface/linux/drm_memory_operations_handler_bind.h"
#include "shared/test/common/mocks/mock_driver_model.h"
#include "shared/test/unit_test/execution_environment/execution_environment_tests.h"

using namespace NEO;

void ExecutionEnvironmentSortTests::setupOsSpecifcEnvironment(uint32_t rootDeviceIndex) {
    auto &rootDeviceEnvironment = *executionEnvironment.rootDeviceEnvironments[rootDeviceIndex];
    auto osInterface = std::make_unique<OSInterface>();
    auto driverModel = std::make_unique<MockDriverModel>(DriverModelType::DRM);
    driverModel->pciBusInfo = inputBusInfos[rootDeviceIndex];
    osInterface->setDriverModel(std::move(driverModel));
    rootDeviceEnvironment.osInterface = std::move(osInterface);
    rootDeviceEnvironment.memoryOperationsInterface = std::make_unique<DrmMemoryOperationsHandlerBind>(rootDeviceEnvironment, rootDeviceIndex);
}
