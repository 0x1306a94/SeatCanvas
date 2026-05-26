const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 8081;

// Enable SharedArrayBuffer
app.use((req, res, next) => {
    res.set('Cross-Origin-Opener-Policy', 'same-origin');
    res.set('Cross-Origin-Embedder-Policy', 'require-corp');
    next();
});

const resourcesDir = path.join(__dirname, 'demo', 'sample_resources');
app.use('/sample_resources', express.static(resourcesDir));
app.use('/wasm', express.static(path.join(__dirname, 'demo', 'wasm')));
app.use(express.static(path.join(__dirname, 'demo')));

// API: list basemap SVG files
app.get('/api/basemaps', (req, res) => {
    const basemapDir = path.join(resourcesDir, 'default', 'basemap');
    try {
        const files = fs.readdirSync(basemapDir)
            .filter(f => f.endsWith('.svg'))
            .sort();
        res.json(files);
    } catch (e) {
        res.json([]);
    }
});

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'demo', 'index.html'));
});

app.listen(PORT, () => {
    console.log(`SeatCanvas Web server running at http://localhost:${PORT}`);
});
