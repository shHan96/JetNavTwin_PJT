<template>
  <div class="monitor-container">
    <div class="monitor-panel three-panel">
      <ThreeScene />
    </div>
    <div class="monitor-panel status-panel">
      <AGVStatus />
    </div>
  </div>
</template>

<script setup>
import ThreeScene from '../components/ThreeScene.vue'
import AGVStatus from '../components/AGVStatus.vue'
</script>

<style scoped>
.monitor-container {
  display: flex;
  width: 100vw;
  height: 100vh;
  margin: 0;
  padding: 0;
  overflow: hidden;
}

.monitor-panel {
  flex: 0 0 50%;
  height: 100%;
  margin: 0;
  padding: 0;
  overflow: hidden;
  position: relative;
}

.three-panel {
  border-right: 1px solid #ddd;
}

.status-panel {
  z-index: 1;
  background-color: #f5f5f5;
}
</style>