/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "level_zero/sysman/source/linux/udev/udev_lib.h"
#include "level_zero/sysman/source/os_sysman_driver.h"

namespace L0 {
namespace Sysman {

class LinuxEventsUtil;
struct SysmanDeviceImp;

class LinuxSysmanDriverImp : public OsSysmanDriver {
  public:
    LinuxSysmanDriverImp();
    ~LinuxSysmanDriverImp() override;

    ze_result_t eventsListen(uint64_t timeout, uint32_t count, zes_device_handle_t *phDevices, uint32_t *pNumDeviceEvents, zes_event_type_flags_t *pEvents) override;
    void eventRegister(zes_event_type_flags_t events, SysmanDeviceImp *pSysmanDevice);
    L0::Sysman::UdevLib *getUdevLibHandle();

  protected:
    L0::Sysman::UdevLib *pUdevLib = nullptr;
    L0::Sysman::LinuxEventsUtil *pLinuxEventsUtil = nullptr;
};

} // namespace Sysman
} // namespace L0
