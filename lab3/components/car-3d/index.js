import * as THREE from 'three';

export class Car3DComponent {
    constructor(container, color = 0xE31E24) {
        this.container = container;
        this.color = color;
        this.scene = null;
        this.camera = null;
        this.renderer = null;
        this.car = null;
    }

    init() {
        const width = this.container.clientWidth;
        const height = 400;
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0xf8f9fa);
        this.camera = new THREE.PerspectiveCamera(45, width/height, 0.1, 1000);
        this.camera.position.set(3, 2, 5);
        this.camera.lookAt(0, 0, 0);
        this.renderer = new THREE.WebGLRenderer({ antialias: true });
        this.renderer.setSize(width, height);
        this.renderer.shadowMap.enabled = true;
        this.container.appendChild(this.renderer.domElement);

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

        this.createCarModel();
        this.animate();
    }

    createCarModel() {
        const carGroup = new THREE.Group();
        const bodyMat = new THREE.MeshStandardMaterial({ color: this.color, roughness: 0.3, metalness: 0.7 });
        const body = new THREE.Mesh(new THREE.BoxGeometry(1.2, 0.5, 2.2), bodyMat);
        body.castShadow = true;
        body.position.y = 0;
        carGroup.add(body);
        const hoodMat = new THREE.MeshStandardMaterial({ color: this.color, roughness: 0.4 });
        const hood = new THREE.Mesh(new THREE.BoxGeometry(0.9, 0.2, 0.7), hoodMat);
        hood.position.set(0, 0.2, 0.9);
        hood.castShadow = true;
        carGroup.add(hood);
        const roofMat = new THREE.MeshStandardMaterial({ color: 0x222222, roughness: 0.2, metalness: 0.9 });
        const roof = new THREE.Mesh(new THREE.BoxGeometry(0.9, 0.4, 1.2), roofMat);
        roof.position.y = 0.45;
        roof.castShadow = true;
        carGroup.add(roof);
        const wheelGeo = new THREE.CylinderGeometry(0.35, 0.35, 0.4, 24);
        const wheelMat = new THREE.MeshStandardMaterial({ color: 0x111111, roughness: 0.5 });
        const positions = [[-0.7,-0.25,-0.9], [0.7,-0.25,-0.9], [-0.7,-0.25,0.8], [0.7,-0.25,0.8]];
        positions.forEach(pos => {
            const wheel = new THREE.Mesh(wheelGeo, wheelMat);
            wheel.rotation.z = Math.PI/2;
            wheel.position.set(pos[0], pos[1], pos[2]);
            wheel.castShadow = true;
            carGroup.add(wheel);
        });
        const headMat = new THREE.MeshStandardMaterial({ color: 0xffaa66, emissive: 0x442200 });
        const leftLight = new THREE.Mesh(new THREE.SphereGeometry(0.12,16,16), headMat);
        leftLight.position.set(-0.5, 0.15, 1.15);
        const rightLight = new THREE.Mesh(new THREE.SphereGeometry(0.12,16,16), headMat);
        rightLight.position.set(0.5, 0.15, 1.15);
        carGroup.add(leftLight, rightLight);
        const tailMat = new THREE.MeshStandardMaterial({ color: 0xcc3300, emissive: 0x331100 });
        const leftTail = new THREE.Mesh(new THREE.SphereGeometry(0.1,16,16), tailMat);
        leftTail.position.set(-0.55, 0.15, -1.1);
        const rightTail = new THREE.Mesh(new THREE.SphereGeometry(0.1,16,16), tailMat);
        rightTail.position.set(0.55, 0.15, -1.1);
        carGroup.add(leftTail, rightTail);
        this.car = carGroup;
        this.scene.add(this.car);
    }

    animate() {
        requestAnimationFrame(this.animate.bind(this));
        if (this.car) {
            this.car.rotation.y = Math.sin(Date.now() * 0.001) * 0.2;
            this.car.position.y = Math.sin(Date.now() * 0.002) * 0.03;
        }
        if (this.renderer) this.renderer.render(this.scene, this.camera);
    }

    onResize() {
        const width = this.container.clientWidth;
        const height = 400;
        this.camera.aspect = width/height;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(width, height);
    }

    render() {
        if (!this.renderer) this.init();
        window.addEventListener('resize', () => this.onResize());
    }
}
