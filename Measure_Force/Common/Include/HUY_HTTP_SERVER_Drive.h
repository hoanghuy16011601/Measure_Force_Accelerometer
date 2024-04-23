#include "esp_http_server.h"

void Server_Run();
void Server_Initalization(char * URI,void (*Address_Handler),httpd_method_t Method);