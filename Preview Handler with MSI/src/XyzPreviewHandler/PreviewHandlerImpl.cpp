#include "PreviewHandlerImpl.h"
#include "Globals.h"
#include <new>

CXyzPreviewHandler::CXyzPreviewHandler()
    : m_refCount(1)
    , m_pStream(nullptr)
    , m_pSite(nullptr)
    , m_hwndParent(nullptr)
    , m_hwndEdit(nullptr)
    , m_hFont(nullptr)
{
    m_rcParent = {0, 0, 0, 0};
    InterlockedIncrement(&g_cDllRef);
}


CXyzPreviewHandler::~CXyzPreviewHandler()
{
    DestroyPreviewWindow();
    ReleaseStream();

    if (m_pSite)
    {
        m_pSite->Release();
        m_pSite = nullptr;
    }
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = nullptr;
    }

    InterlockedDecrement(&g_cDllRef);
}


HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::QueryInterface(REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPreviewHandler))
        *ppv = static_cast<IPreviewHandler*>(this);
    else if (IsEqualIID(riid, IID_IPreviewHandlerVisuals))
        *ppv = static_cast<IPreviewHandlerVisuals*>(this);
    else if (IsEqualIID(riid, IID_IObjectWithSite))
        *ppv = static_cast<IObjectWithSite*>(this);
    else if (IsEqualIID(riid, IID_IOleWindow))
        *ppv = static_cast<IOleWindow*>(this);
    else if (IsEqualIID(riid, IID_IInitializeWithStream))
        *ppv = static_cast<IInitializeWithStream*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}


ULONG STDMETHODCALLTYPE CXyzPreviewHandler::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}


ULONG STDMETHODCALLTYPE CXyzPreviewHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_refCount);
    if (cRef == 0)
        delete this;
    return static_cast<ULONG>(cRef);
}




HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::Initialize(IStream *pStream, DWORD /*grfMode*/)
{
    if (!pStream) return E_INVALIDARG;

    ReleaseStream();
    m_pStream = pStream;
    m_pStream->AddRef();
    return S_OK;
}




HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::SetWindow(HWND hwnd, const RECT *prc)
{
    if (!hwnd || !prc) return E_INVALIDARG;

    if (m_hwndEdit && m_hwndParent != hwnd)
        DestroyPreviewWindow();

    m_hwndParent = hwnd;
    m_rcParent = *prc;

    UpdateEditBounds();
    return S_OK;
}



HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::SetRect(const RECT *prc)
{
    if (!prc) return E_INVALIDARG;

    m_rcParent = *prc;

    UpdateEditBounds();
    return S_OK;
}




void CXyzPreviewHandler::UpdateEditBounds()
{
    if (!m_hwndEdit) return;

    SetWindowPos(m_hwndEdit, nullptr,
                 m_rcParent.left, m_rcParent.top,
                 m_rcParent.right - m_rcParent.left,
                 m_rcParent.bottom - m_rcParent.top,
                 SWP_NOZORDER | SWP_NOACTIVATE);

    RECT rcClient;
    GetClientRect(m_hwndEdit, &rcClient);
    
    SendMessageW(m_hwndEdit, EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcClient));
    RedrawWindow(m_hwndEdit, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
}




HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::DoPreview()
{
    if (!m_hwndParent) return E_FAIL;

    if (!m_hwndEdit)
    {
        m_hwndEdit = CreateWindowExW(
            0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP |
                ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_LEFT,
            m_rcParent.left, m_rcParent.top,
            m_rcParent.right - m_rcParent.left,
            m_rcParent.bottom - m_rcParent.top,
            m_hwndParent, nullptr, g_hModule, nullptr);

        if (!m_hwndEdit)
            return HRESULT_FROM_WIN32(GetLastError());

        if (m_hFont)
            SendMessageW(m_hwndEdit, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);
    }

    LoadStreamTextIntoControl();
    UpdateEditBounds();
    ShowWindow(m_hwndEdit, SW_SHOW);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::Unload()
{
    DestroyPreviewWindow();
    ReleaseStream();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::SetFocus()
{
    if (!m_hwndEdit) return S_FALSE;
    ::SetFocus(m_hwndEdit);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::QueryFocus(HWND *phwnd)
{
    if (!phwnd) return E_POINTER;
    *phwnd = GetFocus();
    return (*phwnd != nullptr) ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::TranslateAccelerator(MSG * /*pmsg*/)
{
    return S_FALSE;
}


HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::SetSite(IUnknown *pUnkSite)
{
    if (m_pSite)
    {
        m_pSite->Release();
        m_pSite = nullptr;
    }
    if (pUnkSite)
    {
        m_pSite = pUnkSite;
        m_pSite->AddRef();
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::GetSite(REFIID riid, void **ppvSite)
{
    if (!ppvSite) return E_POINTER;
    *ppvSite = nullptr;
    if (!m_pSite) return E_FAIL;
    return m_pSite->QueryInterface(riid, ppvSite);
}


HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::GetWindow(HWND *phwnd)
{
    if (!phwnd) return E_INVALIDARG;
    *phwnd = m_hwndParent;
    return m_hwndParent ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::ContextSensitiveHelp(BOOL /*fEnterMode*/)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::SetBackgroundColor(COLORREF /*color*/)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::SetFont(const LOGFONTW *plf)
{
    if (!plf) return E_INVALIDARG;

    HFONT hNewFont = CreateFontIndirectW(plf);
    if (!hNewFont) return E_FAIL;

    if (m_hFont)
        DeleteObject(m_hFont);
    m_hFont = hNewFont;

    if (m_hwndEdit)
        SendMessageW(m_hwndEdit, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXyzPreviewHandler::SetTextColor(COLORREF /*color*/)
{
    return S_OK;
}

void CXyzPreviewHandler::ReleaseStream()
{
    if (m_pStream)
    {
        m_pStream->Release();
        m_pStream = nullptr;
    }
}

void CXyzPreviewHandler::DestroyPreviewWindow()
{
    if (m_hwndEdit)
    {
        DestroyWindow(m_hwndEdit);
        m_hwndEdit = nullptr;
    }
}

void CXyzPreviewHandler::LoadStreamTextIntoControl()
{
    if (!m_hwndEdit || !m_pStream) return;

    LARGE_INTEGER liZero{};
    m_pStream->Seek(liZero, STREAM_SEEK_SET, nullptr);

    const DWORD MAX_BYTES = 256 * 1024;
    BYTE *buffer = new (std::nothrow) BYTE[MAX_BYTES];
    if (!buffer)
    {
        SetWindowTextW(m_hwndEdit, L"(out of memory)");
        return;
    }

    ULONG bytesRead = 0;
    HRESULT hr = m_pStream->Read(buffer, MAX_BYTES, &bytesRead);

    if (FAILED(hr) && hr != S_FALSE)
    {
        SetWindowTextW(m_hwndEdit, L"(unable to read this file for preview)");
        delete[] buffer;
        return;
    }

    BYTE *pStart = buffer;
    ULONG len = bytesRead;

    if (len >= 3 && pStart[0] == 0xEF && pStart[1] == 0xBB && pStart[2] == 0xBF)
    {
        pStart += 3;
        len -= 3;
    }



    int wideLen = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(pStart),
                                       static_cast<int>(len), nullptr, 0);
    if (wideLen <= 0)
    {
        SetWindowTextW(m_hwndEdit, L"(preview not available for this file's content)");
        delete[] buffer;
        return;
    }

    wchar_t *wideBuf = new (std::nothrow) wchar_t[static_cast<size_t>(wideLen) + 1];
    // Worst case every character is a lone '\n' or '\r' that becomes "\r\n".
    wchar_t *textBuf = new (std::nothrow) wchar_t[static_cast<size_t>(wideLen) * 2 + 1];
    if (!wideBuf || !textBuf)
    {
        SetWindowTextW(m_hwndEdit, L"(out of memory)");
        delete[] wideBuf;
        delete[] textBuf;
        delete[] buffer;
        return;
    }



    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(pStart),
                         static_cast<int>(len), wideBuf, wideLen);

    
    size_t out = 0;
    for (int i = 0; i < wideLen; ++i)
    {
        wchar_t ch = wideBuf[i];
        if (ch == L'\r')
        {
            if (i + 1 < wideLen && wideBuf[i + 1] == L'\n')
                ++i;                       // "\r\n" -> keep as one break
            textBuf[out++] = L'\r';
            textBuf[out++] = L'\n';
        }
        else if (ch == L'\n')
        {
            textBuf[out++] = L'\r';
            textBuf[out++] = L'\n';
        }
        else if (ch != L'\0')
        {
            textBuf[out++] = ch;           // embedded NULs would truncate the text
        }
    }
    textBuf[out] = L'\0';

    SetWindowTextW(m_hwndEdit, textBuf);

    delete[] textBuf;
    delete[] wideBuf;
    delete[] buffer;
}



HRESULT CXyzPreviewHandler_CreateInstance(REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    CXyzPreviewHandler *pHandler = new (std::nothrow) CXyzPreviewHandler();
    if (!pHandler) return E_OUTOFMEMORY;

    HRESULT hr = pHandler->QueryInterface(riid, ppv);
    pHandler->Release();
    return hr;
}
