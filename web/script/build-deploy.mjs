import * as esbuild from 'esbuild';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const webDir = path.resolve(__dirname, '..');
const tgfxSrcDir = path.resolve(webDir, '../third_party/tgfx/web/src');
const deployDir = path.join(webDir, 'deploy');

const tgfxAliasPlugin = {
  name: 'tgfx-alias',
  setup(build) {
    build.onResolve({ filter: /^@tgfx\/.*$/ }, (args) => {
      const subPath = args.path.replace('@tgfx/', '');
      return { path: path.join(tgfxSrcDir, subPath) + '.ts' };
    });
  },
};

// Clean deploy dir
if (fs.existsSync(deployDir)) {
  fs.rmSync(deployDir, { recursive: true });
}

// Bundle demo JS
await esbuild.build({
  entryPoints: ['demo/index.ts'],
  bundle: true,
  outfile: 'deploy/index.js',
  format: 'esm',
  platform: 'browser',
  target: 'es2020',
  external: ['./wasm/libseatcanvas.js'],
  plugins: [tgfxAliasPlugin],
});

// Copy static files
const filesToCopy = ['index.html', 'index.css'];
for (const file of filesToCopy) {
  fs.copyFileSync(path.join(webDir, 'demo', file), path.join(deployDir, file));
}

// Copy wasm glue (only current Emscripten output names)
const wasmArtifacts = ['libseatcanvas.js', 'libseatcanvas.wasm'];
const wasmDir = path.join(deployDir, 'wasm');
if (fs.existsSync(wasmDir)) {
  fs.rmSync(wasmDir, { recursive: true });
}
fs.mkdirSync(wasmDir, { recursive: true });
const demoWasmDir = path.join(webDir, 'demo', 'wasm');
for (const file of wasmArtifacts) {
  const source = path.join(demoWasmDir, file);
  if (!fs.existsSync(source)) {
    throw new Error(`Missing ${source}. Run npm run build:wasm first.`);
  }
  fs.copyFileSync(source, path.join(wasmDir, file));
}

// Copy sample resources
const srcResources = path.join(webDir, 'demo', 'sample_resources');
const dstResources = path.join(deployDir, 'sample_resources');
fs.cpSync(srcResources, dstResources, { recursive: true });

// Copy fonts for FreeType fallback registration
const srcFonts = path.resolve(webDir, '..', 'resources', 'fonts');
const dstFonts = path.join(deployDir, 'fonts');
const minFontBytes = 1_000_000;
const requiredFonts = ['NotoSansSC-Regular.otf', 'NotoColorEmoji.ttf'];

if (!fs.existsSync(srcFonts)) {
  throw new Error(
    `Missing fonts directory: ${srcFonts}. ` +
    'Place NotoSansSC-Regular.otf and NotoColorEmoji.ttf under resources/fonts/ (see resources/fonts/LICENSE).',
  );
}

for (const fontName of requiredFonts) {
  const fontPath = path.join(srcFonts, fontName);
  if (!fs.existsSync(fontPath)) {
    throw new Error(`Missing font file: ${fontPath}`);
  }
  const fontSize = fs.statSync(fontPath).size;
  if (fontSize <= minFontBytes) {
    throw new Error(
      `Font file too small (${fontSize} bytes): ${fontPath}. ` +
      'Git LFS files may not be pulled. Run: git lfs pull',
    );
  }
}

fs.cpSync(srcFonts, dstFonts, { recursive: true });

// Generate static basemap list (replaces /api/basemaps)
const basemapDir = path.join(srcResources, 'default', 'basemap');
const basemaps = fs.readdirSync(basemapDir)
  .filter(f => f.endsWith('.svg'))
  .sort();
fs.writeFileSync(path.join(dstResources, 'basemaps.json'), JSON.stringify(basemaps));

// Cloudflare Pages headers (COOP/COEP required for SharedArrayBuffer)
const headers = `/*
  Cross-Origin-Opener-Policy: same-origin
  Cross-Origin-Embedder-Policy: require-corp
`;
fs.writeFileSync(path.join(deployDir, '_headers'), headers);

console.log(`Deploy build complete → ${deployDir}`);
