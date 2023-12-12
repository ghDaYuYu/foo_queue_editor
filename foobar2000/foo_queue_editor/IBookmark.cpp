#include "stdafx.h"
#include "IBookmark.h"

namespace ibom {

	HRESULT __stdcall IBom::QueryInterface(REFIID riid, void** ppvObject)
	{
		return E_NOTIMPL;
	}

	ULONG __stdcall IBom::AddRef(void)
	{
		return 0;
	}

	ULONG __stdcall IBom::Release(void)
	{
		return 0;
	}

	HRESULT __stdcall IBom::GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::QueryGetData(FORMATETC* pformatetc)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::DUnadvise(DWORD dwConnection)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall IBom::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
	{
		return E_NOTIMPL;
	}
}