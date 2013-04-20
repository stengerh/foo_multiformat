#include "stdafx.h"
#include "multiformat_dialog.h"
#include "branch_formatter.h"
#include "resource.h"

// {2CB95F72-FCAA-4D5A-B017-8AB389B9E30C}
static const GUID guid_multiformat_pattern = 
{ 0x2cb95f72, 0xfcaa, 0x4d5a, { 0xb0, 0x17, 0x8a, 0xb3, 0x89, 0xb9, 0xe3, 0xc } };

static cfg_string g_multiformat_pattern(guid_multiformat_pattern, "$upper($left(%<artist>%,1))|%<artist>%|%title%");

void multiformat_dialog::g_show_dialog(metadb_handle_list_cref p_tracks) {
	if (p_tracks.get_count() > 0) {
		multiformat_dialog *dlg = new multiformat_dialog(p_tracks);
		dlg->show();
	}
}

multiformat_dialog::multiformat_dialog(metadb_handle_list_cref p_tracks)
	: m_wnd(NULL)
	, m_tracks(p_tracks) {
}

multiformat_dialog::~multiformat_dialog() {
}

void multiformat_dialog::show() {
	m_wnd = ::CreateDialogParam(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_MULTIFORMAT),
		core_api::get_main_window(),
		&multiformat_dialog::g_dialog_proc, reinterpret_cast<LPARAM>(this));
	if (m_wnd != NULL) {
		::ShowWindow(m_wnd, SW_SHOW);
	}
}

BOOL multiformat_dialog::g_dialog_proc(HWND p_wnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam) {
	multiformat_dialog * dlg = NULL;
	if (p_msg == WM_INITDIALOG) {
		::SetWindowLongPtr(p_wnd, DWL_USER, p_lparam);
		dlg = reinterpret_cast<multiformat_dialog *>(p_lparam);
	} else {
		dlg = reinterpret_cast<multiformat_dialog *>(::GetWindowLong(p_wnd, DWL_USER));
	}

	BOOL rv = FALSE;

	if (dlg != NULL) {
		rv = dlg->dialog_proc(p_wnd, p_msg, p_wparam, p_lparam);
	}

	if (p_msg == WM_DESTROY) {
		if (dlg != NULL) {
			delete dlg;
		}
	}

	return rv;
}

BOOL multiformat_dialog::dialog_proc(HWND p_wnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam) {
	switch (p_msg) {
	case WM_INITDIALOG:
		uSetDlgItemText(p_wnd, IDC_SOURCE, g_multiformat_pattern.get_ptr());
		return TRUE;

	case WM_COMMAND:
		switch (p_wparam) {
		case IDOK:
			{
				m_results.remove_all();
				::SendDlgItemMessage(p_wnd, IDC_RESULTS, LB_RESETCONTENT, 0, 0);
				uSetDlgItemText(p_wnd, IDC_OUTPUT, "");

				pfc::string pattern = ::uGetDlgItemText(p_wnd, IDC_SOURCE);
				g_multiformat_pattern.set_string(pattern.get_ptr());

				titleformat_object_wrapper script(pattern.get_ptr());
				multiformat::branch_formatter formatter(script);

				{
					in_metadb_sync lock;
					for (t_size n = 0; n < m_tracks.get_count(); ++n) {
						formatter.run_nonlocking(m_results, m_tracks[n].get_ptr(), NULL);
					}
				}

				for (t_size n = 0; n < m_results.get_count(); ++n) {
					pfc::string_formatter text;
					text << "(" << (n+1) << "/" << m_results.get_count() << ") " << m_results[n];
					::uSendDlgItemMessageText(p_wnd, IDC_RESULTS, LB_ADDSTRING, 0, text.get_ptr());
				}
			}
			break;

		case IDCANCEL:
			::DestroyWindow(p_wnd);
			return TRUE;

		case MAKEWPARAM(IDC_RESULTS, LBN_SELCHANGE):
			{
				LRESULT selectedIndex = ::SendDlgItemMessage(p_wnd, IDC_RESULTS, LB_GETCURSEL, 0, 0);
				if (selectedIndex != LB_ERR) {
					uSetDlgItemText(p_wnd, IDC_OUTPUT, m_results[selectedIndex].get_ptr());
				}
			}
			return TRUE;
		}
	}

	return FALSE;
}
