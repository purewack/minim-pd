{
  "targets": [
    {
      "target_name": "minim",
      "sources": [ 
          '<!@(ls -1 src/*.cpp)',
          '<!@(ls -1 ../../api/src/*.cpp)',
          '<!@(ls -1 ../../api/gfx/*.cpp)'
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