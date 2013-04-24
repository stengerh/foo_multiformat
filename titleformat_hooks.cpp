#include "stdafx.h"
#include "titleformat_hooks.h"

titleformat_hook_impl_random::titleformat_hook_impl_random(const char * p_name, t_size p_name_length) : m_name(p_name, p_name_length) {
}

bool titleformat_hook_impl_random::process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag) {
	p_found_flag = false;
	return false;
}

bool titleformat_hook_impl_random::process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) {
	if (0 == pfc::strcmp_ex(m_name, m_name.get_length(), p_name, p_name_length)) {
		if (p_params->get_param_count() == 1) {
			t_size range = p_params->get_param_uint(0);
			
			if (range > 0) {
				if (m_rng.is_empty()) {
					m_rng = genrand_service::g_create();
					m_rng->seed(::GetTickCount());
				}

				t_size value = m_rng->genrand(range);

				p_out->write_int(titleformat_inputtypes::unknown, value);
			}

			p_found_flag = false;
			return true;
		} else {
			p_found_flag = false;
			return false;
		}
	} else {
		p_found_flag = false;
		return false;
	}
}


titleformat_hook_impl_counter::titleformat_hook_impl_counter(const char * p_name, t_size p_name_length) : m_name(p_name, p_name_length), m_counter(0) {
}

bool titleformat_hook_impl_counter::process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag) {
	if (0 == pfc::strcmp_ex(m_name, m_name.get_length(), p_name, p_name_length)) {
		t_size value = ++m_counter;
		p_out->write_int(titleformat_inputtypes::unknown, value);

		p_found_flag = false;
		return true;
	} else {
		p_found_flag = false;
		return false;
	}
}

bool titleformat_hook_impl_counter::process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) {
	p_found_flag = false;
	return false;
}
