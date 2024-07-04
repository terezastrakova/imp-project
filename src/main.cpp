#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "ssd1306.h"

#include "APDS9960.h"

#define MAIN_TAG "MAIN"
#define SSD1306_TAG "SSD1306"
#define STORAGE_NAMESPACE "storage"

#define IMAGES 10
#define HOURS 24
#define MINUTES 12

// default index value for hours
int32_t h_index = 8;
// default index value for minutes
int32_t m_index = 0;

// array of hours
char *hours[HOURS] = {
    "00", "01", "02", "03",
    "04", "05", "06", "07",
    "08", "09", "10", "11",
    "12", "13", "14", "15",
    "16", "17", "18", "19",
    "20", "21", "22", "23"};

// array of strings of possible minutes values
char *minutes[MINUTES] = {
    "   00", "   05", "   10", "   15",
    "   20", "   25", "   30", "   35",
    "   40", "   45", "   50", "   55"};

char *help[9] = {
    "Main menu:      ",
    " FAR Setting    ",
    "     Alarm      ",
    "Setting Alarm:  ",
    " UP/DOWN Change ",
    "         Value  ",
    " RIGHT Minutes  ",
    " LEFT Hours     ",
    " FAR Save & Exit"};

// for testing purposes
char *scrolltest[9] = {
    "---#=           ",
    "    #-          ",
    "    =#          ",
    "      #-        ",
    "      =#        ",
    "        #-      ",
    "        =#      ",
    "          #-    ",
    "          =#----"};

uint8_t alarm[] = {
    // original image: https://icon-icons.com/icon/alarm/153594
    // 'alarmpng', 56x56px
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0x81, 0xff, 0xff, 0x01, 0xff, 0xe3, 0xff, 0x80, 0xff, 0xfe, 0x03, 0xf8, 0x00, 0x1f, 0xc0, 0x7f, 0xfc, 0x07, 0xe0, 0x00, 0x07, 0xe0, 0x3f, 0xfc, 0x0f, 0x80, 0x00, 0x01, 0xf0, 0x3f, 0xfc, 0x1f, 0x00, 0x00, 0x00, 0xf8, 0x3f, 0xfe, 0x3c, 0x00, 0x7f, 0x00, 0x3c, 0x7f, 0xff, 0xf8, 0x07, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0xff, 0xc3, 0xff, 0x03, 0xff, 0xff, 0xc1, 0xff, 0xc3, 0xff, 0x83, 0xff, 0xff, 0x83, 0xff, 0xc3, 0xff, 0xc1, 0xff, 0xff, 0x83, 0xff, 0xc3, 0xff, 0xc1, 0xff, 0xff, 0x07, 0xff, 0xc3, 0xff, 0xe0, 0xff, 0xff, 0x07, 0xff, 0xc3, 0xff, 0xe0, 0xff, 0xff, 0x07, 0xff, 0xc3, 0xff, 0xf0, 0xff, 0xfe, 0x0f, 0xff, 0xc3, 0xff, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xc1, 0xff, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xc0, 0x01, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xc0, 0x00, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xc0, 0x00, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xc0, 0x00, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xc0, 0x01, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xf0, 0x7f, 0xff, 0x07, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0x83, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xfe, 0x00, 0x3c, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/**
 * @brief Logging current gesture.
 *
 * @param gesture return value of `apds.readGesture()`
 */
void gestureLog(int gesture);

/**
 * @brief Set the up spi interface object.
 *
 * @param dev SSD1306 structure
 */
void setup_spi_interface(SSD1306_t *dev);

/**
 * @brief Function with main loop of the program.
 *
 * @param apds APDS9960 Class
 * @param dev SSD1306 structure
 */
void runAlarmApp(SparkFun_APDS9960 &apds, SSD1306_t *dev);

/**
 * @brief Function sets `h_index` and changes hours on screen.
 *
 * @param apds APDS9960 Class
 * @param dev SSD1306 structure
 */
void settingHours(SparkFun_APDS9960 &apds, SSD1306_t *dev);

/**
 * @brief Function sets `m_index` and changes minutes on screen.
 *
 * @param apds APDS9960 Class
 * @param dev SSD1306 structure
 * @return true On LEFT gesture - continue with setting hours.
 * @return false On FAR gesture - save and exit.
 */
bool settingMinutes(SparkFun_APDS9960 &apds, SSD1306_t *dev);

/**
 * @brief Function displays help on screen.
 *
 * @param apds APDS9960 Class
 * @param dev SSD1306 structure
 */
void showHelp(SparkFun_APDS9960 &apds, SSD1306_t *dev);

/**
 * @brief Scroll up used in help screen.
 *
 * @param dev SSD1306 structure
 */
void scrollUp(SSD1306_t *dev);

/**
 * @brief Scroll down used in help screen.
 *
 * @param dev SSD1306 structure
 */
void scrollDown(SSD1306_t *dev);

/**
 * @brief Display main screen with currently set alarm.
 *
 * @param dev SSD1306 structure
 */
void printMainScreen(SSD1306_t *dev);

/**
 * @brief Display updated value of minutes.
 *
 * @param dev SSD1306 structure
 */
void printMinutesTime(SSD1306_t *dev);

/**
 * @brief Display updated value of hours.
 *
 * @param dev SSD1306 structure
 */
void printHoursTime(SSD1306_t *dev);

/**
 * @brief Read values of `h_index` and `m_index` from flash.
 *
 * @return esp_err_t
 */
esp_err_t read_nvs();

/**
 * @brief Write values of `h_index` and `m_index` to flash.
 *
 * @return esp_err_t
 */
esp_err_t write_nvs();

extern "C" void app_main()
{
    /* Initialize NVS */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // read values from nvs
    err = read_nvs();
    if (err != ESP_OK)
        printf("Error (%s) reading data from NVS!\n", esp_err_to_name(err));
    else
        ESP_LOGI(MAIN_TAG, "h = %d, m = %d", h_index, m_index);

    /* display setup */
    SSD1306_t dev;
    setup_spi_interface(&dev);

    /* loading screen */
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text_x3(&dev, 2, "ALARM", 5, false);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    /* apds-9960 display init */
    SparkFun_APDS9960 apds = SparkFun_APDS9960();
    if (apds.init())
        ESP_LOGI(MAIN_TAG, "APDS-9960 initialization complete");
    else
        ESP_LOGI(MAIN_TAG, "Something went wrong during APDS-9960 init!");

    /* alaram icon for fun */
    ssd1306_clear_screen(&dev, false);
    ssd1306_bitmaps(&dev, 36, 4, alarm, 56, 56, true);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    for (int i = 0; i < 64; i++)
    {
        ssd1306_wrap_arround(&dev, SCROLL_UP, 0, 127, 0);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    runAlarmApp(apds, &dev); // <- main loop of the program
}

void runAlarmApp(SparkFun_APDS9960 &apds, SSD1306_t *dev)
{
    // Start running the APDS-9960 gesture sensor engine
    if (apds.enableGestureSensor(false))
    {
        ESP_LOGI(MAIN_TAG, "Gesture sensor is now running");
    }
    else
    {
        ESP_LOGI(MAIN_TAG, "Something went wrong during gesture sensor init!");
    }

    printMainScreen(dev);
    esp_err_t err;
    while (1)
    {
        if (apds.isGestureAvailable())
        {
            int g = apds.readGesture();
            gestureLog(g);
            switch (g)
            {
            case DIR_FAR:
                settingHours(apds, dev);
                err = write_nvs();
                if (err != ESP_OK)
                    printf("Error (%s) saving data to NVS!\n", esp_err_to_name(err));
                else
                    ESP_LOGI(MAIN_TAG, "h = %d, m = %d", h_index, m_index);
                printMainScreen(dev);
                break;
            case DIR_UP:
                break;
            case DIR_DOWN:
                break;
            case DIR_LEFT:
                break;
            case DIR_RIGHT:
                showHelp(apds, dev);
                printMainScreen(dev);
                break;
            }
        }
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

void settingHours(SparkFun_APDS9960 &apds, SSD1306_t *dev)
{
    ssd1306_clear_screen(dev, false);
    ssd1306_display_text(dev, 0, "  Setting Alarm ", 16, true);
    printHoursTime(dev);

    while (1)
    {
        if (apds.isGestureAvailable())
        {
            int g = apds.readGesture();
            gestureLog(g);
            switch (g)
            {
            case DIR_UP:
                h_index = (h_index + 1) % HOURS;
                ESP_LOGI("HOURS", "%d", h_index);
                ssd1306_display_text_x3(dev, 2, hours[h_index], 2, true);
                break;
            case DIR_DOWN:
                h_index = h_index - 1;
                if (h_index == -1)
                    h_index = HOURS - 1;
                ESP_LOGI("HOURS", "%d", h_index);
                ssd1306_display_text_x3(dev, 2, hours[h_index], 2, true);
                break;
            case DIR_LEFT:
                break;
            case DIR_RIGHT:
                if (!settingMinutes(apds, dev))
                    return;
                printHoursTime(dev);
                break;
            case DIR_FAR:
                return;
                break;
            }
        }
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

bool settingMinutes(SparkFun_APDS9960 &apds, SSD1306_t *dev)
{
    printMinutesTime(dev);

    while (1)
    {
        if (apds.isGestureAvailable())
        {
            int g = apds.readGesture();
            gestureLog(g);
            switch (g)
            {
            case DIR_UP:
                m_index = (m_index + 1) % MINUTES;
                ESP_LOGI("MINUTES", "%d", m_index);
                printMinutesTime(dev);
                break;
            case DIR_DOWN:
                m_index = m_index - 1;
                if (m_index == -1)
                    m_index = MINUTES - 1;
                ESP_LOGI("MINUTES", "%d", m_index);
                printMinutesTime(dev);
                break;
            case DIR_LEFT:
                return true; // continue with setting hours
                break;
            case DIR_RIGHT:
                break;
            case DIR_FAR:
                return false; // go to main screen
                break;
            }
        }
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

void showHelp(SparkFun_APDS9960 &apds, SSD1306_t *dev)
{
    /* -------- load help screen -------- */
    ssd1306_clear_screen(dev, false);
    ssd1306_display_text(dev, 0, "      Help      ", 16, true);
    ssd1306_display_text(dev, 7, "go back: LEFT   ", 16, true);
    for (int line = 0; line < 6; line++)
    {
        ssd1306_display_text(dev, line + 1, help[line], 16, false);
    }

    bool top = true;

    while (1)
    {
        if (apds.isGestureAvailable())
        {
            int g = apds.readGesture();
            gestureLog(g);
            switch (g)
            {
            case DIR_UP:
                if (top)
                {
                    scrollUp(dev);
                    top = false;
                }
                break;
            case DIR_DOWN:
                if (!top)
                {
                    scrollDown(dev);
                    top = true;
                }
                break;
            case DIR_LEFT:
                return; // go back to main screen
                break;
            case DIR_RIGHT:
                break;
            case DIR_FAR:
                break;
            }
        }
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

void scrollUp(SSD1306_t *dev)
{
    // Scroll Up
    ssd1306_software_scroll(dev, ((*dev)._pages - 2), 1);
    for (int line = 6; line < 9; line++)
    {
        ssd1306_scroll_text(dev, help[line], 16, false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void scrollDown(SSD1306_t *dev)
{
    // Scroll Down
    ssd1306_software_scroll(dev, 1, ((*dev)._pages - 2));
    for (int line = 2; line >= 0; line--)
    {
        ssd1306_scroll_text(dev, help[line], 16, false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void printMainScreen(SSD1306_t *dev)
{
    ssd1306_clear_screen(dev, false);
    ssd1306_display_text(dev, 0, "    My Alarm    ", 16, true);
    ssd1306_display_text_x3(dev, 2, minutes[m_index], 5, false);
    ssd1306_display_text_x3(dev, 2, "  :", 3, false);
    ssd1306_display_text_x3(dev, 2, hours[h_index], 2, false);
    ssd1306_display_text(dev, 7, "help: RIGHT     ", 16, true);
}

void printMinutesTime(SSD1306_t *dev)
{
    ssd1306_display_text_x3(dev, 2, minutes[m_index], 5, true);
    ssd1306_display_text_x3(dev, 2, "  :", 3, false);
    ssd1306_display_text_x3(dev, 2, hours[h_index], 2, false);
}

void printHoursTime(SSD1306_t *dev)
{
    ssd1306_display_text_x3(dev, 2, minutes[m_index], 5, false);
    ssd1306_display_text_x3(dev, 2, "  :", 3, false);
    ssd1306_display_text_x3(dev, 2, hours[h_index], 2, true);
}

void gestureLog(int gesture)
{
    switch (gesture)
    {
    case DIR_UP:
        ESP_LOGI(MAIN_TAG, "UP");
        break;
    case DIR_DOWN:
        ESP_LOGI(MAIN_TAG, "DOWN");
        break;
    case DIR_LEFT:
        ESP_LOGI(MAIN_TAG, "LEFT");
        break;
    case DIR_RIGHT:
        ESP_LOGI(MAIN_TAG, "RIGHT");
        break;
    case DIR_NEAR:
        ESP_LOGI(MAIN_TAG, "NEAR");
        break;
    case DIR_FAR:
        ESP_LOGI(MAIN_TAG, "FAR");
        break;
    default:
        ESP_LOGI(MAIN_TAG, "NONE");
    }
}

void setup_spi_interface(SSD1306_t *dev)
{
    ESP_LOGI(SSD1306_TAG, "INTERFACE is SPI");
    ESP_LOGI(SSD1306_TAG, "CONFIG_MOSI_GPIO=%d", CONFIG_MOSI_GPIO);
    ESP_LOGI(SSD1306_TAG, "CONFIG_SCLK_GPIO=%d", CONFIG_SCLK_GPIO);
    ESP_LOGI(SSD1306_TAG, "CONFIG_CS_GPIO=%d", CONFIG_CS_GPIO);
    ESP_LOGI(SSD1306_TAG, "CONFIG_DC_GPIO=%d", CONFIG_DC_GPIO);
    ESP_LOGI(SSD1306_TAG, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
    spi_master_init(dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);

#if CONFIG_FLIP
    dev._flip = true;
    ESP_LOGW(SSD1306_TAG, "Flip upside down");
#endif

    ESP_LOGI(SSD1306_TAG, "Panel is 128x64");
    ssd1306_init(dev, 128, 64);
}

esp_err_t read_nvs()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Read
    err = nvs_get_i32(my_handle, "h_index", &h_index);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        return err;

    err = nvs_get_i32(my_handle, "m_index", &m_index);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        return err;

    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t write_nvs()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Write hours index
    err = nvs_set_i32(my_handle, "h_index", h_index);
    if (err != ESP_OK)
        return err;

    // Write minutes index
    err = nvs_set_i32(my_handle, "m_index", m_index);
    if (err != ESP_OK)
        return err;

    // Commit written value.
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
        return err;

    // Close
    nvs_close(my_handle);
    return ESP_OK;
}
