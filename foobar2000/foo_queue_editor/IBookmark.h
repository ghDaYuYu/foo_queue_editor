#pragma once
#include <atlcomcli.h>
#include <atlsafe.h>
#include <WTypesbase.h>
//
namespace ibom {

	// Convert from STL wstring to the ATL BSTR wrapper
	inline CComBSTR ToBstr(const std::wstring& s)
	{
		// Special case of empty string
		if (s.empty())
		{
			return CComBSTR();
		}
		return CComBSTR(static_cast<int>(s.size()), s.data());
	}

#define MAX_GUIDS 255

	class IBom : public IDataObject {

	public:
		IBom() {
			//..
		}

		~IBom() {
			//..
		}

		//operator const IDataObject* ()
		//{
		//	m_data = CComSafeArray<BSTR>();
		//	return this;
		//}

		//operator IDataObject* ()
		//{
		//	m_data = CComSafeArray<BSTR>();
		//	return this;
		//}

		//operator IBom* ()
		//{
		//	m_data = CComSafeArray<BSTR>();
		//	return this;
		//}

		//IBom operator  *   (IDataObject a/*, const IBom& b*/) {
		//	m_data = CComSafeArray<BSTR>();
		//	return *this;
		//}

		//operator IBom() const
		//{
		//	return *this;
		//}

		//operator IBom* () const
		//{
		//	return *this;
		//}

		//operator IBom()
		//{
		//	return *this;
		//}

		//operator IBom* ()
		//{
		//	return *this;
		//}

		//operator IDataObject& () {
		//	return *this;
		//};

		static void GetDecoClassName(std::wstring& out) {

			CComBSTR cbstr = ibom::IBom::get_deco_class_name();
			out = cbstr.Detach();
			/*if (auto p_bstr = cbstr.Detach()) {
				if (p_bstr) {
					out = std::wstring(p_bstr, cbstr.Length());
				}
			}*/
		}

		static size_t GetVer() {
			return 1;
		}

		static bool isBookmarkDeco(IDataObject* obj) {

			bool bres = false;

			//L"vbookmark data object"
			std::wstring mydeco;
			ibom::IBom::GetDecoClassName(mydeco);

			pfc::com_ptr_t <IEnumFORMATETC> pEnumFORMATETC;
			obj->EnumFormatEtc(DATADIR_GET, pEnumFORMATETC.receive_ptr());

			FORMATETC fe;
			memset(&fe, 0, sizeof(fe));
			while (S_OK == pEnumFORMATETC->Next(1, &fe, NULL))
			{

				if (fe.cfFormat == CF_TEXT /*|| fe.cfFormat == CF_UNICODETEXT*/) {
					UINT cb = static_cast<UINT>(sizeof(mydeco[0]) * (mydeco.length() + 1)); //44
					STGMEDIUM stgm;
					memset(&stgm, 0, sizeof(stgm));
					obj->GetData(&fe, &stgm);

					bool bcheck = false;
					char* data = (char*)GlobalLock(stgm.hGlobal);

					if (data) {
						pfc::stringcvt::string_utf8_from_wide to_str(mydeco.c_str());
						bres = !_stricmp(data, to_str.get_ptr());
					}

					GlobalUnlock(stgm.hGlobal);
					ReleaseStgMedium(&stgm);

					if (bres) {
						//found
						break;
					}
				}
			}

			pEnumFORMATETC.release();

			return bres;
		}

		void SetPlaylistGuids(std::vector<std::wstring> v) {
			SetArray(v);
		}

		LPSAFEARRAY GetPlaylistGuids() {
			return GetArray();
		}

		// Inherited via IDataObject
		virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;

		virtual ULONG __stdcall AddRef(void) override;

		virtual ULONG __stdcall Release(void) override;

		virtual HRESULT __stdcall GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium) override;

		virtual HRESULT __stdcall GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) override;

		virtual HRESULT __stdcall QueryGetData(FORMATETC* pformatetc) override;

		virtual HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut) override;

		virtual HRESULT __stdcall SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease) override;

		virtual HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) override;

		virtual HRESULT __stdcall DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override;

		virtual HRESULT __stdcall DUnadvise(DWORD dwConnection) override;

		virtual HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppenumAdvise) override;

	private:

		static const BSTR get_deco_class_name() {
			return L"vbookmark data object";
		}

		void SetArray(std::vector<std::wstring> v) {
			m_data.m_psa = nullptr;
			
			int cv = static_cast<int>(v.size());

			SAFEARRAYBOUND bounds[1];
			//bounds[0].cElements = 1;
			bounds[0].cElements = 0;
			bounds[0].lLbound = 0;

			//m_data.Create(bounds, cv);

			for (LONG i = 0; (i < MAX_GUIDS) && (i < static_cast<LONG>(v.size())); i++)
			{
				CComBSTR bstr = ToBstr(v[i]);
				HRESULT hr;
				//if (!i) {
				//	hr = m_data.SetAt(0, bstr);
				//}
				//else {
					hr = m_data.Add(bstr);
				//}
				if (FAILED(hr))
				{
					AtlThrow(hr);
				}
			}
		}

		LPSAFEARRAY GetArray() {
			return m_data.Detach();
		}

	private:
		pfc::com_ptr_t<IDataObject> m_cp;

		CComSafeArray<BSTR> m_data;
		bool init = false;
	};

}
