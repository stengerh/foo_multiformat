class multiformat_dialog {
public:
	static void g_show_dialog(metadb_handle_list_cref p_tracks);

private:
	multiformat_dialog(metadb_handle_list_cref p_tracks);
	~multiformat_dialog();

	void show();

	static BOOL CALLBACK g_dialog_proc(HWND p_wnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);
	BOOL dialog_proc(HWND p_wnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);

	HWND m_wnd;

	metadb_handle_list m_tracks;
	pfc::list_t<pfc::string8> m_results;
};
