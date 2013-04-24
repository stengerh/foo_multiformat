#include "stdafx.h"
#include "multiformat_dialog.h"

// {31C8024D-A008-4195-85FB-5B822B0B40F4}
static const GUID guid_contextmenu_multiformat = 
{ 0x31c8024d, 0xa008, 0x4195, { 0x85, 0xfb, 0x5b, 0x82, 0x2b, 0xb, 0x40, 0xf4 } };

DECLARE_CONTEXT_MENU_ITEM(contextmenu_multiformat, "Multi-formatting Tech Demo", "",
	multiformat_dialog::g_show_dialog, guid_contextmenu_multiformat,
	"Opens the window for the Multi-formatting Tech Demo.");
