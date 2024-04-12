#include <node_api.h>
#include <gpiod.h>
#include <assert.h>
#include <stdio.h>
#include <regex.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

char* getChipPath(char* chipName) {
    char* devPath = "/dev/";
    char* path;

    path = malloc(strlen(devPath) + strlen(chipName) + 1);
    strcpy(path, devPath);
    strcat(path, chipName);

    return path;
}

char* getChipLabel(struct gpiod_chip* chip) {
    char * chipLabel;

    #if GPIOD_MAJOR == 1
    chipLabel = (char *) gpiod_chip_label(chip);

    #elif GPIOD_MAJOR == 2
    struct gpiod_chip_info* chipInfo;

    chipInfo = gpiod_chip_get_info(chip);
    chipLabel = (char *)gpiod_chip_info_get_label(chipInfo);
    gpiod_chip_info_free(chipInfo);
    #endif

    return chipLabel;
}

napi_value detectChip(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    regex_t regex;
    char regexString[256];
    size_t regexStringLength;

    struct gpiod_chip* chip = NULL;
    struct dirent* directoryEntry = NULL;
    DIR *dirp = NULL;
    char* devPath = "/dev/";
    char* chipPrefix = "gpiochip";
    char* chipLabel = NULL;
    char *path = NULL;

    napi_value nvChipPtr;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    napi_get_value_string_utf8(env, args[0], regexString, sizeof(regexString), &regexStringLength);

    regcomp(&regex, regexString, 0);

    dirp = opendir(devPath);
    if(dirp != NULL) {
        while((directoryEntry = readdir(dirp))) {
            if(strncmp(directoryEntry->d_name, chipPrefix, strlen(chipPrefix)) == 0) {
                path = getChipPath(directoryEntry->d_name);

                chip = gpiod_chip_open(path);
                if(chip == NULL) {
                    continue;
                }

                chipLabel = getChipLabel(chip);
                if(regexec(&regex, chipLabel, 0, NULL, 0) == 0) {

                    napi_create_external(env, chip, NULL, NULL, &nvChipPtr);
                    return nvChipPtr;
                }
            }
        }
    }

    napi_throw_error(env, "ENOENT", "No chip detected.");
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}


//configure direction
#if GPIOD_MAJOR == 1
void configureDirection(const char* direction, struct gpiod_line_request_config* config, int* defaultValue) {
    if(strncmp("out", direction, strlen("out")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
    } else if(strncmp("in", direction, strlen("in")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_DIRECTION_INPUT;
    } else if(strncmp("high", direction, strlen("high")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
        *defaultValue = 1;
    } else if(strncmp("low", direction, strlen("low")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
        *defaultValue = 0;
    } else if(strncmp("as-is", direction, strlen("as-is")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_DIRECTION_AS_IS;
    }
}
#elif GPIOD_MAJOR == 2
void configureDirection(const char* direction, struct gpiod_line_settings* lineSettings) {
    if(strncmp("out", direction, strlen("out")) == 0) {
        gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_OUTPUT);
    } else if(strncmp("in", direction, strlen("in")) == 0) {
        gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_INPUT);
    } else if(strncmp("high", direction, strlen("high")) == 0) {
        gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_OUTPUT);
        gpiod_line_settings_set_output_value(lineSettings, GPIOD_LINE_VALUE_ACTIVE);
    } else if(strncmp("low", direction, strlen("low")) == 0) {
        gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_OUTPUT);
        gpiod_line_settings_set_output_value(lineSettings, GPIOD_LINE_VALUE_INACTIVE);
    } else if(strncmp("as-is", direction, strlen("as-is")) == 0) {
        gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_AS_IS);
    }
}
#endif

//configure active low
#if GPIOD_MAJOR == 1
void configureActiveLow(bool activeLow, struct gpiod_line_request_config* config) {
    if(activeLow) {
        config->flags = GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW;
    } else {
        config->flags = 0;
    }
}
#endif

//configure edge
#if GPIOD_MAJOR == 1
void configureEdge(const char* edge, struct gpiod_line_request_config* config) {
    if(strncmp("none", edge, strlen("none")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
    } else if(strncmp("rising", edge, strlen("rising")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_EVENT_RISING_EDGE;
    } else if(strncmp("falling", edge, strlen("falling")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE;
    } else if(strncmp("both", edge, strlen("both")) == 0) {
        config->request_type = GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES;
    }
}
#elif GPIOD_MAJOR == 2
void configureEdge(const char* edge, struct gpiod_line_settings* lineSettings) {
    if(strncmp("none", edge, strlen("none")) == 0) {
        gpiod_line_settings_set_edge_detection(lineSettings, GPIOD_LINE_EDGE_NONE);
    } else if(strncmp("rising", edge, strlen("rising")) == 0) {
        gpiod_line_settings_set_edge_detection(lineSettings, GPIOD_LINE_EDGE_RISING);
    } else if(strncmp("falling", edge, strlen("falling")) == 0) {
        gpiod_line_settings_set_edge_detection(lineSettings, GPIOD_LINE_EDGE_FALLING);
    } else if(strncmp("both", edge, strlen("both")) == 0) {
        gpiod_line_settings_set_edge_detection(lineSettings, GPIOD_LINE_EDGE_BOTH);
    }
}
#endif

//configure line
napi_value configureLine(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    #if GPIOD_MAJOR == 1
    struct gpiod_line* line;
    #elif GPIOD_MAJOR == 2
    struct gpiod_line_request* lineRequest;
    #endif

    uintptr_t linePtr;
    #if __WORDSIZE == 64
    bool lossless;
    napi_get_value_bigint_uint64(env, args[0], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[0], &linePtr);
    #endif

    #if GPIOD_MAJOR == 1
    line = (struct gpiod_line*)linePtr;
    #elif GPIOD_MAJOR == 2
    lineRequest = (struct gpiod_line_request*)linePtr;
    #endif

    int offset;
    napi_get_value_int32(env, args[1], &offset);

    char direction[16];
    size_t directionLength;
    napi_get_value_string_utf8(env, args[2], direction, 12, &directionLength);

    char edge[16];
    size_t edgeLength;
    napi_get_value_string_utf8(env, args[3], edge, 12, &edgeLength);

    napi_value napiActiveLow;
    bool activeLow;
    napi_get_named_property(env, args[4], "activeLow",  &napiActiveLow);
    napi_get_value_bool(env, napiActiveLow, &activeLow);

    napi_value napiReconfigureDirection;
    bool reconfigureDirection;
    napi_get_named_property(env, args[4], "reconfigureDirection",  &napiReconfigureDirection);
    napi_get_value_bool(env, napiReconfigureDirection, &reconfigureDirection);

    #if GPIOD_MAJOR == 1
        struct gpiod_line_request_config config;
        int defaultValue = 0;
        config.consumer = "onoff";

        int currentDirection;
        currentDirection = gpiod_line_direction(line);

        configureActiveLow(activeLow, &config);
        if((strncmp("none", edge, strlen("none")) != 0) && (strncmp("in", direction, strlen("in")) == 0)) {
            configureEdge(edge, &config);
        } else {
            if((currentDirection == GPIOD_LINE_DIRECTION_OUTPUT) && (strncmp("in", direction, strlen("in")) != 0)) {
                configureDirection("as-is", &config, NULL);
            } else {
                configureDirection(direction, &config, &defaultValue);
            }
        }
        gpiod_line_release(line);
        int lineConfigStatus = gpiod_line_request(line, &config, defaultValue);
        if(lineConfigStatus != 0) {
            napi_throw_error(env, "EIO", "Line request failed");
        }

    #elif GPIOD_MAJOR == 2
        struct gpiod_line_settings* lineSettings = gpiod_line_settings_new();
        gpiod_line_settings_set_active_low(lineSettings, activeLow);

        char *chipName = (char *)gpiod_line_request_get_chip_name(lineRequest);
        const char* chipPath = getChipPath(chipName);
        struct gpiod_chip *chip = gpiod_chip_open(chipPath);
        struct gpiod_line_info *lineInfo = gpiod_chip_get_line_info(chip, offset);
        int currentDirection = gpiod_line_info_get_direction(lineInfo);

        if((strncmp("none", edge, strlen("none")) != 0) && (strncmp("in", direction, strlen("in")) == 0)) {
            configureEdge(edge, lineSettings);
        } else {
            if((currentDirection == GPIOD_LINE_DIRECTION_OUTPUT) && (strncmp("in", direction, strlen("in")) != 0)) {
                configureDirection("as-is", lineSettings);
            } else {
                configureDirection(direction, lineSettings);
            }
        }


        struct gpiod_line_config *lineConfig = gpiod_line_config_new();

        gpiod_line_config_add_line_settings(lineConfig, (const unsigned int *)&offset, 1, lineSettings);
        gpiod_line_request_reconfigure_lines(lineRequest, lineConfig);
    #endif

    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}

napi_value getLine(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_chip* chip;
    napi_get_value_external(env, args[0], (void**)&chip);

    int offset;
    napi_get_value_int32(env, args[1], &offset);

    #if GPIOD_MAJOR == 1
        struct gpiod_line *line;
        line = gpiod_chip_get_line(chip, offset);

        napi_value result;
        #if __WORDSIZE == 64
        napi_create_bigint_uint64(env, (uintptr_t)line, &result);
        #else
        napi_create_uint32(env, (uintptr_t)line, &result);
        #endif

        return result;
    #elif GPIOD_MAJOR == 2
        struct gpiod_line_settings* lineSettings;
        lineSettings = gpiod_line_settings_new();
        gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_AS_IS);
        struct gpiod_line_config* lineConfig = gpiod_line_config_new();
        gpiod_line_config_add_line_settings(lineConfig, (const unsigned int *)&offset, 1, lineSettings);
        struct gpiod_request_config *requestConfig = gpiod_request_config_new();
        gpiod_request_config_set_consumer(requestConfig, "onoff");
        struct gpiod_line_request* lineRequest;
        lineRequest = gpiod_chip_request_lines(chip, requestConfig, lineConfig);

        napi_value result;
        #if __WORDSIZE == 64
        napi_create_bigint_uint64(env, (uintptr_t)lineRequest, &result);
        #else
        napi_create_uint32(env, (uintptr_t)lineRequest, &result);
        #endif

        return result;
    #endif
}

napi_value setLineValue(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    int value;
    napi_get_value_int32(env, args[1], &value);

    uintptr_t linePtr;
    #if __WORDSIZE == 64
    bool lossless;
    napi_get_value_bigint_uint64(env, args[0], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[0], &linePtr);
    #endif

    #if GPIOD_MAJOR == 1
        struct gpiod_line *line;
        line = (struct gpiod_line*)linePtr;
        int status = gpiod_line_set_value (line, value);

    #elif GPIOD_MAJOR == 2
        struct gpiod_line_request* lineRequest;
        lineRequest = (struct gpiod_line_request*)linePtr;

        int numLines = gpiod_line_request_get_num_requested_lines(lineRequest);
        unsigned int *offsets = malloc(sizeof(int) * numLines);
        gpiod_line_request_get_requested_offsets(lineRequest, offsets, numLines);
        int status = gpiod_line_request_set_value(lineRequest, offsets[0], value);
    #endif

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

napi_value getLineValue(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    uintptr_t linePtr;
    #if __WORDSIZE == 64
    bool lossless;
    napi_get_value_bigint_uint64(env, args[0], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[0], &linePtr);
    #endif

    #if GPIOD_MAJOR == 1
        struct gpiod_line *line;
        line = (struct gpiod_line*)linePtr;
        napi_get_value_external(env, args[0], (void**)&line);
        gpiod_line_active_state(line);
        int status = gpiod_line_get_value (line);

    #elif GPIOD_MAJOR == 2
        struct gpiod_line_request* lineRequest;
        lineRequest = (struct gpiod_line_request*)linePtr;

        int numLines = gpiod_line_request_get_num_requested_lines(lineRequest);
        unsigned int *offsets = malloc(sizeof(int) * numLines);
        gpiod_line_request_get_requested_offsets(lineRequest, offsets, numLines);
        int status = gpiod_line_request_get_value(lineRequest, offsets[0]);
    #endif

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

napi_value waitForEvent(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    uintptr_t linePtr;
    #if __WORDSIZE == 64
    bool lossless;
    napi_get_value_bigint_uint64(env, args[0], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[0], &linePtr);
    #endif

    #if GPIOD_MAJOR == 1
        struct gpiod_line *line;
        struct timespec timeout;
        timeout.tv_sec = 1;
        timeout.tv_nsec = 0;
        line = (struct gpiod_line*)linePtr;
        int status = gpiod_line_event_wait(line, &timeout);

        if(status > 0) {
            struct gpiod_line_event event;
            gpiod_line_event_read(line, &event);
        }

    #elif GPIOD_MAJOR == 2
        int64_t timeout_ns = 1000000;
        struct gpiod_line_request* lineRequest;
        lineRequest = (struct gpiod_line_request*)linePtr;
        int status = gpiod_line_request_wait_edge_events(lineRequest, timeout_ns);

        if(status > 0) {
            struct gpiod_edge_event_buffer* buffer = gpiod_edge_event_buffer_new(1);
            gpiod_line_request_read_edge_events(lineRequest, buffer, 1);
            gpiod_edge_event_buffer_free(buffer);
        }
    #endif

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor descriptor[] = {
    DECLARE_NAPI_METHOD("detectChip", detectChip),
    DECLARE_NAPI_METHOD("getLine", getLine),
    DECLARE_NAPI_METHOD("configureLine", configureLine),
    DECLARE_NAPI_METHOD("setLineValue", setLineValue),
    DECLARE_NAPI_METHOD("getLineValue", getLineValue),
    DECLARE_NAPI_METHOD("waitForEvent", waitForEvent)

  };
  status = napi_define_properties(env, exports, sizeof(descriptor)/sizeof(descriptor[0]), descriptor);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)