#include "HUY_HTTP_SERVER_Drive.h"


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


