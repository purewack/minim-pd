{
  "targets": [
    {
      "target_name": "minim",
      "sources": [ 
          "src/wrapBufferPainter.cpp",
          "src/wrapDisplayList.cpp",
          "src/wrapControlSurface.cpp",
          "src/wrapInit.cpp",

          "../../api/BufferPainter.cpp",
          "../../api/DisplayList.cpp",
          "../../api/ControlSurfaceAPI5.cpp",
        ],
      'include_dirs': [
          "<!@(node -p \"require('node-addon-api').include\")",
          "src",
          "../../api"
        ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }
  ]
}