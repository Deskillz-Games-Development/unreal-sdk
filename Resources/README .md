# Resources Folder

This folder contains plugin resources such as icons, images, and other assets used by the Deskillz SDK.

## What Goes Here

| File | Size | Description |
|------|------|-------------|
| `Icon128.png` | 128x128 | Plugin icon shown in Unreal Editor Plugin Browser |
| `Icon64.png` | 64x64 | Small plugin icon |
| `Icon16.png` | 16x16 | Tiny icon for toolbars |
| `ButtonIcon_40x.png` | 40x40 | Editor toolbar button icon |

## Required: Plugin Icon

Every Unreal Engine plugin should have an icon. Create `Icon128.png`:

### Specifications:
- **Size:** 128 x 128 pixels
- **Format:** PNG with transparency
- **Design:** Deskillz logo or game controller icon

### How to Create:

1. Create a 128x128 pixel image
2. Use the Deskillz brand colors:
   - Primary Blue: `#3399FF`
   - Dark Background: `#1A1A24`
   - Gold Accent: `#FFD700`
3. Include a simple recognizable icon (game controller, trophy, etc.)
4. Save as `Icon128.png` in this folder

### Example Icon Design:
```
┌────────────────────────┐
│     ████████████       │
│   ██            ██     │
│  █   ┌──────┐    █    │
│  █   │ D X  │    █    │  ← "DX" for Deskillz
│  █   └──────┘    █    │
│   ██            ██     │
│     ████████████       │
│                        │
│    D E S K I L L Z     │
└────────────────────────┘
```

## Optional: Additional Resources

### Editor Toolbar Icons
If you add custom editor tools, include toolbar icons:
- `ButtonIcon_40x.png` - 40x40 pixels for editor buttons

### Splash/Loading Images
For loading screens:
- `Splash_1920x1080.png` - Full HD splash screen
- `Loading_256x256.png` - Loading spinner/animation

### Currency Icons (Optional)
If you want custom cryptocurrency icons:
- `Icon_BTC.png` - Bitcoin (24x24)
- `Icon_ETH.png` - Ethereum (24x24)
- `Icon_SOL.png` - Solana (24x24)
- `Icon_USDT.png` - Tether (24x24)
- `Icon_USDC.png` - USD Coin (24x24)

## Plugin Descriptor Reference

The `Deskillz.uplugin` file references these resources:

```json
{
  "FileVersion": 3,
  "Version": 1,
  "VersionName": "1.0.0",
  "FriendlyName": "Deskillz SDK",
  "Description": "Competitive gaming with cryptocurrency prizes",
  "Category": "Gaming",
  "CreatedBy": "Deskillz Games",
  "CreatedByURL": "https://deskillz.games",
  "DocsURL": "https://docs.deskillz.games",
  "MarketplaceURL": "",
  "SupportURL": "https://support.deskillz.games",
  "CanContainContent": true,
  "IsBetaVersion": false,
  "IsExperimentalVersion": false,
  "Installed": false,
  "Modules": [
    {
      "Name": "Deskillz",
      "Type": "Runtime",
      "LoadingPhase": "Default"
    },
    {
      "Name": "DeskillzEditor",
      "Type": "Editor",
      "LoadingPhase": "Default"
    }
  ]
}
```

## Notes

- All images should be PNG format with transparency where appropriate
- Keep file sizes small for fast loading
- Use consistent visual style across all icons
- Test icons at different DPI settings
