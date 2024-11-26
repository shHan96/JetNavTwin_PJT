<template>
  <div class="monitor-container">
    <header class="tech-header">
      <div class="header-content">
        <div class="header-bg"></div>
        <h1 class="title">
          <span class="text-gradient">SSAFY AGV</span>
          <span class="subtitle">WEB GUI</span>
        </h1>
        <nav class="header-nav">
          <router-link to="/" class="nav-link">Monitor</router-link>
          <router-link to="/analytics" class="nav-link">Analytics</router-link>
        </nav>
        <div class="scan-line"></div>
        <div class="corner top-left"></div>
        <div class="corner top-right"></div>
        <div class="corner bottom-left"></div>
        <div class="corner bottom-right"></div>
      </div>
    </header>
    <div class="monitor-panel three-panel">
      <ThreeScene />
    </div>
    <div class="monitor-panel status-panel">
      <div class="status-section">
        <AGVStatus />
      </div>
      <div class="analytics-section">
        <DataAnalytics />
      </div>
    </div>
    <footer class="footer">
      <p>SSAFY 12th AGB Team [팀장: 한상훈, 팀원: 윤여범]</p>
    </footer>
  </div>
</template>

<script setup>
import ThreeScene from '../components/ThreeScene.vue'
import AGVStatus from '../components/AGVStatus.vue'
import DataAnalytics from '../components/DataAnalytics.vue'
</script>

<!-- AGVMonitor.vue -->
<style scoped>
.header-nav {
    position: absolute;
    right: 20px;
    display: flex;
    gap: 20px;
}

.nav-link {
    color: var(--text-primary);
    text-decoration: none;
    padding: 8px 16px;
    border-radius: var(--radius-sm);
    transition: all var(--transition-normal);
}

.nav-link:hover {
    background: var(--primary-300);
    color: var(--surface-dark);
    box-shadow: 0 0 15px var(--primary-300);
}

.router-link-active {
    background: var(--primary-300);
    color: var(--surface-dark);
}

.monitor-container {
  display: flex;
  width: 100vw;
  height: 100vh;
  margin: 0;
  padding: 0;
  overflow: hidden;
  background: linear-gradient(135deg, var(--bg-darker) 0%, var(--bg-dark) 100%);
  padding-top: calc(60px + 2rem);
  padding-bottom: 4rem; /* footer 높이만큼 패딩 추가 */
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
  display: flex;
  justify-content: center;
  align-items: center;
  background: var(--bg-dark);
  border-right: 1px solid var(--primary-300);
}

.status-panel {
  display: flex;
  flex-direction: column;
}

.status-section {
  flex: 0 0 auto;
}

.analytics-section {
  flex: 1;
  overflow: auto;
}

.tech-header {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  z-index: 10;
  padding: 1rem;
}

.header-content {
  position: relative;
  height: 60px;
  background: var(--surface-dark);
  border: 1px solid var(--primary-300);
  border-radius: var(--radius-md);
  display: flex;
  align-items: center;
  justify-content: center;
  overflow: hidden;
}

.header-bg {
  position: absolute;
  inset: 0;
  background: linear-gradient(90deg, 
    transparent 0%,
    rgba(97, 218, 251, 0.1) 15%,
    rgba(97, 218, 251, 0.1) 85%,
    transparent 100%
  );
  animation: pulse 3s ease-in-out infinite;
}

.title {
  font-size: 1.5rem;
  letter-spacing: 2px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 4px;
}

.text-gradient {
  background: var(--gradient-primary);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  animation: hue-rotate 10s linear infinite;
}

.subtitle {
  font-size: 0.9rem;
  color: var(--primary-300);
  opacity: 0.8;
}

.scan-line {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 2px;
  background: var(--gradient-primary);
  animation: scan 2s linear infinite;
}

.corner {
  position: absolute;
  width: 10px;
  height: 10px;
  border: 1px solid var(--primary-300);
}

.top-left {
  top: -1px;
  left: -1px;
  border-right: none;
  border-bottom: none;
}

.top-right {
  top: -1px;
  right: -1px;
  border-left: none;
  border-bottom: none;
}

.bottom-left {
  bottom: -1px;
  left: -1px;
  border-right: none;
  border-top: none;
}

.bottom-right {
  bottom: -1px;
  right: -1px;
  border-left: none;
  border-top: none;
}

.footer {
  position: fixed;
  bottom: 0;
  width: 100%;
  text-align: center;
  padding: 1rem;
  background: var(--surface-dark);
  color: var(--text-primary);
  border-top: 1px solid var(--primary-300);
}

@keyframes scan {
  0% { transform: translateX(-100%); }
  100% { transform: translateX(100%); }
}

@keyframes pulse {
  0%, 100% { opacity: 0.5; }
  50% { opacity: 0.8; }
}

</style>