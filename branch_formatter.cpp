#include "stdafx.h"
#include "branch_formatter.h"

namespace multiformat
{
	branch_formatter::branch_formatter(titleformat_object::ptr p_script)
		: m_script(p_script)
	{
	}

	void branch_formatter::run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_text_filter * p_filter)
	{
		m_stack.add_item(branch_info(1));

		const file_info * info = 0;
		if (!p_handle->get_info_locked(info)) return;
		titleformat_hook_impl_file_info_branch hook(*this, p_handle->get_location(), info);

		do
		{
			m_current = 0;
			p_handle->format_title_nonlocking(&hook, m_buffer, m_script, p_filter);

			if (!is_branch_empty())
			{
				p_list_out.add_item(m_buffer);
			}
		}	while (next_branch());
	}

	bool branch_formatter::is_branch_empty() const
	{
		for (t_size n = 0; n < m_stack.get_count(); ++n)
		{
			if (0 == m_stack[n].total)
			{
				return true;
			}
		}
		return false;
	}

	bool branch_formatter::next_branch()
	{
		for (t_size n = m_stack.get_count(); n > 0; --n)
		{
			if (++m_stack[n-1].current == m_stack[n-1].total)
			{
				m_stack.remove_by_idx(n-1);
			}
			else
			{
				return true;
			}
		}
		return false;
	}

	titleformat_hook_impl_file_info_branch::titleformat_hook_impl_file_info_branch(branch_formatter & p_formatter, const playable_location & p_location,const file_info * p_info)
		: m_formatter(p_formatter), titleformat_hook_impl_file_info(p_location, p_info)
	{
	}

	bool titleformat_hook_impl_file_info_branch::process_function(titleformat_text_out *p_out, const char *p_name, t_size p_name_length, titleformat_hook_function_params *p_params, bool &p_found_flag)
	{
		return false;
	}

	bool titleformat_hook_impl_file_info_branch::process_field(titleformat_text_out *p_out, const char *p_name, t_size p_name_length, bool &p_found_flag)
	{
		if ((p_name_length >= 2) && (p_name[0] == '<') && (p_name[p_name_length-1] == '>'))
		{
			t_size current = ++m_formatter.m_current;
			t_size meta_index;
			t_size value_index;
			if (current == m_formatter.m_stack.get_count())
			{
				// Check for previous occurrence of this field.
				pfc::string8 name(p_name, p_name_length);
				if (m_formatter.m_map.exists(name))
				{
					// Use existing entry.
					t_size stack_index = m_formatter.m_map[name];
					meta_index = m_formatter.m_stack[stack_index].meta_index;
					value_index = m_formatter.m_stack[stack_index].current;
				}
				else
				{
					// Add new entry.
					if (remap_meta(meta_index, p_name+1, p_name_length-2))
					{
						t_size value_count = m_info->meta_enum_value_count(meta_index);
						m_formatter.m_stack.add_item(branch_info(pfc::max_t<t_size>(1, value_count), meta_index));
						value_index = 0;
						m_formatter.m_map[name] = current;
					}
					else
					{
						m_formatter.m_stack.add_item(branch_info(1));
						p_found_flag = false;
						return true;
					}
				}
			}
			else
			{
				meta_index = m_formatter.m_stack[current].meta_index;
				value_index = m_formatter.m_stack[current].current;
			}

			p_out->write(titleformat_inputtypes::meta, m_info->meta_enum_value(meta_index, value_index));

			p_found_flag = true;
			return true;
		}
		return false;
	}
} // namespace multiformat