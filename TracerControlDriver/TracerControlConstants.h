#pragma once

#include "stdafx.h"

typedef CONST UNICODE_STRING TRACER_CONTROL_DEVICE_NAME;
typedef CONST UNICODE_STRING TRACER_CONTROL_WIN32_DEVICE_NAME;

static TRACER_CONTROL_DEVICE_NAME TracerControlDeviceName = \
    RTL_CONSTANT_STRING(L"\\Device\\TracerControlDriver");

static TRACER_CONTROL_WIN32_DEVICE_NAME TracerControlWin32DeviceName = \
    RTL_CONSTANT_STRING(L"\\Global??\\TracerControl");

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
