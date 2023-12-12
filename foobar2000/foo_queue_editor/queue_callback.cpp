#include "stdafx.h"
#include "window_manager.h"

class queuecontents_queue_callback : public playback_queue_callback {
public:
	virtual void on_changed(t_change_origin p_origin)
	{
		static_api_ptr_t<main_thread_callback_manager>()->add_callback(new service_impl_t<global_refresh_callback>());
	}
};

static service_factory_t< queuecontents_queue_callback > queuecontents_qcallback;
