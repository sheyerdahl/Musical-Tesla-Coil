import { useEffect, useRef } from 'react'
import type { FrequencySweepData } from '../utils/teslaCoilBluetooth'

interface FrequencySweepGraphProps {
  data: FrequencySweepData[]
}

export default function FrequencySweepGraph({ data }: FrequencySweepGraphProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas || data.length === 0) return

    const ctx = canvas.getContext('2d')
    if (!ctx) return

    const rect = canvas.getBoundingClientRect()
    canvas.width = rect.width * window.devicePixelRatio
    canvas.height = rect.height * window.devicePixelRatio
    ctx.scale(window.devicePixelRatio, window.devicePixelRatio)

    const width = rect.width
    const height = rect.height
    const padding = 60
    const chartWidth = width - 2 * padding
    const chartHeight = height - 2 * padding

    // Clear canvas
    ctx.fillStyle = 'rgba(0, 0, 0, 0.8)'
    ctx.fillRect(0, 0, width, height)

    // Find min/max values for scaling
    const frequencies = data.map(d => d.frequency)
    const values = data.map(d => d.value)
    const minFreq = Math.min(...frequencies)
    const maxFreq = Math.max(...frequencies)
    const minValue = Math.min(...values)
    const maxValue = Math.max(...values)
    
    const freqRange = maxFreq - minFreq || 1
    const valueRange = maxValue - minValue || 1

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

    for (let i = 0; i <= 10; i++) {
      const x = padding + (chartWidth / 10) * i
      ctx.beginPath()
      ctx.moveTo(x, padding)
      ctx.lineTo(x, height - padding)
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

    // Draw data points and line
    if (data.length > 0) {
      ctx.strokeStyle = '#22c55e' // Green
      ctx.lineWidth = 2
      ctx.beginPath()

      data.forEach((point, index) => {
        const x = padding + ((point.frequency - minFreq) / freqRange) * chartWidth
        const y = height - padding - ((point.value - minValue) / valueRange) * chartHeight

        if (index === 0) {
          ctx.moveTo(x, y)
        } else {
          ctx.lineTo(x, y)
        }
      })

      ctx.stroke()

      // Draw data points
      ctx.fillStyle = '#22c55e'
      data.forEach(point => {
        const x = padding + ((point.frequency - minFreq) / freqRange) * chartWidth
        const y = height - padding - ((point.value - minValue) / valueRange) * chartHeight
        ctx.beginPath()
        ctx.arc(x, y, 3, 0, 2 * Math.PI)
        ctx.fill()
      })
    }

    // Draw labels
    ctx.fillStyle = 'rgba(255, 255, 255, 0.8)'
    ctx.font = '14px Arial'
    ctx.textAlign = 'center'
    ctx.fillText('Frequency (KHz)', width / 2, height - 10)

    ctx.save()
    ctx.translate(20, height / 2)
    ctx.rotate(-Math.PI / 2)
    ctx.fillText('Value', 0, 0)
    ctx.restore()

    // Draw frequency labels
    ctx.fillStyle = 'rgba(255, 255, 255, 0.6)'
    ctx.font = '10px Arial'
    ctx.textAlign = 'center'
    for (let i = 0; i <= 5; i++) {
      const freq = minFreq + (freqRange / 5) * i
      const x = padding + (chartWidth / 5) * i
      ctx.fillText(freq.toFixed(0), x, height - 20)
    }

    // Draw value labels
    ctx.textAlign = 'right'
    for (let i = 0; i <= 5; i++) {
      const value = minValue + (valueRange / 5) * i
      const y = height - padding - (chartHeight / 5) * i
      ctx.fillText(value.toFixed(1), padding - 10, y + 3)
    }

    // Draw data info
    ctx.fillStyle = 'rgba(255, 255, 255, 0.8)'
    ctx.font = '12px Arial'
    ctx.textAlign = 'left'
    ctx.fillText(`Data Points: ${data.length}`, padding, padding + 20)
    if (data.length > 0) {
      const latestPoint = data[data.length - 1]
      ctx.fillText(`Latest: ${latestPoint.frequency}Hz, ${latestPoint.value}`, padding, padding + 40)
    }

  }, [data])

  return (
    <div className="frequency-sweep-graph">
      <h4>Frequency Sweep Results</h4>
      <canvas 
        ref={canvasRef}
        className="frequency-sweep-canvas"
        style={{ width: '100%', height: '400px' }}
      />
    </div>
  )
}
