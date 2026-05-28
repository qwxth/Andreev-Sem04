import { defineConfig } from 'vite';

export default defineConfig({
    build: {
        outDir: './dist',
        emptyOutDir: true,
        rollupOptions: {
            input: 'index.html',
            // Указываем внешние зависимости – они будут подключены через CDN или не бандлить
            external: [],
        },
    },
    server: {
        port: 5173,
    },
    // Указываем, что нужно обрабатывать эти модули как обычные (не external)
    optimizeDeps: {
        include: ['three', 'three/examples/jsm/loaders/GLTFLoader.js', 'three/examples/jsm/controls/OrbitControls.js'],
    },
});
