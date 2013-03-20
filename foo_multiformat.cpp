// foo_multiformat.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "branch_formatter.h"

DECLARE_COMPONENT_VERSION(
"Multi-formatting Test",
"1.0 alpha 2008-11-10",
"Copyright (C) 2008 Holger Stenger"
)

class multiformat_contextmenu_item : public contextmenu_item_simple
{
private:
	enum
	{
		cmd_multiformat_test = 0,
		cmd_total
	};

public:
	virtual unsigned get_num_items();
	virtual void get_item_name(unsigned p_index, pfc::string_base & p_out);
	virtual GUID get_parent();
	virtual bool get_item_description(unsigned p_index, pfc::string_base & p_out);
	virtual GUID get_item_guid(unsigned p_index);
	virtual void context_command(unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller);
};

static service_factory_single_t<multiformat_contextmenu_item> g_multiformat_contextmenu_item_factory;

unsigned multiformat_contextmenu_item::get_num_items()
{
	return cmd_total;
}

void multiformat_contextmenu_item::get_item_name(unsigned int p_index, pfc::string_base &p_out)
{
	switch (p_index)
	{
	case cmd_multiformat_test:
		p_out = "Multi-format test";
		break;

	default:
		uBugCheck();
	}
}

GUID multiformat_contextmenu_item::get_parent()
{
	return contextmenu_groups::utilities;
}

bool multiformat_contextmenu_item::get_item_description(unsigned int p_index, pfc::string_base &p_out)
{
	switch (p_index)
	{
	case cmd_multiformat_test:
		p_out = "Runs multi-format test.";
		return true;

	default:
		uBugCheck();
	}
}

GUID multiformat_contextmenu_item::get_item_guid(unsigned int p_index)
{
	// {CA6C5005-C513-4f23-A91A-7E271E394FF7}
	static const GUID guid =
	{ 0xca6c5005, 0xc513, 0x4f23, { 0xa9, 0x1a, 0x7e, 0x27, 0x1e, 0x39, 0x4f, 0xf7 } };

	switch (p_index)
	{
	case cmd_multiformat_test:
		return guid;

	default:
		uBugCheck();
	}
}

void multiformat_contextmenu_item::context_command(unsigned int p_index, metadb_handle_list_cref p_data, const GUID &p_caller)
{
	switch (p_index)
	{
	case cmd_multiformat_test:
		{
			pfc::list_t<pfc::string8> list;

			{
				//titleformat_object_wrapper script("$caps($left($put(x,%<genre>%),1))|$get(x)|%<artist>%");
				//titleformat_object_wrapper script("$swapprefix(%<artist>%)");
				//titleformat_object_wrapper script("$if($strcmp(%<genre>%,Rock),%<genre>%)");
				titleformat_object_wrapper script("%<test>%");
				multiformat::branch_formatter formatter(script);
				in_metadb_sync lock;
				for (t_size n = 0; n < p_data.get_count(); ++n)
				{
					formatter.run_nonlocking(list, p_data[n].get_ptr(), NULL);
				}
			}

			{
				console::formatter msg;
				for (t_size n = 0; n < list.get_count(); ++n)
				{
					msg << "\"" << list[n] << "\"\n";
				}
			}

			break;
		}

	default:
		uBugCheck();
	}
}