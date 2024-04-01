#include <node_api.h>
#include <gpiod.h>
#include <assert.h>
#include <stdio.h>
#include <regex.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

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

    #ifdef GPIOD_API
    chipLabel = (char *) gpiod_chip_label(chip);

    #else
    struct gpiod_chip_info* chipInfo;

    chipInfo = gpiod_chip_get_info(chip);
    chipLabel = gpiod_chip_info_get_label(chipInfo);
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
#ifdef GPIOD_API
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
    }
}
#else
#endif

//configure active low
#ifdef GPIOD_API
void configureActiveLow(bool activeLow, struct gpiod_line_request_config* config) {
    if(activeLow) {
        config->flags = GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW;
    } else {
        config->flags = 0;
    }
}
#else
#endif

//configure edge
#ifdef GPIOD_API
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
#else
#endif

//configure line
napi_value configureLine(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    #ifdef GPIOD_API
    struct gpiod_line* line;
    #else
    struct gpiod_line_settings* lineSettings;
    #endif
    napi_get_value_external(env, args[0], (void**)&line);

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

    #ifdef GPIOD_API
    struct gpiod_line_request_config config;
    int defaultValue = 0;
    config.consumer = "onoff";
    configureActiveLow(activeLow, &config);
    if((strncmp("none", edge, strlen("none")) != 0) && (strncmp("in", direction, strlen("in")) == 0)) {
        configureEdge(edge, &config);
    } else {
        configureDirection(direction, &config, &defaultValue);
    }
    gpiod_line_request(line, &config, defaultValue);
    #else
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

    #ifdef GPIOD_API
        struct gpiod_line *line;
        line = gpiod_chip_get_line(chip, offset);

        napi_value result;
        napi_create_external(env, line, NULL, NULL, &result);
        return result;
    #else
        struct gpiod_line_settings* lineSettings;
        lineSettings = gpiod_line_settings_new();
        gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_AS_IS);
        struct gpiod_line_config* lineConfig = gpiod_line_config_new();
        int test = gpiod_line_config_add_line_settings(lineConfig, &offset, 1, lineSettings);
        struct gpiod_line_request* lineRequest;
        lineRequest = gpiod_chip_request_lines(chip, requestConfig, lineConfig);

        napi_value result;
        napi_create_external(env, lineRequest, NULL, NULL, &result);
        return result;
    #endif
}

napi_value lineSetValue(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_line *line;
    int value;
    napi_get_value_external(env, args[0], (void**)&line);
    napi_get_value_int32(env, args[1], &value);
    int status = gpiod_line_set_value (line, value);

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

napi_value lineGetValue(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_line *line;
    napi_get_value_external(env, args[0], (void**)&line);
    gpiod_line_active_state(line);
    int status = gpiod_line_get_value (line);

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
    DECLARE_NAPI_METHOD("lineSetValue", lineSetValue),
    DECLARE_NAPI_METHOD("lineGetValue", lineGetValue)

  };
  status = napi_define_properties(env, exports, sizeof(descriptor)/sizeof(descriptor[0]), descriptor);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)