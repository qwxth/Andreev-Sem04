import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

export class Car3DComponent {
    constructor(container, modelPath) {
        this.container = container;
        this.modelPath = modelPath;
        this.scene = null;
        this.camera = null;
        this.renderer = null;
        this.controls = null;
        this.model = null;
    }

    init() {
        const width = this.container.clientWidth;
        const height = this.container.clientHeight || 400;

        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0xf8f9fa);

        this.camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 1000);
        this.camera.position.set(2, 1.5, 4);
        this.camera.lookAt(0, 0, 0);

        this.renderer = new THREE.WebGLRenderer({ antialias: true });
        this.renderer.setSize(width, height);
        this.renderer.shadowMap.enabled = true;
        this.container.appendChild(this.renderer.domElement);

        this.controls = new OrbitControls(this.camera, this.renderer.domElement);
        this.controls.enableDamping = true;
        this.controls.dampingFactor = 0.05;
        this.controls.autoRotate = false;
        this.controls.enableZoom = true;
        this.controls.target.set(0, 0.5, 0);

        const ambient = new THREE.AmbientLight(0xffffff, 0.6);
        this.scene.add(ambient);
        const dirLight = new THREE.DirectionalLight(0xffffff, 1);
        dirLight.position.set(2, 5, 3);
        dirLight.castShadow = true;
        this.scene.add(dirLight);
        const fillLight = new THREE.PointLight(0xccaa88, 0.3);
        fillLight.position.set(0, -1, 0);
        this.scene.add(fillLight);
        const backLight = new THREE.PointLight(0x88aaff, 0.4);
        backLight.position.set(0, 1, -3);
        this.scene.add(backLight);

        const grid = new THREE.GridHelper(8, 20, 0xaaaaaa, 0xdddddd);
        grid.position.y = -0.8;
        this.scene.add(grid);

        this.showLoading();
        this.loadModel();
        this.animate();
    }

    showLoading() {
        this.loadingDiv = document.createElement('div');
        this.loadingDiv.textContent = 'Загрузка 3D модели...';
        this.loadingDiv.style.position = 'absolute';
        this.loadingDiv.style.top = '50%';
        this.loadingDiv.style.left = '50%';
        this.loadingDiv.style.transform = 'translate(-50%, -50%)';
        this.loadingDiv.style.background = 'rgba(0,0,0,0.7)';
        this.loadingDiv.style.color = 'white';
        this.loadingDiv.style.padding = '10px 20px';
        this.loadingDiv.style.borderRadius = '30px';
        this.loadingDiv.style.fontSize = '14px';
        this.loadingDiv.style.zIndex = '100';
        this.container.style.position = 'relative';
        this.container.appendChild(this.loadingDiv);
    }

    hideLoading() {
        if (this.loadingDiv) this.loadingDiv.remove();
    }

    loadModel() {
        const loader = new GLTFLoader();
        loader.load(this.modelPath, (gltf) => {
            this.hideLoading();
            this.model = gltf.scene;

            const box = new THREE.Box3().setFromObject(this.model);
            const center = box.getCenter(new THREE.Vector3());
            const size = box.getSize(new THREE.Vector3());
            const scale = 1.2 / Math.max(size.x, size.y, size.z);
            this.model.scale.set(scale, scale, scale);
            this.model.position.set(-center.x * scale, -center.y * scale + 0.2, -center.z * scale);

            this.model.traverse(node => {
                if (node.isMesh) {
                    node.castShadow = true;
                    node.receiveShadow = true;
                }
            });

            this.scene.add(this.model);
            this.controls.target.set(0, 0.5, 0);
            this.controls.update();
        }, undefined, (error) => {
            this.hideLoading();
            console.error('Ошибка загрузки модели:', error);
            if (this.loadingDiv) {
                this.loadingDiv.textContent = 'Ошибка загрузки модели';
                this.loadingDiv.style.background = 'red';
            }
        });
    }

    animate() {
        requestAnimationFrame(this.animate.bind(this));
        if (this.controls) this.controls.update();
        if (this.renderer && this.scene && this.camera) {
            this.renderer.render(this.scene, this.camera);
        }
    }

    onResize() {
        const width = this.container.clientWidth;
        const height = this.container.clientHeight || 400;
        this.camera.aspect = width / height;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(width, height);
    }

    render() {
        if (!this.renderer) this.init();
        window.addEventListener('resize', () => this.onResize());
    }
}
