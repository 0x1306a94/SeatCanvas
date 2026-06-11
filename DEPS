{
  "version": "1.4.5",
  "vars": {
    "GITHUB_BASE_URL": "https://github.com"
  },
  "repos": {
    "common": [
      {
        "url": "${GITHUB_BASE_URL}/libpag/tgfx.git",
        "commit": "64c8597101809078bc71499c13fe850553fbfa1e",
        "dir": "third_party/tgfx"
      },
      {
        "url": "${GITHUB_BASE_URL}/emscripten-core/emsdk.git",
        "commit": "ba0585d60fb9cfd6f5088abb10a637ef34bcee9e",
        "dir": "third_party/emsdk"
      },
      {
        "url": "${GITHUB_BASE_URL}/google/googletest.git",
        "commit": "6910c9d9165801d8827d628cb72eb7ea9dd538c5",
        "dir": "third_party/googletest"
      },
      {
        "url": "${GITHUB_BASE_URL}/QMUI/LookinServer.git",
        "commit": "ae396132ce6494477457cf450116dc760518d8ad",
        "dir": "third_party/LookinServer"
      }
    ]
  },
  "actions": {
    "common": [
      {
        "command": "depctl --clean",
        "dir": "third_party"
      },
      {
        "command": "python tgfx/third_party/shaderc/utils/git-sync-deps",
        "dir": "third_party"
      }
    ]
  }
}