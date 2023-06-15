{
  "targets": [
    {
      "target_name": "minim",
      "sources": [ 
          "src/wrapBufferPainter.cpp",
          "src/wrapDisplayList.cpp",
          "src/wrapControlSurface.cpp",
          "src/wrapInit.cpp",

          "../../api/src/BufferPainter.cpp",
          "../../api/src/DisplayList.cpp",
          "../../api/src/ControlSurfaceAPI5.cpp",
          "../../api/src/utilCommandFactory.cpp",
        ],
      'include_dirs': [
          "<!@(node -p \"require('node-addon-api').include\")",
          "src",
          "../../api/include",
          "../../api/gfx"
        ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }
  ]
}