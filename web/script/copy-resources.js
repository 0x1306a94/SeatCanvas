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

// Generate static basemap list (replaces /api/basemaps for static hosting)
const basemapDir = path.join(dstDir, 'default', 'basemap');
if (fs.existsSync(basemapDir)) {
    const basemaps = fs.readdirSync(basemapDir)
        .filter(f => f.endsWith('.svg'))
        .sort();
    fs.writeFileSync(path.join(dstDir, 'basemaps.json'), JSON.stringify(basemaps));
    console.log(`Generated basemaps.json with ${basemaps.length} entries`);
}
