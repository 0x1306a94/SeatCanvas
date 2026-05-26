import * as esbuild from 'esbuild';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const webDir = path.resolve(__dirname, '..');
const tgfxSrcDir = path.resolve(webDir, '../third_party/tgfx/web/src');

/** src/wasm 供解析；打包后 import 指向 demo 下的 ./wasm/libseatcanvas.js */
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

await esbuild.build({
  entryPoints: ['demo/index.ts'],
  bundle: true,
  outfile: 'demo/index.js',
  format: 'esm',
  platform: 'browser',
  target: 'es2020',
  external: ['./wasm/libseatcanvas.js'],
  plugins: [wasmGluePlugin, tgfxAliasPlugin],
});

console.log('Demo built successfully.');
