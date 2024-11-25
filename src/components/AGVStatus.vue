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
  .status-container {
    padding: 20px;
    color: #333;
  }
  
  .status-item {
    margin-bottom: 30px;
  }
  
  .battery-indicator {
    background: #ddd;
    height: 20px;
    border-radius: 10px;
    position: relative;
    overflow: hidden;
  }
  
  .battery-level {
    background: #4CAF50;
    height: 100%;
    transition: width 0.3s ease;
  }
  
  .wifi-indicator {
    display: flex;
    gap: 4px;
    height: 30px;
    align-items: flex-end;
  }
  
  .signal-bar {
    width: 6px;
    background: #ddd;
    border-radius: 2px;
  }
  
  .signal-bar:nth-child(1) { height: 25%; }
  .signal-bar:nth-child(2) { height: 50%; }
  .signal-bar:nth-child(3) { height: 75%; }
  .signal-bar:nth-child(4) { height: 100%; }
  
  .signal-bar.active {
    background: #2196F3;
  }
  
  .status-badge {
    display: inline-block;
    padding: 6px 12px;
    border-radius: 4px;
    font-weight: bold;
  }
  
  .status-badge.running {
    background: #4CAF50;
    color: white;
  }
  </style>