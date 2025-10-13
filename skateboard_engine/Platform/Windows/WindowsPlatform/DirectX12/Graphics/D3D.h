#pragma once

#pragma comment(lib,"d3d12")
#pragma comment(lib,"dxgi")
#pragma comment(lib,"dxguid")
#pragma comment(lib,"dxcompiler")

#include <wrl.h>
#include <comdef.h>

#include "d3d12.h"
#include "dxgi1_6.h"
#include <dxcapi.h>
#include <d3d12shader.h> // Contains functions and structures useful in accessing shader information.
#include <d3dx12.h>

//#include <DirectXMath.h>

#include <string>

#ifndef D3D_CHECK_FAILURE
#define D3D_CHECK_FAILURE(x) ASSERT_SIMPLE(SUCCEEDED(x), SKTBD_LOG_CRITICAL("D3D",_com_error(x).ErrorMessage()));
#endif

#ifndef ROUND_UP
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
#endif

struct DXException : public std::exception
{
	HRESULT ErrorCode = S_OK;

	DXException() = default;
	DXException(HRESULT hr)
		: ErrorCode(hr)
	{}

	std::wstring ToString() const
	{
		return _com_error(ErrorCode).ErrorMessage();
	}
};