/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/test/common/test_macros/test.h"

#include "level_zero/core/source/device/device.h"
#include "level_zero/core/test/unit_tests/fixtures/device_fixture.h"
#include "level_zero/core/test/unit_tests/white_box.h"
#include "level_zero/tools/source/metrics/metric.h"
#include "level_zero/tools/source/metrics/metric_oa_enumeration_imp.h"
#include "level_zero/tools/source/metrics/metric_oa_query_imp.h"
#include "level_zero/tools/test/unit_tests/sources/metrics/mock_metric_oa_enumeration.h"

namespace L0 {
namespace ult {

template <>
struct WhiteBox<::L0::MetricQuery> : public ::L0::MetricQuery {
    using BaseClass = ::L0::MetricQuery;
};

template <>
struct WhiteBox<::L0::MetricQueryPool> : public ::L0::MetricQueryPool {
    using BaseClass = ::L0::MetricQuery;
};

using MetricQuery = WhiteBox<::L0::MetricQuery>;
using MetricQueryPool = WhiteBox<::L0::MetricQueryPool>;

using MetricsLibraryApi::ClientData_1_0;
using MetricsLibraryApi::ClientDataLinuxAdapter_1_0;
using MetricsLibraryApi::ClientType_1_0;
using MetricsLibraryApi::CommandBufferData_1_0;
using MetricsLibraryApi::CommandBufferSize_1_0;
using MetricsLibraryApi::ConfigurationActivateData_1_0;
using MetricsLibraryApi::ConfigurationCreateData_1_0;
using MetricsLibraryApi::ConfigurationHandle_1_0;
using MetricsLibraryApi::ContextCreateData_1_0;
using MetricsLibraryApi::ContextHandle_1_0;
using MetricsLibraryApi::GetReportData_1_0;
using MetricsLibraryApi::LinuxAdapterType;
using MetricsLibraryApi::MarkerCreateData_1_0;
using MetricsLibraryApi::MarkerHandle_1_0;
using MetricsLibraryApi::OverrideCreateData_1_0;
using MetricsLibraryApi::OverrideHandle_1_0;
using MetricsLibraryApi::ParameterType;
using MetricsLibraryApi::QueryCreateData_1_0;
using MetricsLibraryApi::QueryHandle_1_0;
using MetricsLibraryApi::StatusCode;
using MetricsLibraryApi::TypedValue_1_0;
using MetricsLibraryApi::ValueType;

struct MockMetricsLibraryApi {

    // Original api functions.
    static StatusCode ML_STDCALL contextCreate(ClientType_1_0 clientType, ContextCreateData_1_0 *createData, ContextHandle_1_0 *handle);
    static StatusCode ML_STDCALL contextDelete(const ContextHandle_1_0 handle);
    static StatusCode ML_STDCALL getParameter(const ParameterType parameter, ValueType *type, TypedValue_1_0 *value);
    static StatusCode ML_STDCALL commandBufferGet(const CommandBufferData_1_0 *data);
    static StatusCode ML_STDCALL commandBufferGetSize(const CommandBufferData_1_0 *data, CommandBufferSize_1_0 *size);
    static StatusCode ML_STDCALL queryCreate(const QueryCreateData_1_0 *createData, QueryHandle_1_0 *handle);
    static StatusCode ML_STDCALL queryDelete(const QueryHandle_1_0 handle);
    static StatusCode ML_STDCALL overrideCreate(const OverrideCreateData_1_0 *createData, OverrideHandle_1_0 *handle);
    static StatusCode ML_STDCALL overrideDelete(const OverrideHandle_1_0 handle);
    static StatusCode ML_STDCALL markerCreate(const MarkerCreateData_1_0 *createData, MarkerHandle_1_0 *handle);
    static StatusCode ML_STDCALL markerDelete(const MarkerHandle_1_0 handle);
    static StatusCode ML_STDCALL configurationCreate(const ConfigurationCreateData_1_0 *createData, ConfigurationHandle_1_0 *handle);
    static StatusCode ML_STDCALL configurationActivate(const ConfigurationHandle_1_0 handle, const ConfigurationActivateData_1_0 *activateData);
    static StatusCode ML_STDCALL configurationDeactivate(const ConfigurationHandle_1_0 handle);
    static StatusCode ML_STDCALL configurationDelete(const ConfigurationHandle_1_0 handle);
    static StatusCode ML_STDCALL getData(GetReportData_1_0 *data);

    // Mocked api functions.
    MOCK_METHOD(StatusCode, MockContextCreate, (ClientType_1_0 clientType, ContextCreateData_1_0 *createData, ContextHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockContextDelete, (const ContextHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockGetParameter, (const ParameterType parameter, ValueType *type, TypedValue_1_0 *value));
    MOCK_METHOD(StatusCode, MockCommandBufferGet, (const CommandBufferData_1_0 *data));
    MOCK_METHOD(StatusCode, MockCommandBufferGetSize, (const CommandBufferData_1_0 *data, CommandBufferSize_1_0 *size));
    MOCK_METHOD(StatusCode, MockQueryCreate, (const QueryCreateData_1_0 *createData, QueryHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockQueryDelete, (const QueryHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockOverrideCreate, (const OverrideCreateData_1_0 *createData, OverrideHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockOverrideDelete, (const OverrideHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockMarkerCreate, (const MarkerCreateData_1_0 *createData, MarkerHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockMarkerDelete, (const MarkerHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockConfigurationCreate, (const ConfigurationCreateData_1_0 *createData, ConfigurationHandle_1_0 *handle));
    MOCK_METHOD(StatusCode, MockConfigurationActivate, (const ConfigurationHandle_1_0 handle, const ConfigurationActivateData_1_0 *activateData));
    MOCK_METHOD(StatusCode, MockConfigurationDeactivate, (const ConfigurationHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockConfigurationDelete, (const ConfigurationHandle_1_0 handle));
    MOCK_METHOD(StatusCode, MockGetData, (GetReportData_1_0 * data));
};

template <>
struct Mock<MetricsLibrary> : public MetricsLibrary {

  public:
    Mock(::L0::OaMetricSourceImp &metricSource);
    ~Mock() override;

    using MetricsLibrary::handle;
    using MetricsLibrary::initializationState;

    // Api mock enable/disable.
    void setMockedApi(MockMetricsLibraryApi *mockedApi);

    // Mocked metrics library functions.
    MOCK_METHOD(bool, load, (), (override));
    MOCK_METHOD(bool, getContextData, (::L0::Device &, ContextCreateData_1_0 &), (override));
    MOCK_METHOD(bool, getMetricQueryReportSize, (size_t & rawDataSize), (override));

    // Not mocked metrics library functions.
    bool metricsLibraryGetContextData(::L0::Device &device, ContextCreateData_1_0 &contextData) { return MetricsLibrary::getContextData(device, contextData); }

    // Original metrics library implementation used by metric context.
    ::L0::MetricsLibrary *metricsLibrary = nullptr;

    // Mocked metrics library api version.
    // We cannot use a static instance here since the gtest validates memory usage,
    // and mocked functions will stay in memory longer than the test.
    static MockMetricsLibraryApi *g_mockApi; // NOLINT(readability-identifier-naming)
};

template <>
struct Mock<MetricQueryPool> : public MetricQueryPool {
    Mock();
    ~Mock() override;

    MOCK_METHOD(ze_result_t, metricQueryCreate, (uint32_t, zet_metric_query_handle_t *), (override));
    MOCK_METHOD(ze_result_t, destroy, (), (override));
};

template <>
struct Mock<MetricQuery> : public MetricQuery {
    Mock();
    ~Mock() override;

    MOCK_METHOD(ze_result_t, getData, (size_t *, uint8_t *), (override));
    MOCK_METHOD(ze_result_t, reset, (), (override));
    MOCK_METHOD(ze_result_t, destroy, (), (override));
};

class MockIpSamplingOsInterface : public MetricIpSamplingOsInterface {

  public:
    ze_result_t getMetricsTimerResolutionReturn = ZE_RESULT_SUCCESS;
    ~MockIpSamplingOsInterface() override = default;
    ze_result_t startMeasurement(uint32_t &notifyEveryNReports, uint32_t &samplingPeriodNs) override {
        return ZE_RESULT_ERROR_UNKNOWN;
    }
    ze_result_t stopMeasurement() override { return ZE_RESULT_ERROR_UNKNOWN; }
    ze_result_t readData(uint8_t *pRawData, size_t *pRawDataSize) override { return ZE_RESULT_ERROR_UNKNOWN; }
    uint32_t getRequiredBufferSize(const uint32_t maxReportCount) override { return 0; }
    uint32_t getUnitReportSize() override { return 0; }
    bool isNReportsAvailable() override { return false; }
    bool isDependencyAvailable() override { return false; }
    ze_result_t getMetricsTimerResolution(uint64_t &timerResolution) override {
        timerResolution = 12500000UL;
        return getMetricsTimerResolutionReturn;
    }
};
class MockOAOsInterface : public MetricOAOsInterface {
  public:
    ~MockOAOsInterface() override = default;
    ze_result_t getMetricsTimerResolutionReturn = ZE_RESULT_SUCCESS;
    uint8_t failGetResolutionOnCall = 0;
    uint8_t getResolutionCallCount = 0;
    ze_result_t getMetricsTimerResolution(uint64_t &timerResolution) override {
        ze_result_t retVal;
        getResolutionCallCount++;
        if ((failGetResolutionOnCall) && (getResolutionCallCount >= failGetResolutionOnCall)) {
            timerResolution = 0UL;
            retVal = getMetricsTimerResolutionReturn;
        } else {
            timerResolution = 25000000UL; // PVC as reference
            retVal = ZE_RESULT_SUCCESS;
        }
        return retVal;
    }
};

class MetricContextFixture : public DeviceFixture {

  protected:
    void setUp();
    void tearDown();
    void openMetricsAdapter();
    void openMetricsAdapterGroup();

  public:
    // Mocked objects.
    std::unique_ptr<Mock<MetricEnumeration>> mockMetricEnumeration = nullptr;
    std::unique_ptr<Mock<MetricsLibrary>> mockMetricsLibrary = nullptr;

    // Mocked metrics library/discovery APIs.
    MockMetricsLibraryApi mockMetricsLibraryApi = {};
    MockMetricsDiscoveryApi mockMetricsDiscoveryApi = {};

    // Metrics discovery device
    Mock<IAdapterGroup_1_9> adapterGroup;
    Mock<IAdapter_1_9> adapter;
    Mock<IMetricsDevice_1_5> metricsDevice;
    MetricsDiscovery::TMetricsDeviceParams_1_2 metricsDeviceParams = {};
    MetricsDiscovery::TTypedValue_1_0 defaultMaximumOaBufferSize = {};
    void setupDefaultMocksForMetricDevice(Mock<IMetricsDevice_1_5> &metricDevice);
    MockOAOsInterface *mockOAOsInterface;
    MockIpSamplingOsInterface *mockIpSamplingOsInterface;
};

class MetricMultiDeviceFixture : public MultiDeviceFixture {

  protected:
    void setUp();
    void tearDown();
    void openMetricsAdapter();
    void openMetricsAdapterSubDevice(uint32_t subDeviceIndex);
    void openMetricsAdapterDeviceAndSubDeviceNoCountVerify(uint32_t subDeviceIndex);
    void openMetricsAdapterGroup();

  public:
    std::vector<L0::Device *> devices;

    // Mocked objects.
    std::unique_ptr<Mock<MetricEnumeration>> mockMetricEnumeration = nullptr;
    std::unique_ptr<Mock<MetricsLibrary>> mockMetricsLibrary = nullptr;

    std::vector<std::unique_ptr<Mock<MetricEnumeration>>> mockMetricEnumerationSubDevices;
    std::vector<std::unique_ptr<Mock<MetricsLibrary>>> mockMetricsLibrarySubDevices;

    // Mocked metrics library/discovery APIs.
    MockMetricsLibraryApi mockMetricsLibraryApi = {};
    MockMetricsDiscoveryApi mockMetricsDiscoveryApi = {};

    // Metrics discovery device
    Mock<IAdapterGroup_1_9> adapterGroup;
    Mock<IAdapter_1_9> adapter;
    Mock<IMetricsDevice_1_5> metricsDevice;
    MetricsDiscovery::TMetricsDeviceParams_1_2 metricsDeviceParams = {};
    //    MetricsDiscovery::TTypedValue_1_0 defaultMaximumOaBufferSize = {};
    void setupDefaultMocksForMetricDevice(Mock<IMetricsDevice_1_5> &metricDevice);
};

class MetricStreamerMultiDeviceFixture : public MetricMultiDeviceFixture {
  public:
    void cleanup(zet_device_handle_t &hDevice, zet_metric_streamer_handle_t &hStreamer);
};

} // namespace ult
} // namespace L0
