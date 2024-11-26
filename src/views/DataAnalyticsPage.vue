<template>
    <div class="analytics-page">
        <header class="tech-header">
            <div class="header-content">
                <router-link to="/" class="back-btn">
                    <span class="material-icons">arrow_back</span>
                    Back to Monitor
                </router-link>
                <h1 class="title">
                    <span class="text-gradient">Data Analytics</span>
                </h1>
            </div>
        </header>
        <div class="controls-panel">
            <div class="date-controls">
                <div class="control-group">
                    <label>Date Range</label>
                    <input type="datetime-local" v-model="startDate" class="date-input">
                    <span>to</span>
                    <input type="datetime-local" v-model="endDate" class="date-input">
                </div>
                <div class="control-group">
                    <label>Time Unit</label>
                    <select v-model="timeUnit">
                        <option value="1">1 Second</option>
                        <option value="15">15 Seconds</option>
                        <option value="30">30 Seconds</option>
                        <option value="60">1 Minute</option>
                        <option value="900">15 Minutes</option>
                        <option value="1800">30 Minutes</option>
                        <option value="3600">1 Hour</option>
                    </select>
                </div>
                <button class="apply-btn" @click="applyChanges">Apply</button>
            </div>
        </div>
        <div class="charts-container">
            <DataAnalytics ref="analytics" />
        </div>
    </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, nextTick } from 'vue'
import DataAnalytics from '@/components/DataAnalytics.vue'
import { parseTimestamp } from '@/utils/dateUtils'

const startDate = ref('')
const endDate = ref('')
const timeUnit = ref('60') 
const analytics = ref(null)
const unsubscribe = null;

const setInitialDateRange = (data) => {
    if (data && data.labels && data.labels.length > 0) {
        const firstDate = data.labels[0]
        const lastDate = data.labels[data.labels.length - 1]
        startDate.value = new Date(firstDate).toISOString().slice(0, 16)
        endDate.value = new Date(lastDate).toISOString().slice(0, 16)
    }
}

const resampleData = (data, timeUnitSeconds) => {
    const start = new Date(startDate.value).getTime()
    const end = new Date(endDate.value).getTime()
    
    // Filter data within time range
    const filteredIndices = data.labels.map((time, index) => {
        const timestamp = new Date(time).getTime()
        return timestamp >= start && timestamp <= end ? index : -1
    }).filter(i => i !== -1)

    if (filteredIndices.length === 0) return null

    const timeGroups = {}
    filteredIndices.forEach(index => {
        const timestamp = new Date(data.labels[index]).getTime()
        const groupTime = Math.floor(timestamp / (timeUnitSeconds * 1000)) * (timeUnitSeconds * 1000)
        
        if (!timeGroups[groupTime]) {
            timeGroups[groupTime] = {
                battery: [],
                motor: [],
                servo: []
            }
        }
        
        timeGroups[groupTime].battery.push(data.values.battery[index])
        timeGroups[groupTime].motor.push(data.values.motor[index])
        timeGroups[groupTime].servo.push(data.values.servo[index])
    })

    const resampledData = {
        labels: [],
        values: {
            battery: [],
            motor: [],
            servo: []
        }
    }

    Object.keys(timeGroups).sort().forEach(time => {
        resampledData.labels.push(new Date(parseInt(time)))
        resampledData.values.battery.push(
            timeGroups[time].battery.reduce((a, b) => a + b, 0) / timeGroups[time].battery.length
        )
        
        // Average for each motor value
        const motorAvg = timeGroups[time].motor[0].map((_, i) => 
            timeGroups[time].motor.reduce((sum, arr) => sum + arr[i], 0) / timeGroups[time].motor.length
        )
        resampledData.values.motor.push(motorAvg)
        
        // Average for each servo value
        const servoAvg = timeGroups[time].servo[0].map((_, i) => 
            timeGroups[time].servo.reduce((sum, arr) => sum + arr[i], 0) / timeGroups[time].servo.length
        )
        resampledData.values.servo.push(servoAvg)
    })

    return resampledData
}

// 컬렉션 변경사항을 감지하고 차트를 업데이트하는 리스너
function setupChartListener() {
  const collectionRef = db.collection('sensingTable'); // 원하는 컬렉션명으로 변경하세요
  
  const unsubscribe = collectionRef.onSnapshot((snapshot) => {
    snapshot.docChanges().forEach((change) => {
      if (change.type === 'added') {
        // 데이터 추가 시 차트 업데이트
        drawChart();
      }
    });
  }, (error) => {
    console.error('리스너 에러:', error);
  });

  return unsubscribe; // 리스너 해제 함수 반환
}

const drawChart = async () => {
    if (analytics.value) {
        const initialData = await analytics.value.fetchData()
        setInitialDateRange(initialData)
        
        await nextTick()
        updateCharts(initialData)
    }
}

onMounted(async () => {
    if (analytics.value) {
        const initialData = await analytics.value.fetchData()
        setInitialDateRange(initialData)
    }
    unsubscribe = setupChartListener();
})

onUnmounted(async () => {
    unsubscribe();
})

const applyChanges = async () => {
    if (analytics.value) {
        const rawData = await analytics.value.fetchData()
        if (!startDate.value || !endDate.value) {
            setInitialDateRange(rawData)
        }
        const processedData = resampleData(rawData, parseInt(timeUnit.value))
        if (processedData) {
            analytics.value.updateCharts(processedData)
        }
    }
}
</script>

<style scoped>
/* Page Layout */
.analytics-page {
    min-height: 100vh;
    background: var(--surface-dark);
    color: var(--text-primary);
    font-family: 'Roboto', 'Noto Sans KR', sans-serif;
}

/* Header Styles */
.tech-header {
    padding: 1rem;
    margin-bottom: 20px;
    background: var(--bg-darker);
    border-bottom: 1px solid var(--primary-300);
}

.header-content {
    display: flex;
    align-items: center;
    justify-content: space-between;
    max-width: 1400px;
    margin: 0 auto;
    padding: 0 20px;
}

.back-btn {
    display: flex;
    align-items: center;
    gap: 8px;
    color: var(--primary-300);
    text-decoration: none;
    padding: 8px 16px;
    border: 1px solid var(--primary-300);
    border-radius: var(--radius-sm);
    transition: all var(--transition-normal);
}

.back-btn:hover {
    background: var(--primary-300);
    color: var(--surface-dark);
    box-shadow: 0 0 15px var(--primary-300);
}

.title {
    font-size: 1.5rem;
    letter-spacing: 1px;
}

.text-gradient {
    background: var(--gradient-primary);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
}

/* Controls Panel */
.controls-panel {
    background: var(--bg-darker);
    border: 1px solid var(--primary-300);
    border-radius: var(--radius-md);
    padding: 20px;
    margin: 0 20px 20px 20px;
}

.date-controls {
    display: flex;
    gap: 20px;
    align-items: center;
    flex-wrap: wrap;
}

.control-group {
    display: flex;
    align-items: center;
    gap: 10px;
}

.control-group label {
    color: var(--text-primary);
    font-weight: bold;
}

/* Input Styles */
input[type="datetime-local"],
select {
    background: var(--bg-darker);
    color: var(--text-primary);
    border: 1px solid var(--primary-300);
    border-radius: var(--radius-sm);
    padding: 8px 12px;
    outline: none;
    font-family: inherit;
}

input[type="datetime-local"]:focus,
select:focus {
    border-color: var(--primary-500);
    box-shadow: 0 0 10px var(--primary-300);
}

input[type="datetime-local"]::-webkit-calendar-picker-indicator {
    filter: invert(1) hue-rotate(180deg);
    opacity: 0.8;
    cursor: pointer;
}

/* Button Styles */
.apply-btn {
    background: var(--primary-300);
    color: var(--surface-dark);
    border: none;
    padding: 8px 16px;
    border-radius: var(--radius-sm);
    cursor: pointer;
    transition: all var(--transition-normal);
    font-family: inherit;
}

.apply-btn:hover {
    background: var(--primary-500);
    box-shadow: 0 0 15px var(--primary-300);
}

/* Charts Container */
.charts-container {
    padding: 0 20px;
    height: calc(100vh - 200px);
}
</style>