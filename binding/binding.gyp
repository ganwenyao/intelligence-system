{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "group37_linux.cc"
      ],
      "include_dirs": [
        "<!@(pkg-config --cflags opencv)",
        "<!@(pkg-config --cflags mraa)",
        "/usr/include"
      ],
      "cflags": [
        "-std=c++11"
      ],
      "cflags!" : [ 
        "-fno-exceptions"
      ],
      "cflags_cc!": [ 
        "-fno-rtti",
        "-fno-exceptions"
      ],
      "xcode_settings": {
        "OTHER_CFLAGS": [
          "-std=c++11"
        ]
      },
      "libraries": [
        '<!@(pkg-config --libs opencv)',
        '<!@(pkg-config --libs mraa)'
      ]
    }
  ]
}
