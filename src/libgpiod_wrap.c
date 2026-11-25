#include <node_api.h>
#include <gpiod.h>
#include <assert.h>
#include <stdio.h>
#include <regex.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char* getChipPath(char* chipName) {
    char* devPath = "/dev/";
    char* path;

    path = malloc(strlen(devPath) + strlen(chipName) + 1);
    if(path == NULL) {
        return NULL;
    }

    strcpy(path, devPath);
    strcat(path, chipName);

    return path;
}

char* getChipLabel(struct gpiod_chip* chip) {
    char * chipLabel = NULL;
    char *label = NULL;

    struct gpiod_chip_info* chipInfo;

    chipInfo = gpiod_chip_get_info(chip);
    label = (char *)gpiod_chip_info_get_label(chipInfo);
    if (label) {
        size_t len = strlen(label);
        chipLabel = malloc(len + 1);
        if (chipLabel == NULL) {
            gpiod_chip_info_free(chipInfo);
            return NULL;
        }
        strcpy(chipLabel, label);
        chipLabel[len] = '\0';
    } else {
        chipLabel = NULL;
        label = NULL;
    }
    gpiod_chip_info_free(chipInfo);
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
                    free(path);
                    continue;
                }

                chipLabel = getChipLabel(chip);
                if(regexec(&regex, chipLabel, 0, NULL, 0) == 0) {
                    napi_create_external(env, chip, NULL, NULL, &nvChipPtr);
                    free(path);
                    free(chipLabel);
                    regfree(&regex);
                    closedir(dirp);
                    return nvChipPtr;
                } else {
                    gpiod_chip_close(chip);
                    free(path);
                    free(chipLabel);
                }
            }
        }
        regfree(&regex);
        closedir(dirp);
    }

    napi_throw_error(env, "ENOENT", "No chip detected.");
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}


//configure direction
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
//configure edge
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
//configure line
napi_value configureLine(napi_env env, napi_callback_info info) {
    size_t argc = 6;
    napi_value args[6];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_chip* chip;
    napi_get_value_external(env, args[0], (void**)&chip);

    struct gpiod_line_request* lineRequest;

    uintptr_t linePtr;
    #if UINTPTR_MAX == UINT64_MAX
    bool lossless;
    napi_get_value_bigint_uint64(env, args[1], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[1], &linePtr);
    #endif

    lineRequest = (struct gpiod_line_request*)linePtr;

    int offset;
    napi_get_value_int32(env, args[2], &offset);

    char direction[16];
    size_t directionLength;
    napi_get_value_string_utf8(env, args[3], direction, 12, &directionLength);

    char edge[16];
    size_t edgeLength;
    napi_get_value_string_utf8(env, args[4], edge, 12, &edgeLength);

    napi_value napiActiveLow;
    bool activeLow;
    napi_get_named_property(env, args[5], "activeLow",  &napiActiveLow);
    napi_get_value_bool(env, napiActiveLow, &activeLow);

    napi_value napiReconfigureDirection;
    bool reconfigureDirection;
    napi_get_named_property(env, args[5], "reconfigureDirection",  &napiReconfigureDirection);
    napi_get_value_bool(env, napiReconfigureDirection, &reconfigureDirection);

        struct gpiod_line_settings* lineSettings = gpiod_line_settings_new();
        gpiod_line_settings_set_active_low(lineSettings, activeLow);

        napi_value napiDebounceTimeout;
        double debounceTimeout;
        bool hasDebounceTimeout = false;
        napi_has_named_property(env, args[5], "debounceTimeout", &hasDebounceTimeout);
        if (hasDebounceTimeout &&
            napi_get_named_property(env, args[5], "debounceTimeout", &napiDebounceTimeout) == napi_ok &&
            napi_get_value_double(env, napiDebounceTimeout, &debounceTimeout) == napi_ok &&
            debounceTimeout > 0) {
            if (debounceTimeout > UINT32_MAX / 1000.0) {
                gpiod_line_settings_free(lineSettings);
                napi_throw_range_error(env, NULL, "debounceTimeout is too large");
                return NULL;
            }
            gpiod_line_settings_set_debounce_period_us(lineSettings, (unsigned long)(debounceTimeout * 1000));
        }

        napi_value napiBias;
        char bias[16];
        size_t biasLength;
        bool hasBias = false;
        napi_has_named_property(env, args[5], "bias", &hasBias);
        if (hasBias) {
            napi_get_named_property(env, args[5], "bias",  &napiBias);
            napi_get_value_string_utf8(env, napiBias, bias, sizeof(bias), &biasLength);
            if (strcmp(bias, "pull-up") == 0) {
                gpiod_line_settings_set_bias(lineSettings, GPIOD_LINE_BIAS_PULL_UP);
            } else if (strcmp(bias, "pull-down") == 0) {
                gpiod_line_settings_set_bias(lineSettings, GPIOD_LINE_BIAS_PULL_DOWN);
            } else if (strcmp(bias, "disabled") == 0) {
                gpiod_line_settings_set_bias(lineSettings, GPIOD_LINE_BIAS_DISABLED);
            } else {
                gpiod_line_settings_set_bias(lineSettings, GPIOD_LINE_BIAS_AS_IS);
            }
        }

        struct gpiod_line_info *lineInfo = gpiod_chip_get_line_info(chip, offset);
        int currentDirection = gpiod_line_info_get_direction(lineInfo);

        if((strncmp("none", edge, strlen("none")) != 0) && (strncmp("in", direction, strlen("in")) == 0)) {
            configureEdge(edge, lineSettings);
        }
        
        if((currentDirection == GPIOD_LINE_DIRECTION_OUTPUT) && (strncmp("in", direction, strlen("in")) != 0) && (reconfigureDirection == false)) {
            configureDirection("as-is", lineSettings);
        } else {
            configureDirection(direction, lineSettings);
        }

        struct gpiod_line_config *lineConfig = gpiod_line_config_new();

        gpiod_line_config_add_line_settings(lineConfig, (const unsigned int *)&offset, 1, lineSettings);
        int status = gpiod_line_request_reconfigure_lines(lineRequest, lineConfig);
        int savedErrno = errno;

        gpiod_line_info_free(lineInfo);
        gpiod_line_settings_free(lineSettings);
        gpiod_line_config_free(lineConfig);
        if (status < 0) {
            napi_throw_error(env, NULL, strerror(savedErrno));
            return NULL;
        }
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
        #if UINTPTR_MAX == UINT64_MAX
        napi_create_bigint_uint64(env, (uintptr_t)lineRequest, &result);
        #else
        napi_create_uint32(env, (uintptr_t)lineRequest, &result);
        #endif

        gpiod_line_settings_free(lineSettings);
        gpiod_line_config_free(lineConfig);
        gpiod_request_config_free(requestConfig);
        return result;
}

napi_value setLineValue(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    int value;
    napi_get_value_int32(env, args[1], &value);

    uintptr_t linePtr;
    #if UINTPTR_MAX == UINT64_MAX
    bool lossless;
    napi_get_value_bigint_uint64(env, args[0], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[0], &linePtr);
    #endif
    
    napi_value result;

        struct gpiod_line_request* lineRequest;
        lineRequest = (struct gpiod_line_request*)linePtr;

        int numLines = gpiod_line_request_get_num_requested_lines(lineRequest);
        unsigned int *offsets = malloc(sizeof(int) * numLines);
        gpiod_line_request_get_requested_offsets(lineRequest, offsets, numLines);
        int status = gpiod_line_request_set_value(lineRequest, offsets[0], value);
        napi_create_int32(env, status, &result);
        free(offsets);
    
    return result;
}

napi_value getLineValue(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    uintptr_t linePtr;
    #if UINTPTR_MAX == UINT64_MAX
    bool lossless;
    napi_get_value_bigint_uint64(env, args[0], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[0], &linePtr);
    #endif

    napi_value result;

        struct gpiod_line_request* lineRequest;
        lineRequest = (struct gpiod_line_request*)linePtr;

        int numLines = gpiod_line_request_get_num_requested_lines(lineRequest);
        unsigned int *offsets = malloc(sizeof(int) * numLines);
        gpiod_line_request_get_requested_offsets(lineRequest, offsets, numLines);
        int status = gpiod_line_request_get_value(lineRequest, offsets[0]);
        napi_create_int32(env, status, &result);
        free(offsets);

    return result;
}

napi_value waitForEvent(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    uintptr_t linePtr;
    #if UINTPTR_MAX == UINT64_MAX
    bool lossless;
    napi_get_value_bigint_uint64(env, args[0], &linePtr, &lossless);
    #else
    napi_get_value_uint32(env, args[0], &linePtr);
    #endif

        int64_t timeout_ns = 1000000000; // 1 sec
        struct gpiod_line_request* lineRequest;
        lineRequest = (struct gpiod_line_request*)linePtr;
        int status = gpiod_line_request_wait_edge_events(lineRequest, timeout_ns);

        if(status > 0) {
            struct gpiod_edge_event_buffer* buffer = gpiod_edge_event_buffer_new(1);
            gpiod_line_request_read_edge_events(lineRequest, buffer, 1);
            gpiod_edge_event_buffer_free(buffer);
        }
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