// src/router/index.js
import { createRouter, createWebHistory } from 'vue-router'
import AGVMonitor from '../views/AGVMonitor.vue'
import DataAnalyticsPage from '../views/DataAnalyticsPage.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: AGVMonitor
    },
    {
      path: '/analytics',
      name: 'analytics',
      component: DataAnalyticsPage
    }
  ]
})

export default router