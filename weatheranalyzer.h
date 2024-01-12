#ifndef WEATHER_ANALYZER_H
#define WEATHER_ANALYZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#define API_KEY "358c78d3bca3d8cb8c0fb9b48f44ed88"
#define API_URL "http://api.weatherstack.com/current?access_key=358c78d3bca3d8cb8c0fb9b48f44ed88&query=New%20York"

#define RAW_JSON_FILE "raw.json"
#define PROCESSED_TXT_FILE "processed.txt"
#define REPORT_TXT_FILE "report.txt"

#define GMAIL_USERNAME "ayeshatayyab027@gmail.com"
#define APP_PASSWORD "riew alhv xhkw ktow"  

#define TO_EMAIL "ayeshatayyab027@gmail.com"
#define SUBJECT "Weather Report Anomaly Alert"

struct WeatherData {
    double temperature;
    double humidity;
    double wind_speed;
    double pressure;
    double feels_like;
};

struct AccumulatedWeatherData {
    double total_temperature;
    double total_humidity;
    double total_wind_speed;
    double total_pressure;
    double total_feels_like;
    int count;
};

struct EmailData {
    char *payload_text;
    size_t payload_size;
    size_t sent;
};

size_t write_callback(void *contents, size_t size, size_t nmemb, char **data) {
    size_t realsize = size * nmemb;
    *data = realloc(*data, realsize + 1);
    if (*data == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    memcpy(*data, contents, realsize);
    (*data)[realsize] = '\0';
    return realsize;
}

void accumulateWeatherData(struct AccumulatedWeatherData *accumulated, const struct WeatherData *data) {
    accumulated->total_temperature += data->temperature;
    accumulated->total_humidity += data->humidity;
    accumulated->total_wind_speed += data->wind_speed;
    accumulated->total_pressure += data->pressure;
    accumulated->total_feels_like += data->feels_like;
    accumulated->count++;
}
void calculateAverageWeatherData(const struct AccumulatedWeatherData *accumulated, struct WeatherData *average) {
    average->temperature = accumulated->total_temperature / accumulated->count;
    average->humidity = accumulated->total_humidity / accumulated->count;
    average->wind_speed = accumulated->total_wind_speed / accumulated->count;
    average->pressure = accumulated->total_pressure / accumulated->count;
    average->feels_like = accumulated->total_feels_like / accumulated->count;
}



static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct EmailData *upload = (struct EmailData *)userp;

    if (upload->sent < upload->payload_size) {
        size_t bytes_to_copy = size * nmemb;
        if (bytes_to_copy > (upload->payload_size - upload->sent)) {
            bytes_to_copy = upload->payload_size - upload->sent;
        }

        memcpy(ptr, upload->payload_text + upload->sent, bytes_to_copy);
        upload->sent += bytes_to_copy;

        return bytes_to_copy;
    }

    return 0; 
}


int sendEmail(const char *to, const char *subject, const char *message) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *recipients = NULL;
    struct EmailData *upload = (struct EmailData *)malloc(sizeof(struct EmailData));

    if (upload == NULL) {
        fprintf(stderr, "Memory allocation error for upload\n");
        return EXIT_FAILURE;
    }

    // Construct email payload including headers
    upload->payload_size = snprintf(NULL, 0,
                                    "To: %s\r\n"
                                    "From: %s\r\n"
                                    "Subject: %s\r\n"
                                    "Content-Type: text/plain; charset=UTF-8\r\n\r\n"
                                    "%s",
                                    to, GMAIL_USERNAME, subject, message);

    upload->payload_text = (char *)malloc(upload->payload_size + 1);

    if (upload->payload_text == NULL) {
        fprintf(stderr, "Memory allocation error for payload text\n");
        free(upload);
        return EXIT_FAILURE;
    }

    snprintf(upload->payload_text, upload->payload_size + 1,
             "To: %s\r\n"
             "From: %s\r\n"
             "Subject: %s\r\n"
             "Content-Type: text/plain; charset=UTF-8\r\n\r\n"
             "%s",
             to, GMAIL_USERNAME, subject, message);

    upload->sent = 0;

    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Curl initialization failed\n");
        free(upload->payload_text);
        free(upload);
        return EXIT_FAILURE;
    }

    // Set email server details
    curl_easy_setopt(curl, CURLOPT_USERNAME, GMAIL_USERNAME);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, APP_PASSWORD);

    // Set recipient
    recipients = curl_slist_append(recipients, to);

    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, GMAIL_USERNAME);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, upload);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    free(upload->payload_text);
    free(upload);

    return (int)res;
}
#endif // WEATHER_ANALYZER_H

