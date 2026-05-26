const path = require('path');
const childProcess = require('child_process');
const fs = require('fs');

const emsdkPath = path.resolve(__dirname, '../../third_party/emsdk');
const emscriptenPath = path.resolve(emsdkPath, 'upstream/emscripten');

if (!fs.existsSync(emsdkPath)) {
    console.error('emsdk not found at third_party/emsdk. Run ./sync_deps.sh first.');
    process.exit(1);
}

process.env.PATH = process.platform === 'win32'
    ? `${emsdkPath};${emscriptenPath};${process.env.PATH}`
    : `${emsdkPath}:${emscriptenPath}:${process.env.PATH}`;

process.env.EMSDK_QUIET = '1';

function exec(cmd, dir) {
    const options = {
        shell: process.platform === 'win32' ? 'cmd.exe' : true,
        cwd: path.resolve(dir),
        env: process.env,
        stdio: 'inherit',
    };
    console.log(cmd);
    const result = childProcess.spawnSync(cmd, options);
    if (result.status !== 0) {
        process.exit(result.status ?? 1);
    }
}

function execSafe(cmd, dir) {
    const options = {
        shell: process.platform === 'win32' ? 'cmd.exe' : true,
        stdio: 'pipe',
        encoding: 'utf-8',
        cwd: path.resolve(dir),
        env: process.env,
    };
    try {
        const result = childProcess.spawnSync(cmd, options);
        let text = '';
        if (result.stdout) {
            text += result.stdout.toString();
        }
        if (result.stderr) {
            text += result.stderr.toString();
        }
        return text;
    } catch (e) {
        return '';
    }
}

function applyConstructEnv() {
    if (process.platform === 'win32') {
        return;
    }
    const result = execSafe('EMSDK_QUIET=1 EMSDK_BASH=1 ./emsdk construct_env', emsdkPath);
    for (const line of result.split('\n')) {
        const match = line.match(/^export (\w+)="((?:\\.|[^"])*)";?\s*$/);
        if (match) {
            process.env[match[1]] = match[2].replace(/\\"/g, '"');
        }
    }
}

const emccName = process.platform === 'win32' ? 'emcc.bat' : 'emcc';
const emccPath = path.join(emscriptenPath, emccName);
if (!fs.existsSync(emccPath)) {
    exec('emsdk install latest', emsdkPath);
}
exec('emsdk activate latest', emsdkPath);
applyConstructEnv();
