class titleformat_hook_impl_random : public titleformat_hook {
public:
	titleformat_hook_impl_random(const char * p_name, t_size p_name_length = pfc::infinite_size);

	virtual bool process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag);
	virtual bool process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag);

private:
	pfc::string8 m_name;
	service_ptr_t<genrand_service> m_rng;
};

class titleformat_hook_impl_counter : public titleformat_hook {
public:
	titleformat_hook_impl_counter(const char * p_name, t_size p_name_length = pfc::infinite_size);

	virtual bool process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag);
	virtual bool process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag);

private:
	pfc::string8 m_name;
	t_size m_counter;
};
