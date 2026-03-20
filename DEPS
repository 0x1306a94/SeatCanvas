{
  "version": "1.4.5",
  "vars": {
    "GITHUB_BASE_URL": "https://github.com"
  },
  "repos": {
    "common": [
      {
        "url": "${GITHUB_BASE_URL}/libpag/tgfx.git",
        "commit": "50ce1247c885c962365351614c3df0804ac8392e",
        "dir": "third_party/tgfx"
      }
    ],
    "mac": [
      {
        "url": "${GITHUB_BASE_URL}/QMUI/LookinServer.git",
        "commit": "ae396132ce6494477457cf450116dc760518d8ad",
        "dir": "ios/third_party/LookinServer"
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