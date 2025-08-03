{
  "targets": [
    {
      "target_name": "mmap-io",
      "sources": ["src/mmap-io.cc"],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": ["NAPI_CPP_EXCEPTIONS"],
      "cflags_cc": ["-std=c++20", "-fexceptions"],
      "conditions": [
        [
          "OS==\"mac\"",
          {
            "xcode_settings": {
              "OTHER_CPLUSPLUSFLAGS": ["-std=c++20", "-stdlib=libc++", "-fexceptions"],
              "OTHER_LDFLAGS": ["-stdlib=libc++"],
              "MACOSX_DEPLOYMENT_TARGET": "10.8"
            }
          }
        ]
      ]
    }
  ]
}
