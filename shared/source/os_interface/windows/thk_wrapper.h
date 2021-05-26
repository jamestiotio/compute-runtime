/*
 * Copyright (C) 2018-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/helpers/options.h"
#include "shared/source/os_interface/windows/d3dkmthk_wrapper.h"
#include "shared/source/os_interface/windows/windows_wrapper.h"
#include "shared/source/utilities/api_intercept.h"

namespace NEO {
// Default template for GetID( ) for Thk function, causing compilation error !!
// Returns ID for specific ThkWrapper type
template <class Param>
constexpr unsigned int getThkWrapperId() {
    static_assert(sizeof(Param) > sizeof(Param) + 1, "Template specialization for GetID is required for each new THKWrapper");
    return 0;
}

// Template specializations of GetID(), required for every new Thk function
#define GET_ID(TYPE, VALUE)                          \
    template <>                                      \
    constexpr unsigned int getThkWrapperId<TYPE>() { \
        return 0;                                    \
    }

GET_ID(D3DKMT_OPENADAPTERFROMLUID *, SYSTIMER_ID_OPENADAPTERFROMLUID)
GET_ID(CONST D3DKMT_CLOSEADAPTER *, SYSTIMER_ID_CLOSEADAPTER)
GET_ID(CONST D3DKMT_QUERYADAPTERINFO *, SYSTIMER_ID_QUERYADAPTERINFO)
GET_ID(CONST D3DKMT_ESCAPE *, SYSTIMER_ID_ESCAPE)
GET_ID(D3DKMT_CREATEDEVICE *, SYSTIMER_ID_CREATEDEVICE)
GET_ID(CONST D3DKMT_DESTROYDEVICE *, SYSTIMER_ID_DESTROYDEVICE)
GET_ID(D3DKMT_CREATECONTEXT *, SYSTIMER_ID_CREATECONTEXT)
GET_ID(CONST D3DKMT_DESTROYCONTEXT *, SYSTIMER_ID_DESTROYCONTEXT)
GET_ID(D3DKMT_CREATEALLOCATION *, SYSTIMER_ID_CREATEALLOCATION)
GET_ID(D3DKMT_OPENRESOURCE *, SYSTIMER_ID_OPENRESOURCE)
GET_ID(D3DKMT_QUERYRESOURCEINFO *, SYSTIMER_ID_QUERYRESOURCEINFO)
GET_ID(D3DKMT_CREATESYNCHRONIZATIONOBJECT *, SYSTIMER_ID_CREATESYNCHRONIZATIONOBJECT)
GET_ID(CONST D3DKMT_DESTROYSYNCHRONIZATIONOBJECT *, SYSTIMER_ID_DESTROYSYNCHRONIZATIONOBJECT)
GET_ID(CONST D3DKMT_SIGNALSYNCHRONIZATIONOBJECT *, SYSTIMER_ID_SIGNALSYNCHRONIZATIONOBJECT)
GET_ID(CONST_FROM_WDK_10_0_18328_0 D3DKMT_WAITFORSYNCHRONIZATIONOBJECT *, SYSTIMER_ID_WAITFORSYNCHRONIZATIONOBJECT)
GET_ID(D3DKMT_CREATESYNCHRONIZATIONOBJECT2 *, SYSTIMER_ID_CREATESYNCHRONIZATIONOBJECT2)
GET_ID(D3DKMT_GETDEVICESTATE *, SYSTIMER_ID_GETDEVICESTATE)
GET_ID(D3DDDI_MAKERESIDENT *, SYSTIMER_ID_MAKERESIDENT)
GET_ID(D3DKMT_EVICT *, SYSTIMER_ID_EVICT)
GET_ID(CONST D3DKMT_WAITFORSYNCHRONIZATIONOBJECTFROMCPU *, SYSTIMER_ID_WAITFORSYNCHRONIZATIONOBJECTFROMCPU)
GET_ID(CONST D3DKMT_SIGNALSYNCHRONIZATIONOBJECTFROMCPU *, SYSTIMER_ID_SIGNALSYNCHRONIZATIONOBJECTFROMCPU)
GET_ID(CONST D3DKMT_WAITFORSYNCHRONIZATIONOBJECTFROMGPU *, SYSTIMER_ID_WAITFORSYNCHRONIZATIONOBJECTFROMGPU)
GET_ID(CONST D3DKMT_SIGNALSYNCHRONIZATIONOBJECTFROMGPU *, SYSTIMER_ID_SIGNALSYNCHRONIZATIONOBJECTFROMGPU)
GET_ID(D3DKMT_CREATEPAGINGQUEUE *, SYSTIMER_ID_CREATEPAGINGQUEUE)
GET_ID(D3DDDI_DESTROYPAGINGQUEUE *, SYSTIMER_ID_D3DDDI_DESTROYPAGINGQUEUE)
GET_ID(D3DKMT_LOCK2 *, SYSTIMER_ID_LOCK2)
GET_ID(CONST D3DKMT_UNLOCK2 *, SYSTIMER_ID_UNLOCK2)
GET_ID(CONST D3DKMT_INVALIDATECACHE *, SYSTIMER_ID_INVALIDATECACHE)
GET_ID(D3DDDI_MAPGPUVIRTUALADDRESS *, SYSTIMER_ID_D3DDDI_MAPGPUVIRTUALADDRESS)
GET_ID(D3DDDI_RESERVEGPUVIRTUALADDRESS *, SYSTIMER_ID_D3DDDI_RESERVEGPUVIRTUALADDRESS)
GET_ID(CONST D3DKMT_FREEGPUVIRTUALADDRESS *, SYSTIMER_ID_FREEGPUVIRTUALADDRESS)
GET_ID(CONST D3DKMT_UPDATEGPUVIRTUALADDRESS *, SYSTIMER_ID_UPDATEGPUVIRTUALADDRESS)
GET_ID(D3DKMT_CREATECONTEXTVIRTUAL *, SYSTIMER_ID_CREATECONTEXTVIRTUAL)
GET_ID(CONST D3DKMT_SUBMITCOMMAND *, SYSTIMER_ID_SUBMITCOMMAND)
GET_ID(D3DKMT_OPENSYNCOBJECTFROMNTHANDLE2 *, SYSTIMER_ID_OPENSYNCOBJECTFROMNTHANDLE2)
GET_ID(CONST D3DKMT_DESTROYALLOCATION2 *, SYSTIMER_ID_DESTROYALLOCATION2)
GET_ID(D3DKMT_REGISTERTRIMNOTIFICATION *, SYSTIMER_ID_REGISTERTRIMNOTIFICATION)
GET_ID(D3DKMT_UNREGISTERTRIMNOTIFICATION *, SYSTIMER_ID_UNREGISTERTRIMNOTIFICATION)
GET_ID(D3DKMT_OPENRESOURCEFROMNTHANDLE *, SYSTIMER_ID_OPENRESOURCEFROMNTHANDLE)
GET_ID(D3DKMT_QUERYRESOURCEINFOFROMNTHANDLE *, SYSTIMER_ID_QUERYRESOURCEINFOFROMNTHANDLE)
GET_ID(D3DKMT_CREATEHWQUEUE *, SYSTIMER_ID_CREATEHWQUEUE)
GET_ID(CONST D3DKMT_DESTROYHWQUEUE *, SYSTIMER_ID_DESTROYHWQUEUE)
GET_ID(CONST D3DKMT_SUBMITCOMMANDTOHWQUEUE *, SYSTIMER_ID_SUBMITCOMMANDTOHWQUEUE)
GET_ID(CONST D3DKMT_SETALLOCATIONPRIORITY *, SYSTIMER_ID_SETALLOCATIONPRIORITY)

template <typename Param>
class ThkWrapper {
    typedef NTSTATUS(APIENTRY *Func)(Param);

  public:
    Func mFunc = nullptr;

    inline NTSTATUS operator()(Param param) const {
        if (KMD_PROFILING) {
            SYSTEM_ENTER()
            NTSTATUS Status;
            Status = mFunc(param);
            SYSTEM_LEAVE(getId<Param>());
            return Status;
        } else {
            return mFunc(param);
        }
    }

    ThkWrapper &operator=(void *func) {
        mFunc = reinterpret_cast<decltype(mFunc)>(func);
        return *this;
    }

    // This operator overload is for implicit casting ThkWrapper struct to Function Pointer in GetPfn methods like GetEscapePfn() or for comparing against NULL function pointer
    operator Func() const {
        return mFunc;
    }
};

} // namespace NEO
