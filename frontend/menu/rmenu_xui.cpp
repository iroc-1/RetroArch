/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2013 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2013 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <crtdefs.h>
#include <tchar.h>
#include <xtl.h>
#include <xui.h>
#include <xuiapp.h>

#include "rmenu_xui.h"

#include "utils/file_browser.h"

#include "../../console/rarch_console.h"

#include "../../gfx/gfx_common.h"
#include "../../gfx/gfx_context.h"

#include "../../message.h"

#include "../../general.h"

enum {
   MENU_XUI_ITEM_LOAD_STATE = 0,
   MENU_XUI_ITEM_SAVE_STATE,
   MENU_XUI_ITEM_ASPECT_RATIO,
   MENU_XUI_ITEM_ORIENTATION,
   MENU_XUI_ITEM_RESIZE_MODE,
   MENU_XUI_ITEM_FRAME_ADVANCE,
   MENU_XUI_ITEM_RESET,
   MENU_XUI_ITEM_RETURN_TO_GAME,
   MENU_XUI_ITEM_QUIT_RARCH,
};

enum
{
   S_LBL_ASPECT_RATIO = 0,
   S_LBL_RARCH_VERSION,
   S_LBL_ROTATION,
   S_LBL_LOAD_STATE_SLOT,
   S_LBL_SAVE_STATE_SLOT,
   S_LBL_REWIND_GRANULARITY,
};

HXUIOBJ m_menulist;
HXUIOBJ m_menutitle;
HXUIOBJ m_menutitlebottom;
HXUIOBJ m_back;

class CRetroArch : public CXuiModule
{
   public:
      HXUIOBJ hMainScene;
      HXUIOBJ hControlsMenu;
      HXUIOBJ hFileBrowser;
      HXUIOBJ hCoreBrowser;
      HXUIOBJ hShaderBrowser;
      HXUIOBJ hQuickMenu;
      HXUIOBJ hRetroArchSettings;
   protected:
      virtual HRESULT RegisterXuiClasses();
      virtual HRESULT UnregisterXuiClasses();
};

class CRetroArchMain: public CXuiSceneImpl
{
   public:
      HRESULT OnInit( XUIMessageInit* pInitData, int & bHandled );
      HRESULT OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled );
      HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);

    HRESULT DispatchMessageMap(XUIMessage *pMessage)
    {
	    if (pMessage->dwMessage == XM_INIT)
	    {
		    XUIMessageInit *pData = (XUIMessageInit *) pMessage->pvData;
		    return OnInit(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_CONTROL_NAVIGATE)
	    {
		    XUIMessageControlNavigate *pData = (XUIMessageControlNavigate *) pMessage->pvData;
		    return OnControlNavigate(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_NOTIFY)
	    {
		    XUINotify *pNotify = (XUINotify *) pMessage->pvData;
		    if (pNotify->dwNotify == XN_PRESS)
			    return OnNotifyPress(pNotify->hObjSource, pMessage->bHandled);
	    }
        return __super::DispatchMessageMap(pMessage);
    }

    static HRESULT Register()
    {
        HXUICLASS hClass;
        XUIClass cls;
        memset(&cls, 0x00, sizeof(cls));
        cls.szClassName = L"RetroArchMain";
        cls.szBaseClassName = XUI_CLASS_SCENE;
        cls.Methods.CreateInstance = (PFN_CREATEINST) (CreateInstance);
        cls.Methods.DestroyInstance = (PFN_DESTROYINST) DestroyInstance;
        cls.Methods.ObjectProc = (PFN_OBJECT_PROC) _ObjectProc;
        cls.pPropDefs = _GetPropDef(&cls.dwPropDefCount);
        HRESULT hr = XuiRegisterClass(&cls, &hClass);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }
   
    static HRESULT APIENTRY CreateInstance(HXUIOBJ hObj, void **ppvObj)
    {
        *ppvObj = NULL;
        CRetroArchMain *pThis = new CRetroArchMain();
        if (!pThis)
            return E_OUTOFMEMORY;
        pThis->m_hObj = hObj;
        HRESULT hr = pThis->OnCreate();
        if (FAILED(hr))
        {
            DestroyInstance(pThis);
            return hr;
        }
        *ppvObj = pThis;
        return S_OK;
    }
    
    static HRESULT APIENTRY DestroyInstance(void *pvObj)
    {
        CRetroArchMain *pThis = (CRetroArchMain *) pvObj;
        delete pThis;
        return S_OK;
    }
};

class CRetroArchFileBrowser: public CXuiSceneImpl
{
   protected:

      HXUIOBJ m_dir_game;
   public:
      HRESULT OnInit( XUIMessageInit* pInitData, int & bHandled );
      HRESULT OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled );

    HRESULT DispatchMessageMap(XUIMessage *pMessage)
    {
	    if (pMessage->dwMessage == XM_INIT)
	    {
		    XUIMessageInit *pData = (XUIMessageInit *) pMessage->pvData;
		    return OnInit(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_NOTIFY)
	    {
		    XUINotify *pNotify = (XUINotify *) pMessage->pvData;
		    if (pNotify->dwNotify == XN_PRESS)
			    return OnNotifyPress(pNotify->hObjSource, pMessage->bHandled);
	    }
        return __super::DispatchMessageMap(pMessage);
    }

    static HRESULT Register()
    {
        HXUICLASS hClass;
        XUIClass cls;
        memset(&cls, 0x00, sizeof(cls));
        cls.szClassName = L"RetroArchFileBrowser";
        cls.szBaseClassName = XUI_CLASS_SCENE;
        cls.Methods.CreateInstance = (PFN_CREATEINST) (CreateInstance);
        cls.Methods.DestroyInstance = (PFN_DESTROYINST) DestroyInstance;
        cls.Methods.ObjectProc = (PFN_OBJECT_PROC) _ObjectProc;
        cls.pPropDefs = _GetPropDef(&cls.dwPropDefCount);
        HRESULT hr = XuiRegisterClass(&cls, &hClass);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }
   
    static HRESULT APIENTRY CreateInstance(HXUIOBJ hObj, void **ppvObj)
    {
        *ppvObj = NULL;
        CRetroArchFileBrowser *pThis = new CRetroArchFileBrowser();
        if (!pThis)
            return E_OUTOFMEMORY;
        pThis->m_hObj = hObj;
        HRESULT hr = pThis->OnCreate();
        if (FAILED(hr))
        {
            DestroyInstance(pThis);
            return hr;
        }
        *ppvObj = pThis;
        return S_OK;
    }
    
    static HRESULT APIENTRY DestroyInstance(void *pvObj)
    {
        CRetroArchFileBrowser *pThis = (CRetroArchFileBrowser *) pvObj;
        delete pThis;
        return S_OK;
    }
};

class CRetroArchCoreBrowser: public CXuiSceneImpl
{
   public:
      HRESULT OnInit( XUIMessageInit* pInitData, int & bHandled );
      HRESULT OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled );

    HRESULT DispatchMessageMap(XUIMessage *pMessage)
    {
	    if (pMessage->dwMessage == XM_INIT)
	    {
		    XUIMessageInit *pData = (XUIMessageInit *) pMessage->pvData;
		    return OnInit(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_NOTIFY)
	    {
		    XUINotify *pNotify = (XUINotify *) pMessage->pvData;
		    if (pNotify->dwNotify == XN_PRESS)
			    return OnNotifyPress(pNotify->hObjSource, pMessage->bHandled);
	    }
        return __super::DispatchMessageMap(pMessage);
    }

    static HRESULT Register()
    {
        HXUICLASS hClass;
        XUIClass cls;
        memset(&cls, 0x00, sizeof(cls));
        cls.szClassName = L"RetroArchCoreBrowser";
        cls.szBaseClassName = XUI_CLASS_SCENE;
        cls.Methods.CreateInstance = (PFN_CREATEINST) (CreateInstance);
        cls.Methods.DestroyInstance = (PFN_DESTROYINST) DestroyInstance;
        cls.Methods.ObjectProc = (PFN_OBJECT_PROC) _ObjectProc;
        cls.pPropDefs = _GetPropDef(&cls.dwPropDefCount);
        HRESULT hr = XuiRegisterClass(&cls, &hClass);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }
   
    static HRESULT APIENTRY CreateInstance(HXUIOBJ hObj, void **ppvObj)
    {
        *ppvObj = NULL;
        CRetroArchCoreBrowser *pThis = new CRetroArchCoreBrowser();
        if (!pThis)
            return E_OUTOFMEMORY;
        pThis->m_hObj = hObj;
        HRESULT hr = pThis->OnCreate();
        if (FAILED(hr))
        {
            DestroyInstance(pThis);
            return hr;
        }
        *ppvObj = pThis;
        return S_OK;
    }
    
    static HRESULT APIENTRY DestroyInstance(void *pvObj)
    {
        CRetroArchCoreBrowser *pThis = (CRetroArchCoreBrowser *) pvObj;
        delete pThis;
        return S_OK;
    }
};

class CRetroArchShaderBrowser: public CXuiSceneImpl
{
   public:
      HRESULT OnInit( XUIMessageInit* pInitData, int & bHandled );
      HRESULT OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled );

    HRESULT DispatchMessageMap(XUIMessage *pMessage)
    {
	    if (pMessage->dwMessage == XM_INIT)
	    {
		    XUIMessageInit *pData = (XUIMessageInit *) pMessage->pvData;
		    return OnInit(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_NOTIFY)
	    {
		    XUINotify *pNotify = (XUINotify *) pMessage->pvData;
		    if (pNotify->dwNotify == XN_PRESS)
			    return OnNotifyPress(pNotify->hObjSource, pMessage->bHandled);
	    }
        return __super::DispatchMessageMap(pMessage);
    }

    static HRESULT Register()
    {
        HXUICLASS hClass;
        XUIClass cls;
        memset(&cls, 0x00, sizeof(cls));
        cls.szClassName = L"RetroArchShaderBrowser";
        cls.szBaseClassName = XUI_CLASS_SCENE;
        cls.Methods.CreateInstance = (PFN_CREATEINST) (CreateInstance);
        cls.Methods.DestroyInstance = (PFN_DESTROYINST) DestroyInstance;
        cls.Methods.ObjectProc = (PFN_OBJECT_PROC) _ObjectProc;
        cls.pPropDefs = _GetPropDef(&cls.dwPropDefCount);
        HRESULT hr = XuiRegisterClass(&cls, &hClass);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }
   
    static HRESULT APIENTRY CreateInstance(HXUIOBJ hObj, void **ppvObj)
    {
        *ppvObj = NULL;
        CRetroArchShaderBrowser *pThis = new CRetroArchShaderBrowser();
        if (!pThis)
            return E_OUTOFMEMORY;
        pThis->m_hObj = hObj;
        HRESULT hr = pThis->OnCreate();
        if (FAILED(hr))
        {
            DestroyInstance(pThis);
            return hr;
        }
        *ppvObj = pThis;
        return S_OK;
    }
    
    static HRESULT APIENTRY DestroyInstance(void *pvObj)
    {
        CRetroArchShaderBrowser *pThis = (CRetroArchShaderBrowser *) pvObj;
        delete pThis;
        return S_OK;
    }
};

class CRetroArchQuickMenu: public CXuiSceneImpl
{
   public:
      HRESULT OnInit( XUIMessageInit* pInitData, int & bHandled );
      HRESULT OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled );
      HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);

    HRESULT DispatchMessageMap(XUIMessage *pMessage)
    {
	    if (pMessage->dwMessage == XM_INIT)
	    {
		    XUIMessageInit *pData = (XUIMessageInit *) pMessage->pvData;
		    return OnInit(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_CONTROL_NAVIGATE)
	    {
		    XUIMessageControlNavigate *pData = (XUIMessageControlNavigate *) pMessage->pvData;
		    return OnControlNavigate(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_NOTIFY)
	    {
		    XUINotify *pNotify = (XUINotify *) pMessage->pvData;
		    if (pNotify->dwNotify == XN_PRESS)
			    return OnNotifyPress(pNotify->hObjSource, pMessage->bHandled);
	    }
        return __super::DispatchMessageMap(pMessage);
    }

    static HRESULT Register()
    {
        HXUICLASS hClass;
        XUIClass cls;
        memset(&cls, 0x00, sizeof(cls));
        cls.szClassName = L"RetroArchQuickMenu";
        cls.szBaseClassName = XUI_CLASS_SCENE;
        cls.Methods.CreateInstance = (PFN_CREATEINST) (CreateInstance);
        cls.Methods.DestroyInstance = (PFN_DESTROYINST) DestroyInstance;
        cls.Methods.ObjectProc = (PFN_OBJECT_PROC) _ObjectProc;
        cls.pPropDefs = _GetPropDef(&cls.dwPropDefCount);
        HRESULT hr = XuiRegisterClass(&cls, &hClass);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }
   
    static HRESULT APIENTRY CreateInstance(HXUIOBJ hObj, void **ppvObj)
    {
        *ppvObj = NULL;
        CRetroArchQuickMenu *pThis = new CRetroArchQuickMenu();
        if (!pThis)
            return E_OUTOFMEMORY;
        pThis->m_hObj = hObj;
        HRESULT hr = pThis->OnCreate();
        if (FAILED(hr))
        {
            DestroyInstance(pThis);
            return hr;
        }
        *ppvObj = pThis;
        return S_OK;
    }
    
    static HRESULT APIENTRY DestroyInstance(void *pvObj)
    {
        CRetroArchQuickMenu *pThis = (CRetroArchQuickMenu *) pvObj;
        delete pThis;
        return S_OK;
    }
};

class CRetroArchSettings: public CXuiSceneImpl
{
   public:
      HRESULT OnInit( XUIMessageInit* pInitData, int & bHandled );
      HRESULT OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled );
      HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);

    HRESULT DispatchMessageMap(XUIMessage *pMessage)
    {
	    if (pMessage->dwMessage == XM_INIT)
	    {
		    XUIMessageInit *pData = (XUIMessageInit *) pMessage->pvData;
		    return OnInit(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_CONTROL_NAVIGATE)
	    {
		    XUIMessageControlNavigate *pData = (XUIMessageControlNavigate *) pMessage->pvData;
		    return OnControlNavigate(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_NOTIFY)
	    {
		    XUINotify *pNotify = (XUINotify *) pMessage->pvData;
		    if (pNotify->dwNotify == XN_PRESS)
			    return OnNotifyPress(pNotify->hObjSource, pMessage->bHandled);
	    }
        return __super::DispatchMessageMap(pMessage);
    }

    static HRESULT Register()
    {
        HXUICLASS hClass;
        XUIClass cls;
        memset(&cls, 0x00, sizeof(cls));
        cls.szClassName = L"RetroArchSettings";
        cls.szBaseClassName = XUI_CLASS_SCENE;
        cls.Methods.CreateInstance = (PFN_CREATEINST) (CreateInstance);
        cls.Methods.DestroyInstance = (PFN_DESTROYINST) DestroyInstance;
        cls.Methods.ObjectProc = (PFN_OBJECT_PROC) _ObjectProc;
        cls.pPropDefs = _GetPropDef(&cls.dwPropDefCount);
        HRESULT hr = XuiRegisterClass(&cls, &hClass);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }
   
    static HRESULT APIENTRY CreateInstance(HXUIOBJ hObj, void **ppvObj)
    {
        *ppvObj = NULL;
        CRetroArchSettings *pThis = new CRetroArchSettings();
        if (!pThis)
            return E_OUTOFMEMORY;
        pThis->m_hObj = hObj;
        HRESULT hr = pThis->OnCreate();
        if (FAILED(hr))
        {
            DestroyInstance(pThis);
            return hr;
        }
        *ppvObj = pThis;
        return S_OK;
    }
    
    static HRESULT APIENTRY DestroyInstance(void *pvObj)
    {
        CRetroArchSettings *pThis = (CRetroArchSettings *) pvObj;
        delete pThis;
        return S_OK;
    }
};

class CRetroArchControls: public CXuiSceneImpl
{
   public:
      HRESULT OnInit( XUIMessageInit* pInitData, int & bHandled );
      HRESULT OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled );
      HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);

    HRESULT DispatchMessageMap(XUIMessage *pMessage)
    {
	    if (pMessage->dwMessage == XM_INIT)
	    {
		    XUIMessageInit *pData = (XUIMessageInit *) pMessage->pvData;
		    return OnInit(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_CONTROL_NAVIGATE)
	    {
		    XUIMessageControlNavigate *pData = (XUIMessageControlNavigate *) pMessage->pvData;
		    return OnControlNavigate(pData, pMessage->bHandled);
	    }
	    if (pMessage->dwMessage == XM_NOTIFY)
	    {
		    XUINotify *pNotify = (XUINotify *) pMessage->pvData;
		    if (pNotify->dwNotify == XN_PRESS)
			    return OnNotifyPress(pNotify->hObjSource, pMessage->bHandled);
	    }
       return __super::DispatchMessageMap(pMessage);
    }

    static HRESULT Register()
    {
        HXUICLASS hClass;
        XUIClass cls;
        memset(&cls, 0x00, sizeof(cls));
        cls.szClassName = L"RetroArchControls";
        cls.szBaseClassName = XUI_CLASS_SCENE;
        cls.Methods.CreateInstance = (PFN_CREATEINST) (CreateInstance);
        cls.Methods.DestroyInstance = (PFN_DESTROYINST) DestroyInstance;
        cls.Methods.ObjectProc = (PFN_OBJECT_PROC) _ObjectProc;
        cls.pPropDefs = _GetPropDef(&cls.dwPropDefCount);
        HRESULT hr = XuiRegisterClass(&cls, &hClass);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }

    static HRESULT APIENTRY CreateInstance(HXUIOBJ hObj, void **ppvObj)
    {
        *ppvObj = NULL;
        CRetroArchControls *pThis = new CRetroArchControls();
        if (!pThis)
            return E_OUTOFMEMORY;
        pThis->m_hObj = hObj;
        HRESULT hr = pThis->OnCreate();
        if (FAILED(hr))
        {
            DestroyInstance(pThis);
            return hr;
        }
        *ppvObj = pThis;
        return S_OK;
    }
    
    static HRESULT APIENTRY DestroyInstance(void *pvObj)
    {
        CRetroArchControls *pThis = (CRetroArchControls *) pvObj;
        delete pThis;
        return S_OK;
    }
};

CRetroArch app;
HXUIOBJ m_list;
HXUIOBJ m_list_path;
HXUIOBJ hCur;

wchar_t strw_buffer[PATH_MAX];
char str_buffer[PATH_MAX];

static int process_input_ret = 0;

/* Register custom classes */
HRESULT CRetroArch::RegisterXuiClasses (void)
{
   CRetroArchMain::Register();
   CRetroArchFileBrowser::Register();
   CRetroArchCoreBrowser::Register();
   CRetroArchShaderBrowser::Register();
   CRetroArchQuickMenu::Register();
   CRetroArchControls::Register();
   CRetroArchSettings::Register();

   return 0;
}

/* Unregister custom classes */
HRESULT CRetroArch::UnregisterXuiClasses (void)
{
   XuiUnregisterClass(L"RetroArchMain");
   XuiUnregisterClass(L"RetroArchCoreBrowser");
   XuiUnregisterClass(L"RetroArchShaderBrowser");
   XuiUnregisterClass(L"RetroArchFileBrowser");
   XuiUnregisterClass(L"RetroArchQuickMenu");
   XuiUnregisterClass(L"RetroArchControls");
   XuiUnregisterClass(L"RetroArchSettings");

   return 0;
}

static void menu_settings_create_menu_item_label_w(wchar_t *strwbuf, unsigned setting, size_t size)
{
   char str[PATH_MAX];

   switch (setting)
   {
      case S_LBL_ASPECT_RATIO:
         snprintf(str, size, "Aspect Ratio: %s", aspectratio_lut[g_settings.video.aspect_ratio_idx].name);
         break;
      case S_LBL_RARCH_VERSION:
         snprintf(str, size, "RetroArch %s", PACKAGE_VERSION);
         break;
      case S_LBL_ROTATION:
         snprintf(str, size, "Rotation: %s", rotation_lut[g_extern.console.screen.orientation]);
         break;
      case S_LBL_LOAD_STATE_SLOT:
         snprintf(str, size, "Load State #%d", g_extern.state_slot);
         break;
      case S_LBL_SAVE_STATE_SLOT:
         snprintf(str, size, "Save State #%d", g_extern.state_slot);
         break;
      case S_LBL_REWIND_GRANULARITY:
         snprintf(str, size, "Rewind granularity: %d", g_settings.rewind_granularity);
         break;
   }

   mbstowcs(strwbuf, str, size / sizeof(wchar_t));
}

void filebrowser_fetch_directory_entries(uint64_t action)
{
   filebrowser_update(rgui->browser, action, rgui->browser->current_dir.extensions); 

   mbstowcs(strw_buffer, rgui->browser->current_dir.directory_path, sizeof(strw_buffer) / sizeof(wchar_t));
   XuiTextElementSetText(m_list_path, strw_buffer);

   XuiListDeleteItems(m_list, 0, XuiListGetItemCount(m_list));
   XuiListInsertItems(m_list, 0, rgui->browser->list->size);

   for(unsigned i = 0; i < rgui->browser->list->size; i++)
   {
      char fname_tmp[256];
      fill_pathname_base(fname_tmp, rgui->browser->list->elems[i].data, sizeof(fname_tmp));
      mbstowcs(strw_buffer, fname_tmp, sizeof(strw_buffer) / sizeof(wchar_t));
      XuiListSetText(m_list, i, strw_buffer);
   }
}

HRESULT CRetroArchFileBrowser::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
   GetChildById(L"XuiRomList", &m_list);
   GetChildById(L"XuiBackButton1", &m_back);
   GetChildById(L"XuiTxtRomPath", &m_list_path);
   GetChildById(L"XuiBtnGameDir", &m_dir_game);

   filebrowser_set_root_and_ext(rgui->browser, rgui->info.valid_extensions,
      default_paths.filebrowser_startup_dir);

   uint64_t action = (1ULL << DEVICE_NAV_B);
   filebrowser_fetch_directory_entries(action);

   return 0;
}

HRESULT CRetroArchFileBrowser::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
   char path[PATH_MAX];
   process_input_ret = 0;

   if(hObjPressed == m_list)
   {
      int index = XuiListGetCurSel(m_list, NULL);
      wcstombs(str_buffer, (const wchar_t *)XuiListGetText(m_list, index), sizeof(str_buffer));
      if (path_file_exists(rgui->browser->list->elems[index].data))
      {
         snprintf(g_extern.fullpath, sizeof(g_extern.fullpath), "%s\\%s",
               rgui->browser->current_dir.directory_path, str_buffer);
         g_extern.lifecycle_mode_state |= (1ULL << MODE_LOAD_GAME);
         process_input_ret = -1;
      }
      else if(rgui->browser->list->elems[index].attr.b)
      {
         snprintf(path, sizeof(path), "%s\\%s", rgui->browser->current_dir.directory_path, str_buffer);
         uint64_t action = (1ULL << DEVICE_NAV_B);
         filebrowser_set_root_and_ext(rgui->browser, rgui->info.valid_extensions, path);
         filebrowser_fetch_directory_entries(action);
      }
   }
   else if (hObjPressed == m_dir_game)
   {
      filebrowser_set_root_and_ext(rgui->browser, rgui->info.valid_extensions,
            g_settings.rgui_browser_directory);
      uint64_t action = (1ULL << DEVICE_NAV_B);
      filebrowser_fetch_directory_entries(action);
   }

   bHandled = TRUE;

   return 0;
}

static void set_dpad_emulation_label(unsigned port, char *str, size_t sizeof_str)
{
   switch(g_settings.input.dpad_emulation[port])
   {
      case ANALOG_DPAD_NONE:
         snprintf(str, sizeof_str, "D-Pad Emulation: None");
         break;
      case ANALOG_DPAD_LSTICK:
         snprintf(str, sizeof_str, "D-Pad Emulation: Left Stick");
         break;
      case ANALOG_DPAD_RSTICK:
         snprintf(str, sizeof_str, "D-Pad Emulation: Right Stick");
         break;
   }
}

static void init_menulist(unsigned menu_id)
{
   XuiListDeleteItems(m_menulist, 0, XuiListGetItemCount(m_menulist));

   switch (menu_id)
   {
      case INGAME_MENU_INPUT_OPTIONS_MODE:
         {
            unsigned i;
            char buttons[RARCH_FIRST_META_KEY][128];
   unsigned keybind_end = RETRO_DEVICE_ID_JOYPAD_R3 + 1;

   for(i = 0; i < keybind_end; i++)
   {
      struct platform_bind key_label;
      strlcpy(key_label.desc, "Unknown", sizeof(key_label.desc));
      key_label.joykey = g_settings.input.binds[rgui->current_pad][i].joykey;

      if (driver.input->set_keybinds)
         driver.input->set_keybinds(&key_label, 0, 0, 0, (1ULL << KEYBINDS_ACTION_GET_BIND_LABEL));

      snprintf(buttons[i], sizeof(buttons[i]), "%s #%d: %s", 
            g_settings.input.binds[rgui->current_pad][i].desc,
            rgui->current_pad, key_label.desc);
      mbstowcs(strw_buffer, buttons[i], sizeof(strw_buffer) / sizeof(wchar_t));
      XuiListInsertItems(m_menulist, i, 1);
      XuiListSetText(m_menulist, i, strw_buffer);
   }

   set_dpad_emulation_label(rgui->current_pad, buttons[0], sizeof(buttons[0]));
   mbstowcs(strw_buffer, buttons[0], sizeof(strw_buffer) / sizeof(wchar_t));
   XuiListInsertItems(m_menulist, keybind_end, 1);
   XuiListSetText(m_menulist, SETTING_CONTROLS_DPAD_EMULATION, strw_buffer);

   XuiListInsertItems(m_menulist, keybind_end + 1, 1);
   XuiListSetText(m_menulist, SETTING_CONTROLS_DEFAULT_ALL, L"Reset all buttons to default");
         }
         break;
      case INGAME_MENU_SETTINGS_MODE:
XuiListInsertItems(m_menulist, 0, 1);
   XuiListSetText(m_menulist, SETTING_EMU_REWIND_ENABLED, g_settings.rewind_enable ? L"Rewind: ON" : L"Rewind: OFF");

   menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_REWIND_GRANULARITY, sizeof(strw_buffer));
   XuiListInsertItems(m_menulist, 1, 1);
   XuiListSetText(m_menulist, SETTING_EMU_REWIND_GRANULARITY, strw_buffer);

   XuiListInsertItems(m_menulist, 2, 1);
   XuiListSetText(m_menulist, SETTING_EMU_SHOW_INFO_MSG, (g_extern.lifecycle_mode_state & (1ULL << MODE_INFO_DRAW)) ? L"Info Messages: ON" : L"Info Messages: OFF");
   XuiListInsertItems(m_menulist, 3, 1);
   XuiListSetText(m_menulist, SETTING_EMU_SHOW_DEBUG_INFO_MSG, (g_extern.lifecycle_mode_state & (1ULL << MODE_FPS_DRAW)) ? L"Debug Info Messages: ON" : L"Debug Info messages: OFF");
   XuiListInsertItems(m_menulist, 4, 1);
   XuiListSetText(m_menulist, SETTING_GAMMA_CORRECTION_ENABLED, g_extern.console.screen.gamma_correction ? L"Gamma Correction: ON" : L"Gamma correction: OFF");
   XuiListInsertItems(m_menulist, 5, 1);
   XuiListSetText(m_menulist, SETTING_HW_TEXTURE_FILTER, g_settings.video.smooth ? L"Default Filter: Linear" : L"Default Filter: Nearest");
   break;
      case INGAME_MENU_MAIN_MODE:
   XuiListInsertItems(m_menulist, INGAME_MENU_CHANGE_LIBRETRO_CORE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_CHANGE_LIBRETRO_CORE, L"Core ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_LOAD_GAME_HISTORY_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_LOAD_GAME_HISTORY_MODE, L"Load Game History ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_CHANGE_GAME, 1);
   XuiListSetText(m_menulist, INGAME_MENU_CHANGE_GAME, L"Load Game ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_CORE_OPTIONS_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_CORE_OPTIONS_MODE, L"Core Options ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_VIDEO_OPTIONS_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_VIDEO_OPTIONS_MODE, L"Video Options ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_AUDIO_OPTIONS_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_AUDIO_OPTIONS_MODE, L"Audio Options ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_INPUT_OPTIONS_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_INPUT_OPTIONS_MODE, L"Input Options ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_PATH_OPTIONS_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_PATH_OPTIONS_MODE, L"Path Options ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_SETTINGS_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_SETTINGS_MODE, L"Settings ...");

   XuiListInsertItems(m_menulist, INGAME_MENU_LOAD_STATE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_LOAD_STATE, L"Load State #");

   XuiListInsertItems(m_menulist, INGAME_MENU_SAVE_STATE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_SAVE_STATE, L"Save State #");

   XuiListInsertItems(m_menulist, INGAME_MENU_SCREENSHOT_MODE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_SCREENSHOT_MODE, L"Take Screenshot");

   XuiListInsertItems(m_menulist, INGAME_MENU_RETURN_TO_GAME, 1);
   XuiListSetText(m_menulist, INGAME_MENU_RETURN_TO_GAME, L"Resume Game");

   XuiListInsertItems(m_menulist, INGAME_MENU_RESET, 1);
   XuiListSetText(m_menulist, INGAME_MENU_RESET, L"Restart Game");

   XuiListInsertItems(m_menulist, INGAME_MENU_REWIND_ENABLED, 1);
   XuiListSetText(m_menulist, INGAME_MENU_REWIND_ENABLED, L"Rewind: ");

   XuiListInsertItems(m_menulist, INGAME_MENU_REWIND_GRANULARITY, 1);
   XuiListSetText(m_menulist, INGAME_MENU_REWIND_GRANULARITY, L"Rewind Granularity: ");

   XuiListInsertItems(m_menulist, INGAME_MENU_FRAME_ADVANCE, 1);
   XuiListSetText(m_menulist, INGAME_MENU_FRAME_ADVANCE, L"Frame Advance");

   XuiListInsertItems(m_menulist, INGAME_MENU_QUIT_RETROARCH, 1);
   XuiListSetText(m_menulist, INGAME_MENU_QUIT_RETROARCH, L"Quit RetroArch");
break;
   }
}

HRESULT CRetroArchControls::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
   GetChildById(L"XuiMenuList", &m_menulist);
   GetChildById(L"XuiBackButton", &m_back);
   GetChildById(L"XuiTxtTitle", &m_menutitle);

   XuiTextElementSetText(m_menutitle, L"Input options");

   init_menulist(INGAME_MENU_INPUT_OPTIONS_MODE);

   return 0;
}

HRESULT CRetroArchControls::OnControlNavigate(
      XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
   char button[128];
   char buttons[RARCH_FIRST_META_KEY][128];
   int i, current_index;

   current_index = XuiListGetCurSel(m_menulist, NULL);

   for(i = 0; i < RARCH_FIRST_META_KEY; i++)
   {
      struct platform_bind key_label;
      strlcpy(key_label.desc, "Unknown", sizeof(key_label.desc));
      key_label.joykey = g_settings.input.binds[rgui->current_pad][i].joykey;

      if (driver.input->set_keybinds)
         driver.input->set_keybinds(&key_label, 0, 0, 0, (1ULL << KEYBINDS_ACTION_GET_BIND_LABEL));

      snprintf(buttons[i], sizeof(buttons[i]), "%s #%d: %s", 
            g_settings.input.binds[rgui->current_pad][i].desc, rgui->current_pad, 
            key_label.desc);
      mbstowcs(strw_buffer, buttons[i], sizeof(strw_buffer) / sizeof(wchar_t));
      XuiListSetText(m_menulist, i, strw_buffer);
   }

   switch(pControlNavigateData->nControlNavigate)
   {
      case XUI_CONTROL_NAVIGATE_LEFT:
         switch(current_index)
         {
            case SETTING_CONTROLS_DPAD_EMULATION:
               if (driver.input->set_keybinds)
               {
                  unsigned keybind_action = 0;

                  switch(g_settings.input.dpad_emulation[rgui->current_pad])
                  {
                     case ANALOG_DPAD_NONE:
                        break;
                     case ANALOG_DPAD_LSTICK:
                        keybind_action = (1ULL << KEYBINDS_ACTION_SET_ANALOG_DPAD_NONE);
                        break;
                     case ANALOG_DPAD_RSTICK:
                        keybind_action = (1ULL << KEYBINDS_ACTION_SET_ANALOG_DPAD_LSTICK);
                        break;
                  }

                  if (keybind_action)
                     driver.input->set_keybinds(driver.input_data, g_settings.input.device[rgui->current_pad],
rgui->current_pad, 0, keybind_action);
               }
               break;
            case SETTING_CONTROLS_DEFAULT_ALL:
               break;
            default:
               {
                  struct platform_bind key_label;
                  strlcpy(key_label.desc, "Unknown", sizeof(key_label.desc));
                  key_label.joykey = g_settings.input.binds[rgui->current_pad][current_index].joykey;

                  if (driver.input->set_keybinds)
                     driver.input->set_keybinds(&key_label, 0, 0, 0, (1ULL << KEYBINDS_ACTION_GET_BIND_LABEL));

                  if (driver.input->set_keybinds)
                     driver.input->set_keybinds(driver.input_data, g_settings.input.device[rgui->current_pad],
                           rgui->current_pad, current_index, (1ULL << KEYBINDS_ACTION_DECREMENT_BIND));

                  snprintf(button, sizeof(button), "%s #%d: %s",
                        g_settings.input.binds[rgui->current_pad][current_index].desc,
rgui->current_pad, key_label.desc);
                  mbstowcs(strw_buffer, button, sizeof(strw_buffer) / sizeof(wchar_t));
                  XuiListSetText(m_menulist, current_index, strw_buffer);
               }
               break;
         }
         break;
      case XUI_CONTROL_NAVIGATE_RIGHT:
         switch(current_index)
         {
            case SETTING_CONTROLS_DPAD_EMULATION:
               if (driver.input->set_keybinds)
               {
                  unsigned keybind_action = 0;

                  switch(g_settings.input.dpad_emulation[rgui->current_pad])
                  {
                     case ANALOG_DPAD_NONE:
                        keybind_action = (1ULL << KEYBINDS_ACTION_SET_ANALOG_DPAD_LSTICK);
                        break;
                     case ANALOG_DPAD_LSTICK:
                        keybind_action = (1ULL << KEYBINDS_ACTION_SET_ANALOG_DPAD_RSTICK);
                        break;
                     case ANALOG_DPAD_RSTICK:
                        break;
                  }

                  if (keybind_action)
                     driver.input->set_keybinds(driver.input_data, g_settings.input.device[rgui->current_pad],
rgui->current_pad, 0, keybind_action);
               }
               break;
            case SETTING_CONTROLS_DEFAULT_ALL:
               break;
            default:
               {
                  struct platform_bind key_label;
                  strlcpy(key_label.desc, "Unknown", sizeof(key_label.desc));
                  key_label.joykey = g_settings.input.binds[rgui->current_pad][current_index].joykey;

                  if (driver.input->set_keybinds)
                     driver.input->set_keybinds(&key_label, 0, 0, 0, (1ULL << KEYBINDS_ACTION_GET_BIND_LABEL));
                  if (driver.input->set_keybinds)
                     driver.input->set_keybinds(driver.input_data, g_settings.input.device[rgui->current_pad],
                           rgui->current_pad, current_index, (1ULL << KEYBINDS_ACTION_INCREMENT_BIND));

                  snprintf(button, sizeof(button), "%s #%d: %s",
                        g_settings.input.binds[rgui->current_pad][current_index].desc, rgui->current_pad, 
                        key_label.desc);
                  mbstowcs(strw_buffer, button, sizeof(strw_buffer) / sizeof(wchar_t));
                  XuiListSetText(m_menulist, current_index, strw_buffer);
               }
               break;
         }
         break;
      case XUI_CONTROL_NAVIGATE_UP:
      case XUI_CONTROL_NAVIGATE_DOWN:
         break;
   }

   set_dpad_emulation_label(rgui->current_pad, button, sizeof(button));

   mbstowcs(strw_buffer, button, sizeof(strw_buffer) / sizeof(wchar_t));
   XuiListSetText(m_menulist, SETTING_CONTROLS_DPAD_EMULATION, strw_buffer);
   XuiListSetText(m_menulist, SETTING_CONTROLS_DEFAULT_ALL, L"Reset all buttons to default");

   return 0;
}

HRESULT CRetroArchControls::OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled )
{
   if ( hObjPressed == m_menulist)
   {
      XUIMessageControlNavigate controls;
      controls.nControlNavigate = (XUI_CONTROL_NAVIGATE)XUI_CONTROL_NAVIGATE_OK;
      OnControlNavigate(&controls, bHandled);
   }

   bHandled = TRUE;
   return 0;
}

HRESULT CRetroArchSettings::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
   GetChildById(L"XuiMenuList", &m_menulist);
   GetChildById(L"XuiBackButton", &m_back);
   GetChildById(L"XuiTxtTitle", &m_menutitle);

   XuiTextElementSetText(m_menutitle, L"Settings");

   init_menulist(INGAME_MENU_SETTINGS_MODE);

   return 0;
}

HRESULT CRetroArchSettings::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
   int current_index;
   xdk_d3d_video_t *device_ptr = (xdk_d3d_video_t*)driver.video_data;

   current_index = XuiListGetCurSel(m_menulist, NULL);

   switch(pControlNavigateData->nControlNavigate)
   {
      case XUI_CONTROL_NAVIGATE_LEFT:
         switch(current_index)
         {
            case SETTING_EMU_REWIND_ENABLED:
               settings_set(1ULL << S_REWIND);
               XuiListSetText(m_menulist, SETTING_EMU_REWIND_ENABLED, g_settings.rewind_enable ? L"Rewind: ON" : L"Rewind: OFF");
               break;
	    case SETTING_EMU_REWIND_GRANULARITY:
	       if (g_settings.rewind_granularity > 1)
		       g_settings.rewind_granularity--;

	       menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_REWIND_GRANULARITY, sizeof(strw_buffer));
	       XuiListSetText(m_menulist, SETTING_EMU_REWIND_GRANULARITY, strw_buffer);
	       break;
            case SETTING_EMU_SHOW_INFO_MSG:
               if (g_extern.lifecycle_mode_state & (1ULL << MODE_INFO_DRAW))
                  g_extern.lifecycle_mode_state &= ~(1ULL << MODE_INFO_DRAW);
               else
                  g_extern.lifecycle_mode_state |= (1ULL << MODE_INFO_DRAW);
               XuiListSetText(m_menulist, SETTING_EMU_SHOW_INFO_MSG, (g_extern.lifecycle_mode_state & (1ULL << MODE_INFO_DRAW)) ? L"Info messages: ON" : L"Info messages: OFF");
               break;
            case SETTING_EMU_SHOW_DEBUG_INFO_MSG:
               if (g_extern.lifecycle_mode_state & (1ULL << MODE_FPS_DRAW))
                  g_extern.lifecycle_mode_state &= ~(1ULL << MODE_FPS_DRAW);
               else
                  g_extern.lifecycle_mode_state |= (1ULL << MODE_FPS_DRAW);
               XuiListSetText(m_menulist, SETTING_EMU_SHOW_DEBUG_INFO_MSG, (g_extern.lifecycle_mode_state & (1ULL << MODE_FPS_DRAW)) ? L"Debug Info messages: ON" : L"Debug Info messages: OFF");
               break;
            case SETTING_GAMMA_CORRECTION_ENABLED:
               if (g_extern.main_is_init)
               {
               g_extern.console.screen.gamma_correction = g_extern.console.screen.gamma_correction ? 0 : 1;
               driver.video->restart();
               XuiListSetText(m_menulist, SETTING_GAMMA_CORRECTION_ENABLED, g_extern.console.screen.gamma_correction ? L"Gamma correction: ON" : L"Gamma correction: OFF");
               }
               break;
            case SETTING_HW_TEXTURE_FILTER:
               g_settings.video.smooth = !g_settings.video.smooth;
               XuiListSetText(m_menulist, SETTING_HW_TEXTURE_FILTER, g_settings.video.smooth ? L"Default Filter: Linear" : L"Default Filter: Nearest");
               break;
            default:
               break;
         }
         break;
      case XUI_CONTROL_NAVIGATE_OK:
      case XUI_CONTROL_NAVIGATE_RIGHT:
         switch(current_index)
         {
            case SETTING_EMU_SHOW_INFO_MSG:
               if (g_extern.lifecycle_mode_state & (1ULL << MODE_INFO_DRAW))
                  g_extern.lifecycle_mode_state &= ~(1ULL << MODE_INFO_DRAW);
               else
                  g_extern.lifecycle_mode_state |= (1ULL << MODE_INFO_DRAW);
               XuiListSetText(m_menulist, SETTING_EMU_SHOW_INFO_MSG, (g_extern.lifecycle_mode_state & (1ULL << MODE_INFO_DRAW)) ? L"Info messages: ON" : L"Info messages: OFF");
               break;
            case SETTING_EMU_SHOW_DEBUG_INFO_MSG:
               if (g_extern.lifecycle_mode_state & (1ULL << MODE_FPS_DRAW))
                  g_extern.lifecycle_mode_state &= ~(1ULL << MODE_FPS_DRAW);
               else
                  g_extern.lifecycle_mode_state |= (1ULL << MODE_FPS_DRAW);
               XuiListSetText(m_menulist, SETTING_EMU_SHOW_DEBUG_INFO_MSG, (g_extern.lifecycle_mode_state & (1ULL << MODE_FPS_DRAW)) ? L"Debug Info messages: ON" : L"Debug Info messages: OFF");
               break;
            case SETTING_GAMMA_CORRECTION_ENABLED:
               if (g_extern.main_is_init)
               {
               g_extern.console.screen.gamma_correction = g_extern.console.screen.gamma_correction ? 0 : 1;
               driver.video->restart();
               XuiListSetText(m_menulist, SETTING_GAMMA_CORRECTION_ENABLED, g_extern.console.screen.gamma_correction ? L"Gamma correction: ON" : L"Gamma correction: OFF");
               }
               break;
            case SETTING_EMU_REWIND_ENABLED:
               settings_set(1ULL << S_REWIND);
               XuiListSetText(m_menulist, SETTING_EMU_REWIND_ENABLED, g_settings.rewind_enable ? L"Rewind: ON" : L"Rewind: OFF");
               break;
	    case SETTING_EMU_REWIND_GRANULARITY:
	       g_settings.rewind_granularity++;

	       menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_REWIND_GRANULARITY, sizeof(strw_buffer));
	       XuiListSetText(m_menulist, SETTING_EMU_REWIND_GRANULARITY, strw_buffer);
	       break;
            case SETTING_HW_TEXTURE_FILTER:
               g_settings.video.smooth = !g_settings.video.smooth;
               XuiListSetText(m_menulist, SETTING_HW_TEXTURE_FILTER, g_settings.video.smooth ? L"Default Filter: Linear" : L"Default Filter: Nearest");
               break;
            default:
               break;
         }
         break;
      case XUI_CONTROL_NAVIGATE_UP:
      case XUI_CONTROL_NAVIGATE_DOWN:
         break;
   }

   bHandled = TRUE;

   switch(pControlNavigateData->nControlNavigate)
   {
      case XUI_CONTROL_NAVIGATE_LEFT:
      case XUI_CONTROL_NAVIGATE_RIGHT:
      case XUI_CONTROL_NAVIGATE_UP:
      case XUI_CONTROL_NAVIGATE_DOWN:
         pControlNavigateData->hObjDest = pControlNavigateData->hObjSource;
         break;
      default:
         break;
   }

   return 0;
}

HRESULT CRetroArchSettings::OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled )
{
   process_input_ret = 0;

   if ( hObjPressed == m_menulist)
   {
      XUIMessageControlNavigate controls;
      controls.nControlNavigate = (XUI_CONTROL_NAVIGATE)XUI_CONTROL_NAVIGATE_OK;
      OnControlNavigate(&controls, bHandled);
   }

   bHandled = TRUE;

   return 0;
}

HRESULT CRetroArchQuickMenu::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
   GetChildById(L"XuiMenuList", &m_menulist);
   GetChildById(L"XuiBackButton", &m_back);
   GetChildById(L"XuiTxtTitle", &m_menutitle);

   XuiListDeleteItems(m_menulist, 0, XuiListGetItemCount(m_menulist));

   XuiTextElementSetText(m_menutitle, L"Menu");

   menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_LOAD_STATE_SLOT, sizeof(strw_buffer));
   XuiListInsertItems(m_menulist, 0, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_LOAD_STATE, strw_buffer);

   menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_SAVE_STATE_SLOT, sizeof(strw_buffer));
   XuiListInsertItems(m_menulist, 1, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_SAVE_STATE, strw_buffer);

   if (driver.video_poke->set_aspect_ratio)
      driver.video_poke->set_aspect_ratio(driver.video_data, g_settings.video.aspect_ratio_idx);

   menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_ASPECT_RATIO, sizeof(strw_buffer));
   XuiListInsertItems(m_menulist, 2, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_ASPECT_RATIO, strw_buffer);

   driver.video->set_rotation(driver.video_data, g_extern.console.screen.orientation);
   menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_ROTATION, sizeof(strw_buffer));
   XuiListInsertItems(m_menulist, 3, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_ORIENTATION, strw_buffer);

   XuiListInsertItems(m_menulist, 4, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_RESIZE_MODE, L"Custom Ratio ...");

   XuiListInsertItems(m_menulist, 5, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_FRAME_ADVANCE, L"Frame Advance ...");

   XuiListInsertItems(m_menulist, 6, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_RESET, L"Restart Game");

   XuiListInsertItems(m_menulist, 7, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_RETURN_TO_GAME, L"Resume Game");

   XuiListInsertItems(m_menulist, 8, 1);
   XuiListSetText(m_menulist, MENU_XUI_ITEM_QUIT_RARCH, L"Quit RetroArch");

   return 0;
}

HRESULT CRetroArchQuickMenu::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
   bool aspectratio_changed = false;
   int current_index;

   current_index = XuiListGetCurSel(m_menulist, NULL);

   switch(pControlNavigateData->nControlNavigate)
   {
      case XUI_CONTROL_NAVIGATE_LEFT:
         switch(current_index)
         {
            case MENU_XUI_ITEM_LOAD_STATE:
            case MENU_XUI_ITEM_SAVE_STATE:
               rarch_state_slot_decrease();
               menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_LOAD_STATE_SLOT, sizeof(strw_buffer));
               XuiListSetText(m_menulist, MENU_XUI_ITEM_LOAD_STATE, strw_buffer);
               menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_SAVE_STATE_SLOT, sizeof(strw_buffer));
               XuiListSetText(m_menulist, MENU_XUI_ITEM_SAVE_STATE, strw_buffer);
               break;
            case MENU_XUI_ITEM_ASPECT_RATIO:
               settings_set(1ULL << S_ASPECT_RATIO_DECREMENT);
               aspectratio_changed = true;
               break;
            case MENU_XUI_ITEM_ORIENTATION:
               settings_set(1ULL << S_ROTATION_DECREMENT);
               menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_ROTATION, sizeof(strw_buffer));
               XuiListSetText(m_menulist, MENU_XUI_ITEM_ORIENTATION, strw_buffer);
               driver.video->set_rotation(driver.video_data, g_extern.console.screen.orientation);
               break;
            default:
               break;
         }
         break;
      case XUI_CONTROL_NAVIGATE_RIGHT:
         switch(current_index)
         {
            case MENU_XUI_ITEM_LOAD_STATE:
            case MENU_XUI_ITEM_SAVE_STATE:
               rarch_state_slot_increase();
               menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_LOAD_STATE_SLOT, sizeof(strw_buffer));
               XuiListSetText(m_menulist, MENU_XUI_ITEM_LOAD_STATE, strw_buffer);
               menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_SAVE_STATE_SLOT, sizeof(strw_buffer));
               XuiListSetText(m_menulist, MENU_XUI_ITEM_SAVE_STATE, strw_buffer);
               break;
            case MENU_XUI_ITEM_ASPECT_RATIO:
               settings_set(1ULL << S_ASPECT_RATIO_INCREMENT);
               aspectratio_changed = true;
               break;
            case MENU_XUI_ITEM_ORIENTATION:
               settings_set(1ULL << S_ROTATION_INCREMENT);
               menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_ROTATION, sizeof(strw_buffer));
               XuiListSetText(m_menulist, MENU_XUI_ITEM_ORIENTATION, strw_buffer);
               driver.video->set_rotation(driver.video_data, g_extern.console.screen.orientation);
               break;
            default:
               break;
         }
         break;
      case XUI_CONTROL_NAVIGATE_UP:
      case XUI_CONTROL_NAVIGATE_DOWN:
         break;
   }

   if(aspectratio_changed)
   {
      if (driver.video_poke->set_aspect_ratio)
         driver.video_poke->set_aspect_ratio(driver.video_data, g_settings.video.aspect_ratio_idx);
      menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_ASPECT_RATIO, sizeof(strw_buffer));
      XuiListSetText(m_menulist, MENU_XUI_ITEM_ASPECT_RATIO, strw_buffer);
   }

   bHandled = TRUE;

   switch(pControlNavigateData->nControlNavigate)
   {
      case XUI_CONTROL_NAVIGATE_LEFT:
      case XUI_CONTROL_NAVIGATE_RIGHT:
      case XUI_CONTROL_NAVIGATE_UP:
      case XUI_CONTROL_NAVIGATE_DOWN:
         pControlNavigateData->hObjDest = pControlNavigateData->hObjSource;
         break;
      default:
         break;
   }

   return 0;
}

HRESULT CRetroArchQuickMenu::OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled )
{
   process_input_ret = 0;

   if ( hObjPressed == m_menulist)
   {
      XUIMessageControlNavigate controls;
      controls.nControlNavigate = (XUI_CONTROL_NAVIGATE)XUI_CONTROL_NAVIGATE_OK;
      OnControlNavigate(&controls, bHandled);
   }

   bHandled = TRUE;

   return 0;
}

HRESULT CRetroArchShaderBrowser::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
   GetChildById(L"XuiRomList", &m_list);
   GetChildById(L"XuiBackButton1", &m_back);
   GetChildById(L"XuiTxtRomPath", &m_list_path);

   filebrowser_set_root_and_ext(rgui->browser, "cg", "game:\\media\\shaders");
   uint64_t action = (1ULL << DEVICE_NAV_B);
   filebrowser_fetch_directory_entries(action);

   return 0;
}

HRESULT CRetroArchShaderBrowser::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
   char path[PATH_MAX];
   process_input_ret = 0;

   if(hObjPressed == m_list)
   {
      int index = XuiListGetCurSel(m_list, NULL);
      if (path_file_exists(rgui->browser->list->elems[index].data))
         wcstombs(str_buffer, (const wchar_t *)XuiListGetText(m_list, index), sizeof(str_buffer));
      else if (rgui->browser->list->elems[index].attr.b)
      {
         wcstombs(str_buffer, (const wchar_t *)XuiListGetText(m_list, index), sizeof(str_buffer));
         snprintf(path, sizeof(path), "%s\\%s", rgui->browser->current_dir.directory_path, str_buffer);
         filebrowser_set_root_and_ext(rgui->browser, "cg", path);
         uint64_t action = (1ULL << DEVICE_NAV_B);
         filebrowser_fetch_directory_entries(action);
      }
   }

   bHandled = TRUE;

   return 0;
}

HRESULT CRetroArchCoreBrowser::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
   GetChildById(L"XuiRomList", &m_list);
   GetChildById(L"XuiBackButton1", &m_back);
   GetChildById(L"XuiTxtRomPath", &m_list_path);

   filebrowser_set_root_and_ext(rgui->browser, "xex|XEX", "game:");
   uint64_t action = (1ULL << DEVICE_NAV_B);
   filebrowser_fetch_directory_entries(action);

   return 0;
}

HRESULT CRetroArchCoreBrowser::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
   char path[PATH_MAX];

   process_input_ret = 0;

   if(hObjPressed == m_list)
   {
      int index = XuiListGetCurSel(m_list, NULL);
      wcstombs(str_buffer, (const wchar_t *)XuiListGetText(m_list, index), sizeof(str_buffer));
      if(path_file_exists(rgui->browser->list->elems[index].data))
      {
         snprintf(g_settings.libretro, sizeof(g_settings.libretro), "%s\\%s",
               rgui->browser->current_dir.directory_path, str_buffer);
         g_extern.lifecycle_mode_state |= (1ULL << MODE_EXIT);
         g_extern.lifecycle_mode_state |= (1ULL << MODE_EXITSPAWN);
         process_input_ret = -1;
      }
      else if (rgui->browser->list->elems[index].attr.b)
      {
         snprintf(path, sizeof(path), "%s\\%s", rgui->browser->current_dir.directory_path, str_buffer);
         filebrowser_set_root_and_ext(rgui->browser, "xex|XEX", path);
         uint64_t action = (1ULL << DEVICE_NAV_B);
         filebrowser_fetch_directory_entries(action);
      }
   }

   bHandled = TRUE;
   return 0;
}

HRESULT CRetroArchMain::OnInit(XUIMessageInit * pInitData, BOOL& bHandled)
{
   GetChildById(L"XuiMenuList", &m_menulist);
   GetChildById(L"XuiTxtTitle", &m_menutitle);
   GetChildById(L"XuiTxtBottom", &m_menutitlebottom);

   init_menulist(INGAME_MENU_MAIN_MODE);

   mbstowcs(strw_buffer, g_extern.title_buf, sizeof(strw_buffer) / sizeof(wchar_t));
   XuiTextElementSetText(m_menutitle, strw_buffer);
   menu_settings_create_menu_item_label_w(strw_buffer, S_LBL_RARCH_VERSION, sizeof(strw_buffer));
   XuiTextElementSetText(m_menutitlebottom, strw_buffer);

   return 0;
}

HRESULT CRetroArchMain::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
   xdk_d3d_video_t *vid = (xdk_d3d_video_t*)driver.video_data;
   HRESULT hr;
   int current_index;
   bool hdmenus_allowed = (g_extern.lifecycle_mode_state & (1ULL << MODE_MENU_HD));

   switch(pControlNavigateData->nControlNavigate)
   {
      case XUI_CONTROL_NAVIGATE_LEFT:
      case XUI_CONTROL_NAVIGATE_RIGHT:
      case XUI_CONTROL_NAVIGATE_UP:
      case XUI_CONTROL_NAVIGATE_DOWN:
         break;
      case XUI_CONTROL_NAVIGATE_OK:
      current_index= XuiListGetCurSel(m_menulist, NULL);

      switch (current_index)
      {
         case INGAME_MENU_CHANGE_LIBRETRO_CORE:
           hr = XuiSceneCreate(hdmenus_allowed ? L"file://game:/media/hd/" : L"file://game:/media/sd/", L"rarch_libretrocore_browser.xur", NULL, &app.hCoreBrowser);

           if (hr < 0)
              RARCH_ERR("Failed to load scene.\n");

            XuiSceneNavigateForward(hCur, false, app.hCoreBrowser, XUSER_INDEX_FOCUS);
            hCur = app.hCoreBrowser;
            break;
         case INGAME_MENU_LOAD_GAME_HISTORY_MODE:
            break;
         case INGAME_MENU_CHANGE_GAME:
            hr = XuiSceneCreate(hdmenus_allowed ? L"file://game:/media/hd/" : L"file://game:/media/sd/", L"rarch_filebrowser.xur", NULL, &app.hFileBrowser);

            if (hr < 0)
               RARCH_ERR("Failed to load scene.\n");

            XuiSceneNavigateForward(hCur, false, app.hFileBrowser, XUSER_INDEX_FOCUS);
            hCur = app.hFileBrowser;
            break;
         case INGAME_MENU_CORE_OPTIONS_MODE:
            break;
         case INGAME_MENU_VIDEO_OPTIONS_MODE:
            hr = XuiSceneCreate(hdmenus_allowed ? L"file://game:/media/hd/" : L"file://game:/media/sd/", L"rarch_quickmenu.xur", NULL, &app.hQuickMenu);

            if (hr < 0)
               RARCH_ERR("Failed to load scene.\n");

            XuiSceneNavigateForward(hCur, false, app.hQuickMenu, XUSER_INDEX_FOCUS);
            hCur = app.hQuickMenu;
            break;
          case INGAME_MENU_AUDIO_OPTIONS_MODE:
            break;
          case INGAME_MENU_INPUT_OPTIONS_MODE:
            hr = XuiSceneCreate(hdmenus_allowed ? L"file://game:/media/hd/" : L"file://game:/media/sd/", L"rarch_controls.xur", NULL, &app.hControlsMenu);

            if (hr < 0)
               RARCH_ERR("Failed to load scene.\n");

            XuiSceneNavigateForward(hCur, false, app.hControlsMenu, XUSER_INDEX_FOCUS);
            hCur = app.hControlsMenu;
            break;
          case INGAME_MENU_PATH_OPTIONS_MODE:
            break;
          case INGAME_MENU_SETTINGS_MODE:
            hr = XuiSceneCreate(hdmenus_allowed ? L"file://game:/media/hd/" : L"file://game:/media/sd/", L"rarch_settings.xur", NULL, &app.hRetroArchSettings);

            if (hr < 0)
               RARCH_ERR("Failed to load scene.\n");

            XuiSceneNavigateForward(hCur, false, app.hRetroArchSettings, XUSER_INDEX_FOCUS);
            hCur = app.hRetroArchSettings;
            break;
          case INGAME_MENU_LOAD_STATE:
            break;
          case INGAME_MENU_SAVE_STATE:
            break;
          case INGAME_MENU_SCREENSHOT_MODE:
            break;
          case INGAME_MENU_RETURN_TO_GAME:
            break;
          case INGAME_MENU_RESET:
            break;
          case INGAME_MENU_REWIND_ENABLED:
            break;
          case INGAME_MENU_REWIND_GRANULARITY:
            break;
          case INGAME_MENU_FRAME_ADVANCE:
            break;
          case INGAME_MENU_QUIT_RETROARCH:
            g_extern.lifecycle_mode_state &= ~(1ULL << MODE_GAME);
            g_extern.lifecycle_mode_state |= (1ULL << MODE_EXIT);
            process_input_ret = -1;
            break;
      }
         break;
   }

   bHandled = TRUE;

   switch(pControlNavigateData->nControlNavigate)
   {
      case XUI_CONTROL_NAVIGATE_LEFT:
      case XUI_CONTROL_NAVIGATE_RIGHT:
      case XUI_CONTROL_NAVIGATE_UP:
      case XUI_CONTROL_NAVIGATE_DOWN:
         pControlNavigateData->hObjDest = pControlNavigateData->hObjSource;
         break;
      default:
         break;
   }

   return 0;
}

HRESULT CRetroArchMain::OnNotifyPress( HXUIOBJ hObjPressed,  int & bHandled )
{
   if ( hObjPressed == m_menulist)
   {
      XUIMessageControlNavigate controls;
      controls.nControlNavigate = (XUI_CONTROL_NAVIGATE)XUI_CONTROL_NAVIGATE_OK;
      OnControlNavigate(&controls, bHandled);
   }

   bHandled = TRUE;
   return 0;
}

rgui_handle_t *rgui_init (void)
{
   HRESULT hr;

   rgui_handle_t *rgui = (rgui_handle_t*)calloc(1, sizeof(*rgui));

   xdk_d3d_video_t *device_ptr = (xdk_d3d_video_t*)driver.video_data;

   bool hdmenus_allowed = (g_extern.lifecycle_mode_state & (1ULL << MODE_MENU_HD));

   if (hdmenus_allowed)
      RARCH_LOG("HD menus enabled.\n");

   D3DPRESENT_PARAMETERS d3dpp;
   video_info_t video_info = {0};

   video_info.vsync = g_settings.video.vsync;
   video_info.force_aspect = false;
   video_info.smooth = g_settings.video.smooth;
   video_info.input_scale = 2;
   video_info.fullscreen = true;
   video_info.rgb32 = false;

   xdk_d3d_generate_pp(&d3dpp, &video_info);

   hr = app.InitShared(device_ptr->d3d_render_device, &d3dpp, XuiPNGTextureLoader);

   if (hr != S_OK)
   {
      RARCH_ERR("Failed initializing XUI application.\n");
      return NULL;
   }

   /* Register font */
   TypefaceDescriptor typeface = {0};
   typeface.szTypeface = L"Arial Unicode MS";
   typeface.szLocator = L"file://game:/media/rarch.ttf";
   typeface.szReserved1 = NULL;
   
   hr = XuiRegisterTypeface( &typeface, TRUE );
   if (hr != S_OK)
   {
      RARCH_ERR("Failed to register default typeface.\n");
      return NULL;
   }

   hr = XuiLoadVisualFromBinary( L"file://game:/media/rarch_scene_skin.xur", NULL);
   if (hr != S_OK)
   {
      RARCH_ERR("Failed to load skin.\n");
      return NULL;
   }

   hr = XuiSceneCreate(hdmenus_allowed ? L"file://game:/media/hd/" : L"file://game:/media/sd/", L"rarch_main.xur", NULL, &app.hMainScene);
   if (hr != S_OK)
   {
      RARCH_ERR("Failed to create scene 'rarch_main.xur'.\n");
      return NULL;
   }

   hCur = app.hMainScene;
   hr = XuiSceneNavigateFirst(app.GetRootObj(), app.hMainScene, XUSER_INDEX_FOCUS);
   if (hr != S_OK)
   {
      RARCH_ERR("XuiSceneNavigateFirst failed.\n");
      return NULL;
   }

   if (driver.video_poke && driver.video_poke->set_texture_enable)
      driver.video_poke->set_texture_frame(driver.video_data, NULL,
            true, 0, 0, 1.0f);

   return rgui;
}

void rgui_free (rgui_handle_t *rgui)
{
   app.Uninit();
}

static void ingame_menu_resize (void)
{
   XINPUT_STATE state;

   XInputGetState(0, &state);

   if(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT || state.Gamepad.sThumbLX < -DEADZONE)
      g_extern.console.screen.viewports.custom_vp.x -= 1;
   else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT || state.Gamepad.sThumbLX > DEADZONE)
      g_extern.console.screen.viewports.custom_vp.x += 1;

   if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP || state.Gamepad.sThumbLY > DEADZONE)
      g_extern.console.screen.viewports.custom_vp.y += 1;
   else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN || state.Gamepad.sThumbLY < -DEADZONE) 
      g_extern.console.screen.viewports.custom_vp.y -= 1;

   if (state.Gamepad.sThumbRX < -DEADZONE || state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
      g_extern.console.screen.viewports.custom_vp.width -= 1;
   else if (state.Gamepad.sThumbRX > DEADZONE || state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
      g_extern.console.screen.viewports.custom_vp.width += 1;

   if (state.Gamepad.sThumbRY > DEADZONE || state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
      g_extern.console.screen.viewports.custom_vp.height += 1;
   else if (state.Gamepad.sThumbRY < -DEADZONE || state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
      g_extern.console.screen.viewports.custom_vp.height -= 1;

   if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
   {
      g_extern.console.screen.viewports.custom_vp.x = 0;
      g_extern.console.screen.viewports.custom_vp.y = 0;
      g_extern.console.screen.viewports.custom_vp.width = 1280; //FIXME: hardcoded
      g_extern.console.screen.viewports.custom_vp.height = 720; //FIXME: hardcoded
   }
}

bool menu_iterate(void)
{
   xdk_d3d_video_t *device_ptr = (xdk_d3d_video_t*)driver.video_data;

   if (g_extern.lifecycle_mode_state & (1ULL << MODE_MENU_PREINIT))
   {
      g_extern.lifecycle_mode_state &= ~(1ULL << MODE_MENU_PREINIT);
      /* FIXME - hack for now */
      rgui->delay_count = 0;
   }

   rgui->trigger_state = 0;

   XINPUT_STATE state;
   XInputGetState(0, &state);

   if((state.Gamepad.wButtons & XINPUT_GAMEPAD_B) && hCur != app.hMainScene)
      rgui->trigger_state = RGUI_ACTION_CANCEL;
   else if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A))
      rgui->trigger_state = RGUI_ACTION_OK;

   /* FIXME - hack for now */
   if (rgui->delay_count > 30)
   {
   bool rmenu_enable = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) &&
      (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
      ) && g_extern.main_is_init;

   if (rmenu_enable)
   {
      g_extern.lifecycle_mode_state |= (1ULL << MODE_GAME);
      process_input_ret = -1;
   }
   }

   if (rgui->trigger_state == RGUI_ACTION_CANCEL)
   {
      XuiSceneNavigateBack(hCur, app.hMainScene, XUSER_INDEX_ANY);
      hCur = app.hMainScene;
      XuiElementGetChildById(app.hMainScene, L"XuiMenuList", &m_menulist);
      init_menulist(INGAME_MENU_MAIN_MODE);
   }

#if 0
   if (input_loop == INPUT_LOOP_RESIZE_MODE)
         ingame_menu_resize();
#endif

   if (driver.video_poke && driver.video_poke->set_texture_enable)
      driver.video_poke->set_texture_enable(driver.video_data, true, true);

   rarch_render_cached_frame();

   if (driver.video_poke && driver.video_poke->set_texture_enable)
      driver.video_poke->set_texture_enable(driver.video_data, false, true);

   /* FIXME - hack for now */
   rgui->delay_count++;

   if(process_input_ret != 0)
      goto deinit;

   return true;

deinit:
   process_input_ret = 0;

   return false;
}

bool menu_iterate_xui(void)
{
   xdk_d3d_video_t *d3d = (xdk_d3d_video_t*)driver.video_data;
   LPDIRECT3DDEVICE d3dr = (LPDIRECT3DDEVICE)d3d->d3d_render_device;

   app.RunFrame(); /* Update XUI */

	XuiRenderBegin( app.GetDC(), D3DCOLOR_ARGB( 255, 0, 0, 0 ) );

    D3DVIEWPORT vp = {0};
    vp.Width  = d3d->win_width;
    vp.Height = d3d->win_height;
    vp.X      = 0;
    vp.Y      = 0;
    vp.MinZ   = 0.0f;
    vp.MaxZ   = 1.0f;
    RD3DDevice_SetViewport(d3dr, &vp); 

    D3DXMATRIX matOrigView;
    XuiRenderGetViewTransform( app.GetDC(), &matOrigView );

	XUIMessage msg;
    XUIMessageRender msgRender;
    XuiMessageRender( &msg, &msgRender, app.GetDC(), 0xffffffff, XUI_BLEND_NORMAL );
    XuiSendMessage( app.GetRootObj(), &msg );

    XuiRenderSetViewTransform( app.GetDC(), &matOrigView );

    XuiRenderEnd( app.GetDC() );
   XuiTimersRun();
   return true;
}
