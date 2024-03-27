#include <node_api.h>
#include <gpiod.h>
#include <assert.h>
#include <stdio.h>

napi_value openChip(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    char path[4096];
    size_t str_length;
    napi_get_value_string_utf8(env, args[0], path, sizeof(path), &str_length);
    struct gpiod_chip* chip;
    chip = gpiod_chip_open(path);

    napi_value result;
    napi_create_external(env, chip, NULL, NULL, &result);
    
    return result;
}

napi_value getChipLabel(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_chip* chip;
    napi_get_value_external(env, args[0], (struct gpiod_chip**)&chip);

    char* label = gpiod_chip_label(chip);
    napi_value result;
    napi_create_string_utf8(env, label, NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value getNumLines(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_chip* chip;
    napi_get_value_external(env, args[0], (struct gpiod_chip**)&chip);

    napi_value result;
    napi_create_int32(env, gpiod_chip_num_lines(chip), &result);
    return result;
}

napi_value chipGetLine(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_chip* chip;
    int offset;
    napi_get_value_external(env, args[0], (struct gpiod_chip**)&chip);
    napi_get_value_int32(env, args[1], &offset);

    struct gpiod_line *line;
    line = gpiod_chip_get_line (chip, offset);

    napi_value result;
    napi_create_external(env, line, NULL, NULL, &result);
    return result;
}

napi_value lineRequestOutput(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_line *line;
    int default_val;
    napi_get_value_external(env, args[0], (struct gpiod_line**)&line);
    napi_get_value_int32(env, args[1], &default_val);
    int status = gpiod_line_request_output (line, "onoff", default_val);

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

napi_value lineRequestInput(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_line *line;
    napi_get_value_external(env, args[0], (struct gpiod_line**)&line);
    int status = gpiod_line_request_input (line, "onoff");

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

napi_value lineSetValue(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_line *line;
    int value;
    napi_get_value_external(env, args[0], (struct gpiod_line**)&line);
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
    napi_get_value_external(env, args[0], (struct gpiod_line**)&line);
    int as = gpiod_line_active_state(line);
    int status = gpiod_line_get_value (line);

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

napi_value lineActiveState(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_line *line;
    napi_get_value_external(env, args[0], (struct gpiod_line**)&line);
    int activeState = gpiod_line_active_state(line);

    napi_value result;
    napi_create_int32(env, activeState, &result);
    return result;
}

napi_value lineSetFlags(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct gpiod_line *line;
    int flags;
    napi_get_value_external(env, args[0], (struct gpiod_line**)&line);
    napi_get_value_int32(env, args[1], &flags);
    int status = gpiod_line_set_flags(line, GPIOD_BIT(flags));

    napi_value result;
    napi_create_int32(env, status, &result);
    return result;
}

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor descriptor[] = {
    DECLARE_NAPI_METHOD("openChip", openChip),
    DECLARE_NAPI_METHOD("getChipLabel", getChipLabel),
    DECLARE_NAPI_METHOD("getNumLines", getNumLines),
    DECLARE_NAPI_METHOD("chipGetLine", chipGetLine),
    DECLARE_NAPI_METHOD("lineRequestOutput", lineRequestOutput),
    DECLARE_NAPI_METHOD("lineRequestInput", lineRequestInput),
    DECLARE_NAPI_METHOD("lineSetValue", lineSetValue),
    DECLARE_NAPI_METHOD("lineGetValue", lineGetValue),
    DECLARE_NAPI_METHOD("lineActiveState", lineActiveState),
    DECLARE_NAPI_METHOD("lineSetFlags", lineSetFlags)

  };
  status = napi_define_properties(env, exports, sizeof(descriptor)/sizeof(descriptor[0]), &descriptor);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)