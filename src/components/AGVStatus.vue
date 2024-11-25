<template>
  <div class="status-container">
    <h2>AGV Status</h2>

    <div class="status-item">
      <h3>Battery</h3>
      <div class="battery-indicator">
        <div class="battery-level" :style="{ width: batteryLevel + '%' }"></div>
        <span>{{ batteryLevel }}%</span>
      </div>
    </div>

    <div class="status-item">
      <h3>WiFi Signal</h3>
      <div class="wifi-indicator">
        <div :class="['signal-bar', wifiStrength >= 1 ? 'active' : '']"></div>
        <div :class="['signal-bar', wifiStrength >= 2 ? 'active' : '']"></div>
        <div :class="['signal-bar', wifiStrength >= 3 ? 'active' : '']"></div>
        <div :class="['signal-bar', wifiStrength >= 4 ? 'active' : '']"></div>
      </div>
    </div>

    <div class="status-item">
      <h3>Operation Status</h3>
      <div class="status-badge" :class="operationStatus.toLowerCase()">
        {{ operationStatus }}
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useWebSocketStore } from '@/stores/websocket'
import { storeToRefs } from 'pinia'

const store = useWebSocketStore()
const { batteryLevel, wifiStrength, operationStatus } = storeToRefs(store)
</script>

<style scoped>
/* Import fonts in index.html or main CSS file */
@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;500;600;700&family=Rajdhani:wght@400;500;600;700&display=swap');

.status-container {
  padding: 20px;
  color: var(--text-primary);
  background: var(--surface-dark);
  border-radius: var(--radius-md);
  box-shadow: var(--shadow-md);
}

.status-container h2 {
  font-family: 'Orbitron', sans-serif;
  font-weight: 700;
  letter-spacing: 2px;
  margin-bottom: 20px;
  text-transform: uppercase;
}

.status-item {
  margin-bottom: 30px;
  padding: 15px;
  border: 1px solid var(--primary-300);
  border-radius: var(--radius-sm);
  background: rgba(97, 218, 251, 0.1);
}

.status-item h3 {
  font-family: 'Rajdhani', sans-serif;
  font-weight: 600;
  letter-spacing: 1px;
  margin-bottom: 10px;
}

.battery-indicator {
  background: var(--bg-darker);
  height: 20px;
  border-radius: var(--radius-full);
  position: relative;
  overflow: hidden;
  border: 1px solid var(--primary-300);
}

.battery-level {
  background: var(--primary-300);
  height: 100%;
  transition: width 0.3s ease;
  box-shadow: 0 0 10px var(--primary-300);
}

.wifi-indicator {
  display: flex;
  gap: 4px;
  height: 30px;
  align-items: flex-end;
}

.signal-bar {
  width: 6px;
  background: var(--bg-darker);
  border-radius: 2px;
  transition: background-color 0.3s ease;
}

.signal-bar:nth-child(1) {
  height: 25%;
}

.signal-bar:nth-child(2) {
  height: 50%;
}

.signal-bar:nth-child(3) {
  height: 75%;
}

.signal-bar:nth-child(4) {
  height: 100%;
}

.signal-bar.active {
  background: var(--primary-300);
  box-shadow: 0 0 10px var(--primary-300);
}

.status-badge {
  display: inline-block;
  padding: 6px 12px;
  border-radius: var(--radius-sm);
  font-weight: bold;
  border: 1px solid var(--primary-300);
  background: var(--surface-dark);
}

.status-badge.running {
  background: var(--accent-green);
  color: var(--text-primary);
  box-shadow: 0 0 10px var(--accent-green);
}
</style>