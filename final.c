#include "weatheranalyzer.h"


int main() {
    CURL *curl;
    CURLcode res;
    char *response_data = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Curl initialization failed\n");
        return EXIT_FAILURE;
    }

    curl_easy_setopt(curl, CURLOPT_URL, API_URL);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Curl request failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(response_data);
        return EXIT_FAILURE;
    }

    curl_easy_cleanup(curl);

    FILE *raw_json_file = fopen(RAW_JSON_FILE, "w");
    if (raw_json_file == NULL) {
        fprintf(stderr, "Failed to open raw JSON file for writing\n");
        free(response_data);
        return EXIT_FAILURE;
    }
    fprintf(raw_json_file, "%s", response_data);
    fclose(raw_json_file);

    json_error_t json_error;
    json_t *root = json_loads(response_data, 0, &json_error);
    free(response_data);

    if (!root) {
        fprintf(stderr, "JSON parsing error: %s\n", json_error.text);
        return EXIT_FAILURE;
    }

    struct WeatherData currentData;
    currentData.temperature = json_number_value(json_object_get(json_object_get(root, "current"), "temperature"));
    currentData.humidity = json_number_value(json_object_get(json_object_get(root, "current"), "humidity"));
    currentData.wind_speed = json_number_value(json_object_get(json_object_get(root, "current"), "wind_speed"));
    currentData.pressure = json_number_value(json_object_get(json_object_get(root, "current"), "pressure"));
    currentData.feels_like = json_number_value(json_object_get(json_object_get(root, "current"), "feelslike"));

    FILE *processed_txt_file = fopen(PROCESSED_TXT_FILE, "a");
    if (processed_txt_file == NULL) {
        fprintf(stderr, "Failed to open processed TXT file for writing\n");
        json_decref(root);
        return EXIT_FAILURE;
    }
    fprintf(processed_txt_file, "Temperature: %.2f\nHumidity: %.2f\nWind Speed: %.2f\nPressure: %.2f\nFeels Like: %.2f\n",
            currentData.temperature, currentData.humidity, currentData.wind_speed, currentData.pressure, currentData.feels_like);
    fclose(processed_txt_file);

    struct AccumulatedWeatherData accumulated = {0};
    accumulateWeatherData(&accumulated, &currentData);

    FILE *prev_processed_txt_file = fopen(PROCESSED_TXT_FILE, "r");
    if (prev_processed_txt_file != NULL) {
        while (fscanf(prev_processed_txt_file, "Temperature: %lf\nHumidity: %lf\nWind Speed: %lf\nPressure: %lf\nFeels Like: %lf\n",
                      &currentData.temperature, &currentData.humidity, &currentData.wind_speed, &currentData.pressure, &currentData.feels_like) == 5) {
            accumulateWeatherData(&accumulated, &currentData);
        }
        fclose(prev_processed_txt_file);
    }

    calculateAverageWeatherData(&accumulated, &currentData);

    FILE *report_txt_file = fopen(REPORT_TXT_FILE, "w");
    if (report_txt_file == NULL) {
        fprintf(stderr, "Failed to open report TXT file for writing\n");
        json_decref(root);
        return EXIT_FAILURE;
    }

    double temp_threshold = 10.0;  
    double humidity_threshold = 10.0;
    double wind_speed_threshold = 20.0;  
    double pressure_threshold = 10.0;
    double feellikes_speed_threshold = 20.0;

    int check = 0;

if (currentData.temperature > (currentData.temperature + temp_threshold)) {
    fprintf(report_txt_file, "Temperature Anomaly: %.2f\n", currentData.temperature);
    printf("Temperature Anomaly: %.2f\n", currentData.temperature);
    char email_message[500];
    snprintf(email_message, sizeof(email_message), "Temperature Anomaly Detected! %.2f\n", currentData.temperature);
    sendEmail(TO_EMAIL, SUBJECT, email_message);
    check = 1;
} else {
    
    if (currentData.temperature > 30.0) {
        fprintf(report_txt_file, "High Temperature: %.2f\n", currentData.temperature);
        printf("High Temperature: %.2f\n", currentData.temperature);
    } else if (currentData.temperature < 10.0) {
        fprintf(report_txt_file, "Low Temperature: %.2f\n", currentData.temperature);
        printf("Low Temperature: %.2f\n", currentData.temperature);
    } else {
        fprintf(report_txt_file, "Normal Temperature: %.2f\n", currentData.temperature);
        printf("Normal Temperature: %.2f\n", currentData.temperature);
    }
}

if (currentData.humidity > (currentData.humidity + humidity_threshold)) {
    fprintf(report_txt_file, "Humidity Anomaly: %.2f\n", currentData.humidity);
    printf("Humidity Anomaly: %.2f\n", currentData.humidity);
    char email_message[500];
    snprintf(email_message, sizeof(email_message), "Humidity Anomaly Detected! %.2f\n", currentData.humidity);
    sendEmail(TO_EMAIL, SUBJECT, email_message);
    check = 1;
} else {
   
    if (currentData.humidity > 80.0) {
        fprintf(report_txt_file, "High Humidity: %.2f\n", currentData.humidity);
        printf("High Humidity: %.2f\n", currentData.humidity);
    } else if (currentData.humidity < 30.0) {
        fprintf(report_txt_file, "Low Humidity: %.2f\n", currentData.humidity);
        printf("Low Humidity: %.2f\n", currentData.humidity);
    } else {
        fprintf(report_txt_file, "Normal Humidity: %.2f\n", currentData.humidity);
        printf("Normal Humidity: %.2f\n", currentData.humidity);
    }
}

if (currentData.wind_speed > (currentData.wind_speed + wind_speed_threshold)) {
    fprintf(report_txt_file, "Wind Speed Anomaly: %.2f\n", currentData.wind_speed);
    printf("Wind Speed Anomaly: %.2f\n", currentData.wind_speed);
    char email_message[500];
    snprintf(email_message, sizeof(email_message), "Wind Speed Anomaly Detected! %.2f\n", currentData.wind_speed);
    sendEmail(TO_EMAIL, SUBJECT, email_message);
    check = 1;
} else {
   
    if (currentData.wind_speed > 40.0) {
        fprintf(report_txt_file, "High Wind Speed: %.2f\n", currentData.wind_speed);
        printf("High Wind Speed: %.2f\n", currentData.wind_speed);
    } else if (currentData.wind_speed < 5.0) {
        fprintf(report_txt_file, "Low Wind Speed: %.2f\n", currentData.wind_speed);
        printf("Low Wind Speed: %.2f\n", currentData.wind_speed);
    } else {
        fprintf(report_txt_file, "Normal Wind Speed: %.2f\n", currentData.wind_speed);
        printf("Normal Wind Speed: %.2f\n", currentData.wind_speed);
    }
}

if (currentData.pressure > (currentData.pressure + pressure_threshold)) {
    fprintf(report_txt_file, "Pressure Anomaly: %.2f\n", currentData.pressure);
    printf("Pressure Anomaly: %.2f\n", currentData.pressure);
    char email_message[500];
    snprintf(email_message, sizeof(email_message), "Pressure Anomaly Detected! %.2f\n", currentData.pressure);
    sendEmail(TO_EMAIL, SUBJECT, email_message);
    check = 1;
} else {
   
    if (currentData.pressure > 1015.0) {
        fprintf(report_txt_file, "High Pressure: %.2f\n", currentData.pressure);
        printf("High Pressure: %.2f\n", currentData.pressure);
    } else if (currentData.pressure < 990.0) {
        fprintf(report_txt_file, "Low Pressure: %.2f\n", currentData.pressure);
        printf("Low Pressure: %.2f\n", currentData.pressure);
    } else {
        fprintf(report_txt_file, "Normal Pressure: %.2f\n", currentData.pressure);
        printf("Normal Pressure: %.2f\n", currentData.pressure);
    }
}

if (currentData.feels_like > (currentData.feels_like + feellikes_speed_threshold)) {
    fprintf(report_txt_file, "Feels Like Anomaly: %.2f\n", currentData.feels_like);
    printf("Feels Like Anomaly: %.2f\n", currentData.feels_like);
    char email_message[500];
    snprintf(email_message, sizeof(email_message), "Feels Like Anomaly Detected! %.2f\n", currentData.feels_like);
    sendEmail(TO_EMAIL, SUBJECT, email_message);
    check = 1;
} else {
   
    if (currentData.feels_like > 35.0) {
        fprintf(report_txt_file, "Feels Very Warm: %.2f\n", currentData.feels_like);
        printf("Feels Very Warm: %.2f\n", currentData.feels_like);
    } else if (currentData.feels_like < 10.0) {
        fprintf(report_txt_file, "Feels Very Cold: %.2f\n", currentData.feels_like);
        printf("Feels Very Cold: %.2f\n", currentData.feels_like);
    } else {
        fprintf(report_txt_file, "Feels Comfortable: %.2f\n", currentData.feels_like);
        printf("Feels Comfortable: %.2f\n", currentData.feels_like);
    }
}

if (check) {
        fclose(report_txt_file);  
        printf("\nEmail sent for anomalies.\n");
    } else {
       
        fprintf(report_txt_file, "\nAverage Temperature: %.2f\nAverage Humidity: %.2f\nAverage Wind Speed: %.2f\nAverage Pressure: %.2f\nAverage Feels Like: %.2f\n",
                currentData.temperature, currentData.humidity, currentData.wind_speed, currentData.pressure, currentData.feels_like);

        char email_message[500];
        snprintf(email_message, sizeof(email_message), "Weather Report: No anomalies detected.\n");
        sendEmail(TO_EMAIL, SUBJECT, email_message);

        printf("\nEmail sent for overall averages.\n");
    }

    fclose(report_txt_file);

  
    json_decref(root);

    return EXIT_SUCCESS;
}