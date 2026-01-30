{
  "targets": [
    {
      "target_name": "dmx_native",
      "sources": [ "dmx.cc" ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")'
      ],
      'cflags': [
        '<!@(pkg-config --cflags libftdi1 || pkg-config --cflags libftdi || echo "")'
      ],
      'ldflags': [
        '<!@(pkg-config --libs libftdi1 || pkg-config --libs libftdi || echo "-lftdi")'
      ],
      "libraries": [ "-lpthread" ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "dmx_native" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/dmx_native.node" ],
          "destination": "."
        }
      ]
    }
  ]
}
