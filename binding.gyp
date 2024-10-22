{
  "targets": [
    {
        "target_name": "ndi",
        "sources": [],
        "defines": [],
        "cflags": ["-std=c++11", "-Wall", "-pedantic", "-O3"],
        "include_dirs": [ 
          "<!(node -p \"require('node-addon-api').include_dir\")",
        ],
        "libraries": [],
        "dependencies": [],
        "conditions": [
            ['OS=="win"', {
              "sources": [ "node-ndi.cpp" ],
              'include_dirs': [
                './node_modules/native-graphics-deps/include',
                './ndi/include'
              ],
              'library_dirs': [
                './ndi/lib/win_x64'
              ],
              'libraries': [
                '-lProcessing.NDI.Lib.x64.lib',
              ],
              'defines' : [
                  'NAPI_DISABLE_CPP_EXCEPTIONS',
                'WIN32_LEAN_AND_MEAN',
                'VC_EXTRALEAN'
              ],
              "copies": [{
                'destination': './build/<(CONFIGURATION_NAME)/',
                'files': ['./ndi/lib/win_x64/Processing.NDI.Lib.x64.dll']
              }]
            }],
            ['OS=="mac"', {
              'cflags+': ['-fvisibility=hidden'],
              'xcode_settings': {},
            }],
            ['OS=="linux"', {}],
        ],
    }
  ]
}