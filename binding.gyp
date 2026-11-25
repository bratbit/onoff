{
  "targets": [
    {
      "target_name": "gpiod-wrap",
      "sources": [ "src/libgpiod_wrap.c" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "libraries": ["-lgpiod"],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}