#!/usr/bin/env node
require('./setup.emsdk');

const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const isDebug = process.argv.includes('--debug');
const buildType = isDebug ? 'Debug' : 'Release';
const webDir = path.resolve(__dirname, '..');
const srcDir = path.resolve(webDir, '..');
const buildDir = path.join(webDir, isDebug ? 'build_debug' : 'build');

console.log(`Building SeatCanvas Web (${buildType})...`);

// Configure with emcmake — root CMakeLists.txt is the entry point
const configureCmd = `emcmake cmake -B "${buildDir}" -G Ninja -DCMAKE_BUILD_TYPE=${buildType} -DCMAKE_POLICY_VERSION_MINIMUM=3.5 "${srcDir}"`;
console.log(`\n> ${configureCmd}`);
execSync(configureCmd, { stdio: 'inherit', cwd: webDir, env: process.env });

// Build
const buildCmd = `cmake --build "${buildDir}" --config ${buildType}`;
console.log(`\n> ${buildCmd}`);
execSync(buildCmd, { stdio: 'inherit', cwd: webDir, env: process.env });

const outputDir = path.join(buildDir, 'src', 'SeatCanvas');
const wasmFile = path.join(outputDir, 'libseatcanvas.wasm');
const jsFile = path.join(outputDir, 'libseatcanvas.js');

if (!fs.existsSync(wasmFile) || !fs.existsSync(jsFile)) {
    console.error(`WASM build output not found in ${outputDir}. Check the CMake build above.`);
    process.exit(1);
}

function copyToDir(targetDir) {
    if (fs.existsSync(targetDir)) {
        fs.rmSync(targetDir, {recursive: true});
    }
    fs.mkdirSync(targetDir, {recursive: true});
    fs.copyFileSync(jsFile, path.join(targetDir, 'libseatcanvas.js'));
    fs.copyFileSync(wasmFile, path.join(targetDir, 'libseatcanvas.wasm'));
}

// Release → lib / src / demo; Debug (-O0 -g3, ~60MB+) → src / demo only (skip lib)
copyToDir(path.join(webDir, 'demo', 'wasm'));
copyToDir(path.join(webDir, 'src', 'wasm'));
if (!isDebug) {
    copyToDir(path.join(webDir, 'lib', 'wasm'));
} else {
    console.log('Debug WASM copied to demo/wasm and src/wasm (lib/wasm unchanged).');
}

console.log('\nBuild complete. Run "node server.js" in web/ to start the demo.');
