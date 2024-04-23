#include "HUY_HTTP_SERVER_Drive.h"

#define EXAMPLE_STATIC_IP_ADDR        "192.168.4.2"
#define EXAMPLE_STATIC_NETMASK_ADDR   "255.255.255.0"
#define EXAMPLE_STATIC_GW_ADDR        "192.168.4.1"

httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
httpd_handle_t server_handle = NULL;



void Server_Run()
{
    httpd_start(&server_handle, &server_config);
}
void Server_Initalization(char * URI,void (*Address_Handler),httpd_method_t Method)
{
    httpd_uri_t uri_config = {
        .uri = URI,
        .method = Method,
        .handler = Address_Handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server_handle, &uri_config);
}

