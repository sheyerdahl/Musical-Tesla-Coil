import { useState } from 'react'
import Slider from '../Slider'
import FrequencySweepGraph from './FrequencySweepGraph.tsx'
import type { FrequencySweepConfig, FrequencySweepData } from '../utils/teslaCoilBluetooth'

interface FrequencySweepProps {
  onConfigChange: (config: FrequencySweepConfig) => void
  onStartSweep: () => void
  sweepData: FrequencySweepData[]
  isSweepActive: boolean
}

export default function FrequencySweep({ 
  onConfigChange, 
  onStartSweep, 
  sweepData, 
  isSweepActive 
}: FrequencySweepProps) {
  const [config, setConfig] = useState<FrequencySweepConfig>({
    minFrequency: 80,
    maxFrequency: 150
  })

  const handleConfigChange = (field: keyof FrequencySweepConfig, value: number) => {
    const newConfig = { ...config, [field]: value }
    setConfig(newConfig)
    onConfigChange(newConfig)
  }

  return (
    <div className="frequency-sweep-section">
      <h3>Frequency Sweep</h3>
      
      <div className="frequency-controls">
        <Slider
          label="Min Frequency (KHz)"
          min={1}
          max={300}
          value={config.minFrequency}
          onChange={(value) => handleConfigChange('minFrequency', value)}
          step={1}
        />

        <Slider
          label="Max Frequency (KHz)"
          min={1}
          max={300}
          value={config.maxFrequency}
          onChange={(value) => handleConfigChange('maxFrequency', value)}
          step={1}
        />

        <div className="sweep-controls">
          <button 
            className={`sweep-button ${isSweepActive ? 'active' : ''}`}
            onClick={onStartSweep}
            disabled={isSweepActive}
          >
            {isSweepActive ? 'Sweep Running...' : 'Start Frequency Sweep'}
          </button>
        </div>
      </div>

      {sweepData.length > 0 && (
        <FrequencySweepGraph data={sweepData} />
      )}
    </div>
  )
}
