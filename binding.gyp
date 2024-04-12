{
  "variables": {
    "gpiod_major": "<!(pkg-config --modversion libgpiod | awk -F'.' '{print $1}')"
},
  "targets": [
    {
      "target_name": "gpiod-wrap",
      "sources": [ "src/libgpiod_wrap.c" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "libraries": ["-lgpiod"],
      "cflags": ["-DGPIOD_MAJOR=<(gpiod_major)"],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}