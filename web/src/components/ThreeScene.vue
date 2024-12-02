<template>
    <div class="scene-container">
        <div ref="container" class="three-scene"></div>
        <div class="controls">
            <button 
                @click="connectWebSocket" 
                :class="{'connected': isConnected}"
            >
                {{ isConnected ? 'Connected' : 'Connect WebSocket' }}
            </button>
        </div>
    </div>
</template>

<script setup>
import { useWebSocketStore } from '@/stores/websocket'
import { storeToRefs } from 'pinia'
import { ref, onMounted, onBeforeUnmount } from 'vue'
import * as THREE from 'three'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader'

const container = ref(null)
const store = useWebSocketStore()

let animationId = null
let scene, camera, renderer, controls
const trackSpeedScaler = 0.05
const axies = [new THREE.Vector3(0, 0, 1), new THREE.Vector3(1, 0, 0), new THREE.Vector3(1, 0, 0), new THREE.Vector3(0, 1, 0), new THREE.Vector3(1, 0, 0)]
let tracks = []
let models = {}

// Use store values
const { isConnected, jointRotations, trackSpeeds } = storeToRefs(store)
const { connectWebSocket, disconnect } = store // disconnect 추가

const loadModel = (path, material) => {
    return new Promise((resolve, reject) => {
        const loader = new OBJLoader()
        loader.load(
            path,
            (object) => {
                object.traverse((child) => {
                    if (child instanceof THREE.Mesh) {
                        child.material = material
                    }
                })
                resolve(object)
            },
            (xhr) => console.log((xhr.loaded / xhr.total * 100) + '% loaded'),
            (error) => reject(error)
        )
    })
}

const initScene = () => {
    scene = new THREE.Scene()
    scene.background = new THREE.Color(0xffffffff)

    const width = container.value.clientWidth
    const height = container.value.clientHeight

    camera = new THREE.PerspectiveCamera(75, width / height, 0.1, 1000)
    // 카메라 위치 조정: 더 멀리, 약간 비스듬히
    camera.position.set(4, 2, 4)
    // 카메라가 scene의 중심을 바라보도록 설정
    camera.lookAt(new THREE.Vector3(0, 0, 0))

    renderer = new THREE.WebGLRenderer({ antialias: true })
    renderer.setSize(width, height)
    container.value.appendChild(renderer.domElement)

    // Add OrbitControls
    controls = new OrbitControls(camera, renderer.domElement)
    controls.enableDamping = true
    controls.dampingFactor = 0.05
    controls.screenSpacePanning = false
    controls.minDistance = 3
    controls.maxDistance = 10
    controls.mouseButtons = {
        LEFT: THREE.MOUSE.ROTATE,
        MIDDLE: THREE.MOUSE.DOLLY,
        RIGHT: null  // 우클릭 비활성화
    }

    // Lighting setup for better contrast
    renderer.shadowMap.enabled = true
    renderer.shadowMap.type = THREE.PCFSoftShadowMap

    // Ambient light for base illumination
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.3) // 강도 감소
    scene.add(ambientLight)

    // Main directional light
    const mainLight = new THREE.DirectionalLight(0xffffff, 0.8)
    mainLight.position.set(5, 10, 5)
    mainLight.castShadow = true
    mainLight.shadow.camera.near = 0.1
    mainLight.shadow.camera.far = 100
    mainLight.shadow.mapSize.width = 2048
    mainLight.shadow.mapSize.height = 2048
    scene.add(mainLight)

    // Fill light from opposite side
    const fillLight = new THREE.DirectionalLight(0xffffff, 0.4)
    fillLight.position.set(-5, 5, -5)
    scene.add(fillLight)

    // Bottom rim light for edge definition
    const rimLight = new THREE.DirectionalLight(0xffffff, 0.2)
    rimLight.position.set(0, -5, 0)
    scene.add(rimLight)

    // Grid
    const gridHelper = new THREE.GridHelper(10, 10)
    scene.add(gridHelper)
}

const createTrackPair = (leftPos, rightPos, isLeft) => {
    const trackGroup = new THREE.Group()

    // Front wheel
    const frontWheel = createWheel()
    frontWheel.position.copy(leftPos)
    trackGroup.add(frontWheel)

    // Back wheel
    const backWheel = createWheel()
    backWheel.position.copy(rightPos)
    trackGroup.add(backWheel)

    // Create links between wheels
    const linkCount = 16
    const wheelDistance = new THREE.Vector3().subVectors(rightPos, leftPos).length()
    const radius = 0.5

    // Top links
    for (let i = 0; i < linkCount / 2; i++) {
        const link = createTrackLink(radius, wheelDistance)
        const t = i / (linkCount / 2 - 1)
        link.position.copy(leftPos).lerp(rightPos, t)
        link.position.y += radius
        trackGroup.add(link)
    }

    // Bottom links
    for (let i = 0; i < linkCount / 2; i++) {
        const link = createTrackLink(radius, wheelDistance)
        const t = i / (linkCount / 2 - 1)
        link.position.copy(leftPos).lerp(rightPos, t)
        link.position.y -= radius
        trackGroup.add(link)
    }

    return trackGroup
}

const createWheel = () => {
    const wheelGeometry = new THREE.CylinderGeometry(0.5, 0.5, 0.2, 32)
    const wheelMaterial = new THREE.MeshStandardMaterial({
        color: 0x666666,
        metalness: 0.5,
        roughness: 0.5
    })
    const wheel = new THREE.Mesh(wheelGeometry, wheelMaterial)
    wheel.rotation.x = Math.PI / 2
    wheel.rotation.z = Math.PI / 2
    return wheel
}

const createTrackLink = (radius, length) => {
    const linkGeometry = new THREE.BoxGeometry(length * 0.2, 0.1, 0.2)
    const linkMaterial = new THREE.MeshStandardMaterial({
        color: 0x888888, // 더 밝은 회색으로 변경
        metalness: 0.5,
        roughness: 0.5
    })
    return new THREE.Mesh(linkGeometry, linkMaterial)
}

const initTracks = () => {
    // Left side track pair
    const leftTrack = createTrackPair(
        new THREE.Vector3(-1, 0, -1),  // front
        new THREE.Vector3(-1, 0, 1),   // back
        true
    )

    // Right side track pair
    const rightTrack = createTrackPair(
        new THREE.Vector3(1, 0, -1),   // front
        new THREE.Vector3(1, 0, 1),    // back
        false
    )

    tracks = [leftTrack, rightTrack]
    tracks.forEach(track => scene.add(track))
}

const loadModels = async () => {
    const bodyMaterial = new THREE.MeshStandardMaterial({
        color: 0xAAAAAA,
        metalness: 0.6,
        roughness: 0.4
    })

    const gripperMaterial = new THREE.MeshStandardMaterial({
        color: "#cde4da",
        metalness: 0.3,
        roughness: 0.4
    })

    const cameraMaterial = new THREE.MeshStandardMaterial({
        color: 0x666666,
        metalness: 0.3,
        roughness: 0.4
    })

    const ssafyMaterial = new THREE.MeshStandardMaterial({
        color: "#00C3FF",
        metalness: 0.3,
        roughness: 0.4
    })

    try {
        // Load models sequentially
        models.body = await loadModel('/models/body.obj', bodyMaterial)
        models.joint1 = await loadModel('/models/joint1.obj', bodyMaterial)
        models.joint2 = await loadModel('/models/joint2.obj', bodyMaterial)
        models.joint3 = await loadModel('/models/joint3.obj', bodyMaterial)
        models.gripper1 = await loadModel('/models/gripper1.obj', gripperMaterial)
        models.gripper2 = await loadModel('/models/gripper2.obj', gripperMaterial)
        models.gripper3 = await loadModel('/models/gripper3.obj', gripperMaterial)
        models.joint5 = await loadModel('/models/joint5.obj', cameraMaterial)
        models.ssafy = await loadModel('/models/SSAFY.obj', ssafyMaterial)

        // Set up hierarchy
        models.body.rotation.x = -Math.PI / 2
        models.body.scale.set(50, 50, 50)
        scene.add(models.body)
        models.body.add(models.joint1)
        models.joint1.position.set(0, -0.010, 0)
        models.joint1.add(models.joint2)
        models.joint2.position.set(0, 0.010 - 0.01378, 0.03553)
        models.joint2.add(models.joint3)
        models.joint3.position.set(0, 0, 0.016)
        models.joint3.add(models.gripper1)
        models.gripper1.position.set(0, 0, 0.018)
        models.gripper1.add(models.gripper2)
        models.gripper1.add(models.gripper3)
        models.joint1.add(models.joint5)
        models.joint5.position.set(0, -0.0065, 0.030)
        models.body.add(models.ssafy)

        jointRotations.value.forEach((_, index) => {
                            updateJointRotation(index)
                        })

        // Initial position for joint1
    } catch (error) {
        console.error('Error loading models:', error)
    }
}

const handleJointUpdate = () => {
    jointRotations.value.forEach((_, index) => {
        updateJointRotation(index)
    })
}

const updateJointRotation = (jointIndex) => {
    if (jointIndex === 3) {
        const angle = (jointRotations.value[jointIndex] * Math.PI) / 180
        const quaternion1 = new THREE.Quaternion()
        const quaternion2 = new THREE.Quaternion()
        quaternion1.setFromAxisAngle(new THREE.Vector3(0, 1, 0), angle)
        models.gripper2.setRotationFromQuaternion(quaternion1)
        quaternion2.setFromAxisAngle(new THREE.Vector3(0, -1, 0), angle)
        models.gripper3.setRotationFromQuaternion(quaternion2)
    }
    else {
        const joint = models[`joint${jointIndex + 1}`]
        if (joint) {
            const angle = (jointRotations.value[jointIndex] * Math.PI) / 180
            const quaternion = new THREE.Quaternion()
            quaternion.setFromAxisAngle(axies[jointIndex], angle)
            joint.setRotationFromQuaternion(quaternion)
        }
    }
}

// Modify onMounted
onMounted(async () => {
    initScene()
    initTracks()
    await loadModels()
    animate()
    window.addEventListener('resize', handleResize)
    window.addEventListener('jointUpdate', handleJointUpdate) // Add event listener
    window.addEventListener('trackUpdate', updateTracks)
})


const updateTracks = () => {   
    tracks.forEach((track, index) => {
        const speed = trackSpeeds.value[index] * trackSpeedScaler // Scale down the speed for smoother movement
        
        track.traverse((child) => {
            if (child instanceof THREE.Mesh) {
                if (child.geometry instanceof THREE.CylinderGeometry) {
                    // Rotate the wheels
                    child.rotation.x += speed
                } else {
                    // Move the track links
                    const isTopLink = child.position.y > 0
                    child.position.z += isTopLink ? speed : -speed

                    // Reset link position when it reaches the end
                    if (child.position.z > 1) child.position.z = -1
                    if (child.position.z < -1) child.position.z = 1
                }
            }
        })
    })
}

const animate = () => {
    animationId = requestAnimationFrame(animate) // Store the ID
    updateTracks()
    if (controls) controls.update()
    renderer.render(scene, camera)
}

const handleResize = () => {
    if (!container.value) return

    const width = container.value.clientWidth
    const height = container.value.clientHeight

    camera.aspect = width / height
    camera.updateProjectionMatrix()

    renderer.setSize(width, height)
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2))
}

onBeforeUnmount(() => {
    disconnect() // 직접 ws를 참조하는 대신 store의 disconnect 메서드 사용
    window.removeEventListener('resize', handleResize)
    window.removeEventListener('jointUpdate', handleJointUpdate)
    window.removeEventListener('trackUpdate', updateTracks)
    
    if (animationId) {
        cancelAnimationFrame(animationId)
    }
    if (renderer) {
        renderer.dispose()
    }
    if (controls) {
        controls.dispose()
    }
})

</script>

<!-- ThreeScene.vue -->
<style scoped>
.scene-container {
  position: relative;
  width: 95%;
  height: 95%;
  margin: 20px;
  background: var(--surface-dark);
  border-radius: var(--radius-lg);
  border: 1px solid var(--primary-300);
  box-shadow: var(--shadow-lg);
  overflow: hidden;
}

.three-scene {
  width: 100%;
  height: 100%;
  border-radius: var(--radius-lg);
  background: linear-gradient(135deg, var(--bg-darker) 0%, var(--bg-dark) 100%);
}

.controls {
  position: absolute;
  bottom: 20px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(32, 35, 42, 0.9);
  backdrop-filter: blur(10px);
  border: 1px solid var(--primary-300);
  padding: 15px;
  border-radius: var(--radius-md);
  box-shadow: var(--shadow-lg);
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.track-controls {
  position: absolute;
  bottom: 20px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(32, 35, 42, 0.9);
  backdrop-filter: blur(10px);
  border: 1px solid var(--primary-300);
  padding: 15px;
  border-radius: var(--radius-md);
  color: var(--text-primary);
}

.controls button {
  padding: 8px 16px;
  border: none;
  border-radius: var(--radius-sm);
  background: var(--surface-dark);
  color: var(--text-primary);
  cursor: pointer;
  transition: all var(--transition-normal);
  border: 1px solid var(--primary-300);
}

.controls button:hover {
  background: var(--primary-300);
  color: var(--surface-dark);
  box-shadow: 0 0 15px var(--primary-300);
}

.controls button.connected {
  background: var(--accent-green);
  color: var(--surface-dark);
  box-shadow: 0 0 15px var(--accent-green);
}

.track-control {
  display: flex;
  align-items: center;
  gap: 10px;
  margin: 5px 0;
}

.track-control input[type="range"] {
  width: 150px;
  -webkit-appearance: none;
  background: var(--surface-light);
  border-radius: var(--radius-sm);
  height: 6px;
}

.track-control input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 16px;
  height: 16px;
  background: var(--primary-300);
  border-radius: 50%;
  cursor: pointer;
  transition: all var(--transition-normal);
}

.track-control input[type="range"]::-webkit-slider-thumb:hover {
  background: var(--primary-500);
  box-shadow: 0 0 10px var(--primary-300);
}
</style>