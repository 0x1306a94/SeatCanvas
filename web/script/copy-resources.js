#!/usr/bin/env node
const fs = require('fs');
const path = require('path');

const srcDir = path.resolve(__dirname, '..', '..', 'resources', 'SeatCanvasSample.bundle');
const dstDir = path.resolve(__dirname, '..', 'demo', 'sample_resources');

if (!fs.existsSync(srcDir)) {
    console.error(`Error: Source directory not found: ${srcDir}`);
    process.exit(1);
}

// Clean and recreate destination
if (fs.existsSync(dstDir)) {
    fs.rmSync(dstDir, { recursive: true, force: true });
}

fs.cpSync(srcDir, dstDir, { recursive: true });
console.log(`Copied resources: ${srcDir} -> ${dstDir}`);

const fontSrcDir = path.resolve(__dirname, '..', '..', 'resources', 'fonts');
const fontDstDir = path.join(path.resolve(__dirname, '..', 'demo'), 'fonts');
const minFontBytes = 1_000_000;
const requiredFonts = ['NotoSansSC-Regular.otf', 'NotoColorEmoji.ttf'];

if (!fs.existsSync(fontSrcDir)) {
    console.error(`Error: Fonts directory not found: ${fontSrcDir}`);
    console.error('Place NotoSansSC-Regular.otf and NotoColorEmoji.ttf under resources/fonts/ (see resources/fonts/LICENSE).');
    process.exit(1);
}

for (const fontName of requiredFonts) {
    const fontPath = path.join(fontSrcDir, fontName);
    if (!fs.existsSync(fontPath)) {
        console.error(`Error: Missing font file: ${fontPath}`);
        process.exit(1);
    }
    const fontSize = fs.statSync(fontPath).size;
    if (fontSize <= minFontBytes) {
        console.error(`Error: Font file too small (${fontSize} bytes): ${fontPath}`);
        console.error('Git LFS files may not be pulled. Run: git lfs pull');
        process.exit(1);
    }
}

if (fs.existsSync(fontDstDir)) {
    fs.rmSync(fontDstDir, { recursive: true, force: true });
}
fs.cpSync(fontSrcDir, fontDstDir, { recursive: true });
console.log(`Copied fonts: ${fontSrcDir} -> ${fontDstDir}`);

// Generate static basemap list (replaces /api/basemaps for static hosting)
const basemapDir = path.join(dstDir, 'default', 'basemap');
if (fs.existsSync(basemapDir)) {
    const basemaps = fs.readdirSync(basemapDir)
        .filter(f => f.endsWith('.svg'))
        .sort();
    fs.writeFileSync(path.join(dstDir, 'basemaps.json'), JSON.stringify(basemaps));
    console.log(`Generated basemaps.json with ${basemaps.length} entries`);
}
