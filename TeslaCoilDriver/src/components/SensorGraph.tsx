import { useEffect, useRef, useState } from 'react'
import type { TeslaCoilData } from '../utils/teslaCoilBluetooth'

interface DataPoint {
  timestamp: number
  data: TeslaCoilData
}

interface SensorGraphProps {
  data: TeslaCoilData | null
  maxDataPoints?: number
  updateInterval?: number
  disabled?: boolean
}

export default function SensorGraph({ 
  data, 
  maxDataPoints = 100, 
  updateInterval = 500,
  disabled = false,
}: SensorGraphProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const [dataPoints, setDataPoints] = useState<DataPoint[]>([])
  const [isVisible, setIsVisible] = useState(true)

  // Collect data points at regular intervals
  useEffect(() => {
    if (!data || disabled) return

    const interval = setInterval(() => {
      const newDataPoint: DataPoint = {
        timestamp: Date.now(),
        data: { ...data }
      }

      setDataPoints(prev => {
        const updated = [...prev, newDataPoint]
        // Keep only the last maxDataPoints
        return updated.slice(-maxDataPoints)
      })
    }, updateInterval)

    return () => clearInterval(interval)
  }, [data, maxDataPoints, updateInterval, disabled])

  // Draw the graph at regular intervals
  useEffect(() => {
    if (!data) return

    const drawGraph = () => {
      const canvas = canvasRef.current
      if (!canvas || dataPoints.length === 0) return

      const ctx = canvas.getContext('2d')
      if (!ctx) return

      const rect = canvas.getBoundingClientRect()
      canvas.width = rect.width * window.devicePixelRatio
      canvas.height = rect.height * window.devicePixelRatio
      ctx.scale(window.devicePixelRatio, window.devicePixelRatio)

      const width = rect.width
      const height = rect.height
      const padding = 40
      const chartWidth = width - 2 * padding
      const chartHeight = height - 2 * padding

      // Clear canvas
      ctx.fillStyle = 'rgba(0, 0, 0, 0.8)'
      ctx.fillRect(0, 0, width, height)

      // Draw grid
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)'
      ctx.lineWidth = 1
      for (let i = 0; i <= 10; i++) {
        const y = padding + (chartHeight / 10) * i
        ctx.beginPath()
        ctx.moveTo(padding, y)
        ctx.lineTo(width - padding, y)
        ctx.stroke()
      }

      // Draw axes
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)'
      ctx.lineWidth = 2
      ctx.beginPath()
      ctx.moveTo(padding, padding)
      ctx.lineTo(padding, height - padding)
      ctx.lineTo(width - padding, height - padding)
      ctx.stroke()

      // Find min/max values for scaling
      const allValues = dataPoints.flatMap(dp => [
        dp.data.vbus,
        dp.data.currentTransformer,
        dp.data.therm1,
        dp.data.therm2
      ])
      const minValue = Math.min(...allValues)
      const maxValue = Math.max(...allValues)
      const valueRange = maxValue - minValue || 1

      // Draw data lines
      const colors = {
        vbus: '#22c55e',           // Green
        current: '#3b82f6',        // Blue
        therm1: '#f59e0b',         // Orange
        therm2: '#ef4444'          // Red
      }

      const sensors = [
        { key: 'vbus', label: 'VBus', color: colors.vbus },
        { key: 'currentTransformer', label: 'Current', color: colors.current },
        { key: 'therm1', label: 'Therm1', color: colors.therm1 },
        { key: 'therm2', label: 'Therm2', color: colors.therm2 }
      ] as const

      sensors.forEach(sensor => {
        ctx.strokeStyle = sensor.color
        ctx.lineWidth = 2
        ctx.beginPath()

        dataPoints.forEach((point, index) => {
          const x = padding + (chartWidth / (dataPoints.length - 1)) * index
          const value = point.data[sensor.key as keyof TeslaCoilData]
          const y = height - padding - ((value - minValue) / valueRange) * chartHeight

          if (index === 0) {
            ctx.moveTo(x, y)
          } else {
            ctx.lineTo(x, y)
          }
        })

        ctx.stroke()
      })

      // Draw legend
      const legendX = width - 150
      const legendY = padding + 20
      sensors.forEach((sensor, index) => {
        ctx.fillStyle = sensor.color
        ctx.fillRect(legendX, legendY + index * 20, 12, 12)
        ctx.fillStyle = '#ffffff'
        ctx.font = '12px Arial'
        ctx.fillText(sensor.label, legendX + 16, legendY + index * 20 + 9)
      })

      // Draw time labels
      if (dataPoints.length > 1) {
        const timeRange = dataPoints[dataPoints.length - 1].timestamp - dataPoints[0].timestamp
        const timeLabels = 5
        ctx.fillStyle = 'rgba(255, 255, 255, 0.6)'
        ctx.font = '10px Arial'
        ctx.textAlign = 'center'

        for (let i = 0; i <= timeLabels; i++) {
          const x = padding + (chartWidth / timeLabels) * i
          const timeOffset = (timeRange / timeLabels) * i
          const time = new Date(dataPoints[0].timestamp + timeOffset)
          const timeStr = time.toLocaleTimeString('en-US', { 
            hour12: false, 
            hour: '2-digit', 
            minute: '2-digit', 
            second: '2-digit' 
          })
          ctx.fillText(timeStr, x, height - 10)
        }
      }

      // Draw value labels
      ctx.textAlign = 'right'
      ctx.fillStyle = 'rgba(255, 255, 255, 0.6)'
      ctx.font = '10px Arial'
      for (let i = 0; i <= 5; i++) {
        const value = minValue + (valueRange / 5) * i
        const y = height - padding - (chartHeight / 5) * i
        ctx.fillText(value.toFixed(1), padding - 10, y + 3)
      }
    }

    // Initial draw
    drawGraph()

    // Set up interval for regular redraws
    const interval = setInterval(drawGraph, updateInterval)

    return () => clearInterval(interval)
  }, [data, dataPoints, updateInterval])

  if (!isVisible) {
    return (
      <div className="sensor-graph-container">
        <button 
          className="graph-toggle-button"
          onClick={() => setIsVisible(true)}
        >
          Show Graph
        </button>
      </div>
    )
  }

  return (
    <div className="sensor-graph-container">
      <div className="graph-header">
        <h3>Sensor Readings Over Time</h3>
        <button 
          className="graph-toggle-button"
          onClick={() => setIsVisible(false)}
        >
          Hide Graph
        </button>
      </div>
      <div className="graph-info">
        <span>Data Points: {dataPoints.length}</span>
        <span>Range: {dataPoints.length > 0 ? 
          `${Math.round((dataPoints[dataPoints.length - 1].timestamp - dataPoints[0].timestamp) / 1000)}s` : 
          '0s'
        }</span>
      </div>
      <canvas 
        ref={canvasRef}
        className="sensor-graph-canvas"
        style={{ width: '100%', height: '300px' }}
      />
    </div>
  )
}
