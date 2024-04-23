
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wpa2.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <netdb.h>

#include "lwip/sys.h"
#include "lwip/err.h"

#include "esp_smartconfig.h"

#include "HUY_WIFI_Drive.h"


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define CONNECTED_BIT  BIT2
#define ESPTOUCH_DONE_BIT BIT3

#define EXAMPLE_ESP_WIFI_SSID      "MEASURE FORCE"
#define EXAMPLE_ESP_WIFI_PASS      "0123456789"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

typedef void (*Smart_Congif_Function_Callback)(HUY_SMART_CONFIG_EVENT_enum);
Smart_Congif_Function_Callback Smart_Config_Handler_Callback;
typedef void (*WIFI_STATION_Function_Callback)(HUY_WIFI_STATION_EVENT_enum);
WIFI_STATION_Function_Callback WiFi_Station_Handler_Callback;

static const char *TAG = "Wifi";

static EventGroupHandle_t s_wifi_event_group;
esp_netif_t *netif;

static int s_retry_num = 0;
static int ESP_MAXIMUM_RETRY =3;


static void Set_DNS_Server(esp_netif_t *netif_config,uint32_t addr, esp_netif_dns_type_t type)
{
    if (addr && (addr != IPADDR_NONE)) {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        esp_netif_set_dns_info(netif_config, type, &dns);
    }
}
void STATIC_IP(void)
{
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr("192.168.4.111");
    ip.netmask.addr = ipaddr_addr("255.255.255.0");
    ip.gw.addr = ipaddr_addr("192.168.4.1");
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip");
    Set_DNS_Server(netif,ipaddr_addr("8.8.8.8"), ESP_NETIF_DNS_MAIN);
    Set_DNS_Server(netif,ipaddr_addr("8.8.4.4"), ESP_NETIF_DNS_BACKUP);
}


static void event_wifistation_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    HUY_WIFI_STATION_EVENT_enum Check_Event_Station = WiFi_Station_Default;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            Check_Event_Station = WiFi_Station_FailConnected;
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        Check_Event_Station = WiFi_Station_Connected;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        Check_Event_Station = WiFi_Station_GotIP;
    }
    WiFi_Station_Handler_Callback(Check_Event_Station);
}

void WIFI_STATION_INIT(void(*WiFi_Station_Event_Handler_Address))
{
    ESP_ERROR_CHECK(esp_netif_init());
    netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    WiFi_Station_Handler_Callback = WiFi_Station_Event_Handler_Address;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_wifistation_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_wifistation_handler,
                                                        NULL,
                                                        &instance_got_ip));
}

void WIFI_STATION_STOP(void)
{
    if(NULL != netif)
    {
        esp_event_handler_instance_unregister(WIFI_EVENT,ESP_EVENT_ANY_ID,&event_wifistation_handler);
        esp_event_handler_instance_unregister(IP_EVENT,IP_EVENT_STA_GOT_IP,&event_wifistation_handler);
        esp_wifi_disconnect();
        esp_wifi_stop();
        esp_netif_destroy(netif);
        esp_wifi_clear_default_wifi_driver_and_handlers(netif);
        netif = NULL;

    }
}

void WIFI_STATION_CONNECT(char *SSID,char *PASSWORD)
{
    uint8_t i =0;
    wifi_config_t wifi_config = {
        .sta ={

        },
    };
    while (*(SSID+i) != NULL)
    {
        wifi_config.sta.ssid[i] =  *(SSID+i);
        i++;
    }
    i=0;
    while (*(PASSWORD+i) != NULL)
    {
        wifi_config.sta.password[i] = *(PASSWORD+i);
        i++;
    }
    esp_wifi_disconnect();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    esp_wifi_connect();
}


static void smartconfig_task(void * parm);
static void event_smartconfig_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    HUY_SMART_CONFIG_EVENT_enum Check_Event = Smart_Config_Default;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        Check_Event = Smart_Config_Connected_WIFI;
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        Check_Event = Smart_Config_Found_Channel;
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        Check_Event = Smart_Config_Got_SSID_PASS;
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        Check_Event = Smart_Config_Over;
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
    Smart_Config_Handler_Callback(Check_Event);
}


void smart_config_WiFi_station(void (*Handler_Address))
{
    Smart_Config_Handler_Callback = Handler_Address;
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_smartconfig_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_smartconfig_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_smartconfig_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}


static void smartconfig_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

static void wifi_softap_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
} 

void WIFI_AP_INIT(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    netif = esp_netif_create_default_wifi_ap();


    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_softap_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void WIFI_AP_STOP(void)
{
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_softap_event_handler);
    esp_wifi_deauth_sta(0);
    esp_wifi_stop();
    esp_netif_destroy(netif);
    esp_wifi_clear_default_wifi_driver_and_handlers(netif);
    netif = NULL;
}


void WiFi_Init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

}


