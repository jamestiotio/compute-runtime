/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/helpers/non_copyable_or_moveable.h"

#include "level_zero/sysman/source/linux/sysman_fs_access.h"
#include "level_zero/sysman/source/linux/zes_os_sysman_imp.h"
#include "level_zero/sysman/source/standby/sysman_os_standby.h"
#include "level_zero/sysman/source/standby/sysman_standby_imp.h"

namespace L0 {
namespace Sysman {

class LinuxStandbyImp : public OsStandby, NEO::NonCopyableOrMovableClass {
  public:
    ze_result_t getMode(zes_standby_promo_mode_t &mode) override;
    ze_result_t setMode(zes_standby_promo_mode_t mode) override;
    ze_result_t osStandbyGetProperties(zes_standby_properties_t &properties) override;

    bool isStandbySupported(void) override;

    LinuxStandbyImp() = default;
    LinuxStandbyImp(OsSysman *pOsSysman, ze_bool_t onSubdevice, uint32_t subdeviceId);
    ~LinuxStandbyImp() override = default;

  protected:
    SysfsAccess *pSysfsAccess = nullptr;

  private:
    std::string standbyModeFile;
    static const int standbyModeDefault = 1;
    static const int standbyModeNever = 0;
    bool isSubdevice = false;
    uint32_t subdeviceId = 0;
    void init();
};

} // namespace Sysman
} // namespace L0
