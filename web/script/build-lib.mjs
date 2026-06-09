import * as esbuild from 'esbuild';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const webDir = path.resolve(__dirname, '..');
const tgfxSrcDir = path.resolve(webDir, '../third_party/tgfx/web/src');

const wasmGluePlugin = {
  name: 'wasm-glue',
  setup(build) {
    build.onResolve({ filter: /\.\/wasm\/libseatcanvas$/ }, () => {
      const wasmJs = path.join(webDir, 'src/wasm/libseatcanvas.js');
      if (!fs.existsSync(wasmJs)) {
        return null;
      }
      return { path: './wasm/libseatcanvas.js', external: true };
    });
  },
};

const tgfxAliasPlugin = {
  name: 'tgfx-alias',
  setup(build) {
    build.onResolve({ filter: /^@tgfx\/.*$/ }, (args) => {
      const subPath = args.path.replace('@tgfx/', '');
      return { path: path.join(tgfxSrcDir, subPath) + '.ts' };
    });
  },
};

const external = ['./wasm/libseatcanvas.js'];

const baseConfig = {
  entryPoints: ['src/SeatCanvas.ts'],
  bundle: true,
  platform: 'browser',
  target: 'es2020',
  external,
  plugins: [wasmGluePlugin, tgfxAliasPlugin],
};

// Clean output
const libDir = path.join(webDir, 'lib');
if (fs.existsSync(libDir)) {
  for (const file of fs.readdirSync(libDir)) {
    if (file.endsWith('.js') || file.endsWith('.map')) {
      fs.unlinkSync(path.join(libDir, file));
    }
  }
}

// ESM
await esbuild.build({
  ...baseConfig,
  format: 'esm',
  outfile: 'lib/libseatcanvas.esm.js',
  sourcemap: true,
});

// CJS
await esbuild.build({
  ...baseConfig,
  format: 'cjs',
  outfile: 'lib/libseatcanvas.cjs.js',
  sourcemap: true,
});

// Generate type declarations
// types are handled by tsc via tsconfig.type.json

console.log('Library build complete.');
