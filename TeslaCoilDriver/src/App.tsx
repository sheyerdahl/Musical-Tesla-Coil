import { useState, useRef, useEffect } from 'react'
import './App.css'
import Slider from './Slider'
import BluetoothConnector from './BluetoothConnector'
import SensorGraph from './components/SensorGraph'
import FrequencySweep from './components/FrequencySweep'
import { TeslaCoilBluetooth } from './utils/teslaCoilBluetooth'
import type { TeslaCoilData, TeslaCoilControl, FrequencySweepConfig, FrequencySweepData } from './utils/teslaCoilBluetooth'

function App() {
  const [teslaCoilData, setTeslaCoilData] = useState<TeslaCoilData | null>(null)
  const [teslaCoilControl, setTeslaCoilControl] = useState<TeslaCoilControl>({
    toggle: false,
    burstLength: 100,
    bps: 10,
    phaseLead: 0,
    reverseBurstPhase: false,
    burstEnabled: false
  })
  const [frequencySweepData, setFrequencySweepData] = useState<FrequencySweepData[]>([])
  const [isFrequencySweepActive, setIsFrequencySweepActive] = useState(false)
  const teslaCoilRef = useRef<TeslaCoilBluetooth | null>(null)
  const readIntervalStartedRef = useRef<boolean>(false)

  useEffect(() => {
    if (readIntervalStartedRef.current) { return; }
    readIntervalStartedRef.current = true

    // Load saved control state from localStorage
    try {
      const saved = localStorage.getItem('teslaCoilControl')
      if (saved) {
        const parsed = JSON.parse(saved)
        // Basic shape validation and defaults
        const loadedControl: TeslaCoilControl = {
          toggle: !!parsed.toggle,
          burstLength: Number(parsed.burstLength) || 100,
          bps: Number(parsed.bps) || 10,
          phaseLead: Number(parsed.phaseLead) || 0,
          reverseBurstPhase: !!parsed.reverseBurstPhase,
          burstEnabled: !!parsed.burstEnabled,
        }
        setTeslaCoilControl(loadedControl)
      }
    } catch (e) {
      console.warn('Failed to load teslaCoilControl from localStorage')
    }
    setInterval(async () => {
      const sensorData = await teslaCoilRef.current?.readSensorData()
      if (sensorData == undefined) {
        //console.warn("Failed to get sensor data")
        return
      }
      setTeslaCoilData(sensorData)
    }, 500)
  }, [])

  // Persist control state every second
  useEffect(() => {
    const id = setInterval(() => {
      try {
        localStorage.setItem('teslaCoilControl', JSON.stringify(teslaCoilControl))
      } catch (e) {
        // ignore storage errors
      }
    }, 1000)
    return () => clearInterval(id)
  }, [teslaCoilControl])

  const handleBluetoothConnected = async (device: BluetoothDevice, server: BluetoothRemoteGATTServer) => {
    console.log('Connected')
    console.log(device)
    console.log(server)
    
    try {
      // Initialize Tesla Coil Bluetooth
      const teslaCoil = new TeslaCoilBluetooth(server)
      await teslaCoil.initialize()
      teslaCoilRef.current = teslaCoil
      
      // Start reading sensor data via notifications
      // await teslaCoil.startNotifications(setTeslaCoilData)

      // Send initial control data
      await teslaCoil.writeControlData(teslaCoilControl)
      
      console.log('Tesla Coil Bluetooth initialized successfully')
    } catch (error) {
      console.error('Failed to initialize Tesla Coil Bluetooth:', error)
    }
  }

  const handleBluetoothDisconnected = () => {
    console.log('Disconnected')
    teslaCoilRef.current = null
    setTeslaCoilData(null)
  }

  const updateTeslaCoilControl = async (newControl: Partial<TeslaCoilControl>) => {
    const updatedControl = { ...teslaCoilControl, ...newControl }
    setTeslaCoilControl(updatedControl)
    
    if (teslaCoilRef.current) {
      try {
        await teslaCoilRef.current.writeControlData(updatedControl)
      } catch (error) {
        console.error('Failed to write control data:', error)
      }
    }
  }

  const handleBurstToggle = async () => {
    const newBurstEnabled = !teslaCoilControl.burstEnabled
    await updateTeslaCoilControl({ burstEnabled: newBurstEnabled })
  }

  const handleFrequencySweepConfigChange = async (config: FrequencySweepConfig) => {
    if (teslaCoilRef.current) {
      try {
        await teslaCoilRef.current.writeFrequencySweepConfig(config)
      } catch (error) {
        console.error('Failed to write frequency sweep config:', error)
      }
    }
  }

  const handleStartFrequencySweep = async () => {
    if (teslaCoilRef.current && !isFrequencySweepActive) {
      try {
        // Clear previous data
        setFrequencySweepData([])
        setIsFrequencySweepActive(true)
        
        // Start the frequency sweep
        await teslaCoilRef.current.startFrequencySweep()
        
        // Start frequency sweep notifications
        let frequencyDataSetTimestamp = Date.now()
        await teslaCoilRef.current.startFrequencySweepNotifications((data) => {
          frequencyDataSetTimestamp = Date.now()
          setFrequencySweepData(prev => [...prev, data])
        })
        
        // If no data is received after 2 seconds, stop the FrequencySweep.
        const interval = setInterval(async () => {
          if (Date.now() - frequencyDataSetTimestamp < 2000) { return }
          
          if (teslaCoilRef.current) {
            await teslaCoilRef.current.stopFrequencySweepNotifications()
          }
          setIsFrequencySweepActive(false)
          clearInterval(interval)
        }, 100)
        
      } catch (error) {
        console.error('Failed to start frequency sweep:', error)
        setIsFrequencySweepActive(false)
      }
    }
  }



  return (
    <>
      {/* <div>
        <a href="https://vite.dev" target="_blank">
          <img src={viteLogo} className="logo" alt="Vite logo" />
        </a>
        <a href="https://react.dev" target="_blank">
          <img src={reactLogo} className="logo react" alt="React logo" />
        </a>
      </div>
      <h1>Tesla Coil Driver</h1>
      
      <div className="card">
        <button onClick={() => setCount((count) => count + 1)}>
          count is {count}
        </button>
        <p>
          Edit <code>src/App.tsx</code> and save to test HMR
        </p>
      </div> */}

      <div className="controls-section">
        <h2>Control Panel</h2>
        
        <BluetoothConnector 
          optionalServiceUuids={['08160660-e062-460c-8834-06f539975761', '08160661-e062-460c-8834-06f539975761']}
          filters={[{services: ['08160660-e062-460c-8834-06f539975761']}]}
          onConnected={handleBluetoothConnected}
          onDisconnected={handleBluetoothDisconnected}
        />

        {/* Tesla Coil Controls */}
        <div className="tesla-controls">
          <h3>Tesla Coil Control</h3>
          
          <div className="control-row">
            <button 
              className={`toggle-button ${teslaCoilControl.toggle ? 'active' : ''}`}
              onClick={() => updateTeslaCoilControl({ toggle: !teslaCoilControl.toggle })}
            >
              {teslaCoilControl.toggle ? 'ON' : 'OFF'}
            </button>
            <button 
              className={`toggle-button ${teslaCoilControl.burstEnabled ? 'active' : ''}`}
              onClick={handleBurstToggle}
            >
              {teslaCoilControl.burstEnabled ? 'Bursts ON' : 'Bursts OFF'}
            </button>
            <button 
              className={`toggle-button ${teslaCoilControl.reverseBurstPhase ? 'active' : ''}`}
              onClick={() => updateTeslaCoilControl({ reverseBurstPhase: !teslaCoilControl.reverseBurstPhase })}
            >
              {teslaCoilControl.reverseBurstPhase ? 'Reverse Phase ON' : 'Reverse Phase OFF'}
            </button>
          </div>

          <Slider
            label="Burst Length (us)"
            min={1}
            max={200}
            value={teslaCoilControl.burstLength}
            onChange={(value) => updateTeslaCoilControl({ burstLength: value })}
            step={1}
          />

          <Slider
            label="Bursts Per Second"
            min={1}
            max={10}
            value={teslaCoilControl.bps}
            onChange={(value) => updateTeslaCoilControl({ bps: value })}
            step={1}
          />

          <Slider
            label="Phase Lead (ns)"
            min={0}
            max={1250}
            value={teslaCoilControl.phaseLead}
            onChange={(value) => updateTeslaCoilControl({ phaseLead: value })}
            step={1}
          />
        </div>

        {/* Tesla Coil Sensor Data */}
        {teslaCoilData && (
          <div className="sensor-data">
            <h3>Sensor Readings</h3>
            <div className="sensor-grid">
              <div className="sensor-item">
                <span className="sensor-label">VBus:</span>
                <span className="sensor-value">{teslaCoilData.vbus.toFixed(2)} V</span>
              </div>
              <div className="sensor-item">
                <span className="sensor-label">Current:</span>
                <span className="sensor-value">{teslaCoilData.currentTransformer.toFixed(2)} A</span>
              </div>
              <div className="sensor-item">
                <span className="sensor-label">Therm1:</span>
                <span className="sensor-value">{teslaCoilData.therm1.toFixed(1)} °C</span>
              </div>
              <div className="sensor-item">
                <span className="sensor-label">Therm2:</span>
                <span className="sensor-value">{teslaCoilData.therm2.toFixed(1)} °C</span>
              </div>
            </div>
          </div>
        )}

        {/* Sensor Graph */}
        <SensorGraph
          data={teslaCoilData}
          disabled={isFrequencySweepActive}
        />

        {/* Frequency Sweep */}
        <FrequencySweep
          onConfigChange={handleFrequencySweepConfigChange}
          onStartSweep={handleStartFrequencySweep}
          sweepData={frequencySweepData}
          isSweepActive={isFrequencySweepActive}
        />
      </div>
    </>
  )
}

export default App
