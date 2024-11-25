// stores/websocket.js
import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useWebSocketStore = defineStore('websocket', () => {
  // State
  const isConnected = ref(false)
  const jointRotations = ref([0, 20, 90, 0, 0])
  const trackSpeeds = ref([0, 0])
  const batteryLevel = ref(85)
  const wifiStrength = ref(3)
  const operationStatus = ref('IDLE')

  // WebSocket setup
  let ws = null
  const messageQueue = []
  let isProcessing = false
  let pingTimeout = null

  // Methods
  const connectWebSocket = () => {
    if (ws?.readyState === WebSocket.OPEN) return

    const wsUrl = `ws://${window.location.host}`
    ws = new WebSocket(wsUrl)
    
    ws.onopen = () => {
      isConnected.value = true
      startHeartbeat()
    }

    ws.onclose = () => {
      isConnected.value = false
      clearTimeout(pingTimeout)
      setTimeout(connectWebSocket, 5000)
    }

    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data)
        messageQueue.push(data)
        processQueue()
      } catch (error) {
        console.error('Message parsing error:', error)
      }
    }
  }

  const processQueue = async () => {
    if (isProcessing || messageQueue.length === 0) return
    
    isProcessing = true
    while (messageQueue.length > 0) {
      const data = messageQueue.shift()
      
      if (data.type === 'control') {
        // Update robot control data
        if (data.jointRotations) {
          jointRotations.value = data.jointRotations

            // date preprocessing
            jointRotations.value[0] = -jointRotations.value[0]
            jointRotations.value[1] = 20 + jointRotations.value[1]
            jointRotations.value[2] = 90 - jointRotations.value[2]
            jointRotations.value[3] = 20 - ((jointRotations.value[3] + 75) * 40 / 75)

          window.dispatchEvent(new CustomEvent('jointUpdate'))
        }
        if (data.trackSpeeds) {
          trackSpeeds.value = data.trackSpeeds

          window.dispatchEvent(new CustomEvent('trackUpdate'))
        }
      }
      else if (data.type === 'status') {
        // Update status data
        if (data.battery) batteryLevel.value = data.battery
        if (data.wifi) wifiStrength.value = data.wifi
        if (data.operation) operationStatus.value = data.operation
      }
    }
    isProcessing = false
  }

  const disconnect = () => {
    if (ws) ws.close()
    clearTimeout(pingTimeout)
  }

  const startHeartbeat = () => {
    clearTimeout(pingTimeout)
    pingTimeout = setTimeout(() => ws?.close(), 35000)
  }

  return {
    isConnected,
    jointRotations,
    trackSpeeds,
    batteryLevel, 
    wifiStrength,
    operationStatus,
    connectWebSocket,
    disconnect
  }
})