import { createRouter, createWebHistory } from 'vue-router'
import AGVMonitor from '../views/AGVMonitor.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'agv-monitor',
      component: AGVMonitor,
    }
  ],
})

export default router