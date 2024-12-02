<template>
    <div class="analytics-container">
        <div v-if="loading" class="loading-overlay">
            <div class="loading-spinner"></div>
        </div>
        <div v-else-if="error" class="error-message">
            {{ error }}
        </div>
        <div v-else class="charts-grid">
            <div class="chart-wrapper">
                <h3>Battery Level</h3>
                <canvas ref="batteryChart"></canvas>
            </div>
            <div class="chart-wrapper">
                <h3>Motor Status</h3>
                <canvas ref="motorChart"></canvas>
            </div>
            <div class="chart-wrapper">
                <h3>Servo Status</h3>
                <canvas ref="servoChart"></canvas>
            </div>
        </div>
    </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { firestore } from '@/firebase/config'
import { collection, getDocs } from 'firebase/firestore'
import { parseTimestamp } from '@/utils/dateUtils'
import Chart from 'chart.js/auto'
import 'chartjs-adapter-luxon'

const batteryChart = ref(null)
const motorChart = ref(null)
const servoChart = ref(null)
const loading = ref(true)
const error = ref(null)
let charts = []

const CHART_COLORS = [
    'rgb(97, 218, 251)',
    'rgb(255, 99, 132)',
    'rgb(75, 192, 192)',
    'rgb(255, 205, 86)',
    'rgb(153, 102, 255)',
    'rgb(255, 159, 64)'
]

const createBatteryChart = (canvas, data) => {
    return new Chart(canvas, {
        type: 'line',
        data: {
            labels: data.labels,
            datasets: [{
                label: 'Battery Level',
                data: data.values.battery,
                borderColor: 'rgb(97, 218, 251)',
                backgroundColor: 'rgba(97, 218, 251, 0.2)', // opacity 0.2로 조정
                borderWidth: 2,
                fill: true,
                tension: 0.4
            }]
        },
        options: createChartOptions('Battery %', 100)
    })
}

const createMotorChart = (canvas, data) => {
    return new Chart(canvas, {
        type: 'line',
        data: {
            labels: data.labels,
            datasets: [
                {
                    label: 'Motor 1',
                    data: data.values.motor.map(v => v[0]),
                    borderColor: CHART_COLORS[0],
                    backgroundColor: `${CHART_COLORS[0]}33`,
                    borderWidth: 2,
                    tension: 0.4
                },
                {
                    label: 'Motor 2',
                    data: data.values.motor.map(v => v[1]),
                    borderColor: CHART_COLORS[1],
                    backgroundColor: `${CHART_COLORS[1]}33`,
                    borderWidth: 2,
                    tension: 0.4
                }
            ]
        },
        options: createChartOptions('Motor Value')
    })
}

const createServoChart = (canvas, data) => {
    return new Chart(canvas, {
        type: 'line',
        data: {
            labels: data.labels,
            datasets: Array.from({ length: 5 }, (_, i) => ({
                label: `Servo ${i + 1}`,
                data: data.values.servo.map(v => v[i]),
                borderColor: CHART_COLORS[i],
                backgroundColor: `${CHART_COLORS[i]}33`,
                borderWidth: 2,
                tension: 0.4
            }))
        },
        options: createChartOptions('Servo Value')
    })
}

const createChartOptions = (yAxisLabel, maxY = undefined) => ({
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
        legend: {
            labels: { color: '#FFFFFF' }
        },
        tooltip: {
            mode: 'index',
            intersect: false,
        }
    },
    scales: {
        x: {
            type: 'time',
            time: {
                unit: 'minute',
                displayFormats: {
                    minute: 'HH:mm'
                }
            },
            ticks: { color: '#FFFFFF' },
            grid: { color: '#FFFFFF33' }
        },
        y: {
            beginAtZero: true,
            max: maxY,
            title: {
                display: true,
                text: yAxisLabel,
                color: '#FFFFFF'
            },
            ticks: { color: '#FFFFFF' },
            grid: { color: '#FFFFFF33' }
        }
    },
    interaction: {
        intersect: false,
        mode: 'index'
    }
})


const fetchData = async () => {
    try {
        loading.value = true
        error.value = null

        const sensingRef = collection(firestore, 'sensingTable')
        const snapshot = await getDocs(sensingRef)

        const documents = []

        const today = new Date()
        today.setHours(0, 0, 0, 0)
        
        snapshot.forEach(doc => {
            if (doc.id.match(/^\d{4}-\d{2}-\d{2} \d{2}:\d{2}$/)) {
                const currentTimeStamp = parseTimestamp(doc.id);
                if (currentTimeStamp >= today) {
                    documents.push({
                    timestamp: currentTimeStamp,
                    id: doc.id,
                    data: doc.data()
                })
                }

            }
        })

        documents.sort((a, b) => b.timestamp - a.timestamp)

        const data = {
            labels: [],
            values: {
                battery: [],
                motor: [],
                servo: []
            }
        }

        documents.slice(0, 100).forEach(doc => {
            data.labels.unshift(doc.timestamp)
            data.values.battery.unshift(doc.data.latest_battery)
            data.values.motor.unshift(doc.data.latest_motor)
            data.values.servo.unshift(doc.data.latest_servo)
        })

        return data
    } catch (err) {
        error.value = 'Failed to load data: ' + err.message
        throw err
    } finally {
        loading.value = false
    }
}

const updateCharts = (data) => {
    if (data.labels.length > 0) {
        charts.forEach(chart => chart.destroy())
        charts = []
        
        if (batteryChart.value) {
            charts.push(createBatteryChart(batteryChart.value, data))
        }
        if (motorChart.value) {
            charts.push(createMotorChart(motorChart.value, data))
        }
        if (servoChart.value) {
            charts.push(createServoChart(servoChart.value, data))
        }
    }
}

defineExpose({
    fetchData,
    updateCharts,
})

const drawChart = async () => {
    try {
        const data = await fetchData()
        if (data.labels.length > 0) {
            if (batteryChart.value) {
                charts.push(createBatteryChart(batteryChart.value, data))
            }
            if (motorChart.value) {
                charts.push(createMotorChart(motorChart.value, data))
            }
            if (servoChart.value) {
                charts.push(createServoChart(servoChart.value, data))
            }
        }
    } catch (err) {
        console.error('Chart creation failed:', err)
    }
}

onMounted(async () => {
    await drawChart();
})

onUnmounted(() => {
    charts.forEach(chart => chart.destroy())
})
</script>

<style scoped>

.analytics-container {
    position: relative;
    display: flex;
    flex-direction: column;
    gap: 20px;
    padding: 20px;
    height: 100%;
    background: var(--surface-dark);
}

.charts-grid {
    display: flex;
    flex-direction: column;
    gap: 20px;
    height: 100%;
    overflow-y: auto;
}

.chart-wrapper {
    background: var(--bg-darker);
    border: 1px solid var(--primary-300);
    border-radius: var(--radius-md);
    padding: 15px;
    height: 400px; /* 고정 높이 설정 */
    position: relative;
}

/* 나머지 스타일은 동일하게 유지 */
.chart-wrapper h3 {
    color: #FFFFFF;
    margin: 0 0 10px 0;
    font-size: 1.1em;
}

.loading-overlay {
    position: absolute;
    inset: 0;
    display: flex;
    justify-content: center;
    align-items: center;
    background: rgba(32, 35, 42, 0.8);
    backdrop-filter: blur(4px);
}

.loading-spinner {
    width: 40px;
    height: 40px;
    border: 3px solid var(--primary-300);
    border-top: 3px solid transparent;
    border-radius: 50%;
    animation: spin 1s linear infinite;
}

.error-message {
    color: var(--accent-red);
    text-align: center;
    padding: 20px;
}

@keyframes spin {
    0% { transform: rotate(0deg); }
    100% { transform: rotate(360deg); }
}

@media (min-width: 768px) {
    .charts-grid {
        grid-template-columns: repeat(2, 1fr);
    }
}

@media (min-width: 1200px) {
    .charts-grid {
        grid-template-columns: repeat(3, 1fr);
    }
}
</style>