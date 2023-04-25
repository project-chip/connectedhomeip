#ifndef SL_EVENT_HANDLER_H
#define SL_EVENT_HANDLER_H

void sl_platform_init(void);
void sl_driver_init(void);
void sl_service_init(void);
void sl_stack_init(void);
void sl_internal_app_init(void);
void sl_platform_process_action(void);
void sl_service_process_action(void);
void sl_stack_process_action(void);
void sl_internal_app_process_action(void);
void sl_iostream_init_instances(void);

#endif // SL_EVENT_HANDLER_H
