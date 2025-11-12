{
  "targets": [
    {
      "target_name": "monnylang_core",
      "sources": [
        "src/bindings.cpp",
        "src/Monny.cpp"
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")",
        "include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],
      "cflags_cc!": [],
      "cflags_cc": ["-std=c++17"]
    }
  ]
}
