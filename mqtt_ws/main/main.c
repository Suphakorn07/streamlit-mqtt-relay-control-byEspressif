#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

// Wi-Fi Configuration
#define WIFI_SSID      "suphakorn"
#define WIFI_PASS      "987654321"

// MQTT Configuration
#define MQTT_BROKER_URI "ws://test.mosquitto.org:8080/mqtt"
#define MQTT_TOPIC_SUBSCRIBE "/topic/wpan"
#define MQTT_TOPIC_PUBLISH "/topic/wpan"
// Relay GPIO Configuration
#define RELAY_GPIO_PIN  GPIO_NUM_2  

static const char *TAG = "MQTT_WS_EXAMPLE";

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

// Wi-Fi event handler function
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT) {
        if(event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if(event_id == WIFI_EVENT_STA_DISCONNECTED) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "Reconnecting to Wi-Fi...");
        }
    } else if(event_base == IP_EVENT) {
        if(event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        }
    }
}

// Wi-Fi initialization function
static void wifi_init(void)
{
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished");
}

// Function to control relay based on received message
void control_relay(char* message) {
    // ใช้ strtok เพื่อตัดช่องว่างหรืออักขระที่ไม่จำเป็นออก
    char *clean_message = strtok(message, "\n\r\t ");

    if (strcmp(clean_message, "on") == 0) {
        gpio_set_level(RELAY_GPIO_PIN, 0);  // เปิดรีเลย์
        ESP_LOGI(TAG, "Relay ON");
    } else if (strcmp(clean_message, "off") == 0) {
        gpio_set_level(RELAY_GPIO_PIN, 1);  // ปิดรีเลย์
        ESP_LOGI(TAG, "Relay OFF");
    } else {
        ESP_LOGI(TAG, "Unknown message: %s", clean_message);
    }
}


// MQTT event handler function
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;

  
    char received_message[128]; 

    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected successfully");
        // Subscribe to topic
        esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUBSCRIBE, 0);
        //esp_mqtt_client_publish(client, MQTT_TOPIC_PUBLISH, "ESP32 connected MQTT Broker", 0, 0, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT disconnected");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "Subscription successful, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Message published, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Received MQTT data");
        printf("Topic: %.*s\n", event->topic_len, event->topic);
        printf("Message: %.*s\n", event->data_len, event->data);

     
        memcpy(received_message, event->data, event->data_len);
        received_message[event->data_len] = '\0';  

        control_relay(received_message); 
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT error occurred");
        break;
    default:
        ESP_LOGI(TAG, "Unhandled MQTT event id: %d", event->event_id);
        break;
    }
    return ESP_OK;
}

// MQTT event handler (Wrapper)
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    mqtt_event_handler_cb(event_data);
}

// MQTT initialization function
void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,    // ใช้ define สำหรับ broker URI
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_mqtt_client_start(client);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    wifi_init();

	gpio_reset_pin(RELAY_GPIO_PIN);  
	gpio_set_direction(RELAY_GPIO_PIN, GPIO_MODE_OUTPUT);
	
	// ตั้งสถานะเริ่มต้นเป็นปิด (สำหรับ Active-Low Relay)
	gpio_set_level(RELAY_GPIO_PIN, 1); 

    // Wait for Wi-Fi connection
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

    // Start MQTT
    mqtt_app_start();
}
