#include "Globals.h"
#include "ClassFactory.h"
#include <new>

// Must match the CLSID registered by the MSI package
// (HKCR\CLSID\{...} and the shellex / PreviewHandlers entries).
// Generate your own GUID for a real project.
// {CC12C1A5-7B5B-4367-8D3E-EBCEF1D697B9}
const CLSID CLSID_XyzPreviewHandler =
    { 0xCC12C1A5, 0x7B5B, 0x4367, { 0x8D, 0x3E, 0xEB, 0xCE, 0xF1, 0xD6, 0x97, 0xB9 } };

HINSTANCE g_hModule = nullptr;
LONG      g_cDllRef = 0;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (!IsEqualCLSID(rclsid, CLSID_XyzPreviewHandler))
        return CLASS_E_CLASSNOTAVAILABLE;

    CClassFactory *pFactory = new (std::nothrow) CClassFactory();
    if (!pFactory) return E_OUTOFMEMORY;

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}


STDAPI DllCanUnloadNow()
{
    return (g_cDllRef > 0) ? S_FALSE : S_OK;
}


// Registration is done declaratively by the MSI package (Registry table),
// not by self-registration (regsvr32), so these are intentionally no-ops.
STDAPI DllRegisterServer()
{
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    return S_OK;
}
