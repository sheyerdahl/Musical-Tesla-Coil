// Tesla Coil Bluetooth characteristic UUIDs and operations
// Based on the service UUID: 08160660-e062-460c-8834-06f539975761

export const TESLA_COIL_SERVICE_UUID = '08160660-e062-460c-8834-06f539975761'
export const FREQUENCY_SWEEP_SERVICE_UUID = '08160661-e062-460c-8834-06f539975761'

// Characteristic UUIDs (these would need to be provided by your device manufacturer)
export const CHARACTERISTIC_UUIDS = {
  // Service characteristics
  VBUS: '18160660-e062-460c-8834-06f539975761',           // Read, Bus voltage
  CURRENT_TRANSFORMER: '28160660-e062-460c-8834-06f539975761', // Read, Current transformer reading
  THERM1: '38160660-e062-460c-8834-06f539975761',         // Read, Temperature sensor 1
  THERM2: '48160660-e062-460c-8834-06f539975761',         // Read, Temperature sensor 2
  TOGGLE: '58160660-e062-460c-8834-06f539975761',         // Write, Enable/disable relays
  BURST_LENGTH: '68160660-e062-460c-8834-06f539975761',   // Write, Burst length control
  BPS: '78160660-e062-460c-8834-06f539975761',            // Write, Bursts per second
  BURST_ENABLED: '88160660-e062-460c-8834-06f539975761',  // Write, Enable/disable bursts
  PHASE_LEAD: '98160660-e062-460c-8834-06f539975761',     // Write, Phase lead control (u16)
  REVERSE_BURST_PHASE: 'a8160660-e062-460c-8834-06f539975761', // Write, Reverse burst phase (bool)
  MIDI_UPLOAD: 'b8160660-e062-460c-8834-06f539975761', // Write, Upload MIDI file data (chunked)
  PLAY_MIDI: 'c8160660-e062-460c-8834-06f539975761', // Write, Play MIDI (bool)
  MIDI_OCTAVE: 'd8160660-e062-460c-8834-06f539975761', // Write, MIDI octave (int8)
  CHORD_SWAP_TIME: 'e8160660-e062-460c-8834-06f539975761', // Write, Chord swap time (uint8)
  
  // FrequencySweepService characteristics
  MIN_FREQUENCY_SWEEP: '08160662-e062-460c-8834-06f539975761', // Write, Min frequency for sweep
  MAX_FREQUENCY_SWEEP: '08160663-e062-460c-8834-06f539975761', // Write, Max frequency for sweep
  START_FREQUENCY_SWEEP: '08160664-e062-460c-8834-06f539975761', // Write, Start frequency sweep
  FREQUENCY_SWEEP_DATA: '08160665-e062-460c-8834-06f539975761', // Read, Frequency sweep data (read)
} as const

export interface TeslaCoilData {
  vbus: number
  currentTransformer: number
  therm1: number
  therm2: number
}

export interface TeslaCoilControl {
  toggle: boolean
  burstLength: number
  bps: number
  phaseLead: number
  reverseBurstPhase: boolean
  burstEnabled: boolean
}

export interface FrequencySweepData {
  frequency: number
  value: number
}

export interface FrequencySweepConfig {
  minFrequency: number
  maxFrequency: number
}

// MIDI upload helpers and API
export class MidiUploadError extends Error {}

export interface MidiUploadProgress {
  totalBytes: number
  bytesSent: number
  percent: number
}

export interface MidiUploadOptions {
  chunkSize?: number // bytes per BLE write (typical safe default ~ 128)
  interChunkDelayMs?: number // small pacing delay to avoid overrun
  onProgress?: (progress: MidiUploadProgress) => void
}

// Extend TeslaCoilBluetooth with MIDI upload via prototype to avoid large refactor
// export interface TeslaCoilBluetooth {
//   uploadMidiData(data: ArrayBuffer, options?: MidiUploadOptions): Promise<void>
// }

export class TeslaCoilBluetooth {
  private server: BluetoothRemoteGATTServer | null = null
  private service: BluetoothRemoteGATTService | null = null
  private frequencySweepService: BluetoothRemoteGATTService | null = null
  private characteristics: Map<string, BluetoothRemoteGATTCharacteristic> = new Map()
  private lastControlState: TeslaCoilControl = {toggle: false, burstLength: 0, bps: 0, phaseLead: 0, reverseBurstPhase: false, burstEnabled: false}

  constructor(server: BluetoothRemoteGATTServer) {
    this.server = server
  }

  async initialize(): Promise<void> {
    if (!this.server) throw new Error('No GATT server connected')
    
    try {
      // Get the Tesla Coil service
      this.service = await this.server.getPrimaryService(TESLA_COIL_SERVICE_UUID)
      
      // Get the Frequency Sweep service
      this.frequencySweepService = await this.server.getPrimaryService(FREQUENCY_SWEEP_SERVICE_UUID)
      
      // Get all characteristics from Tesla Coil service
      const teslaCoilChars = ['VBUS', 'CURRENT_TRANSFORMER', 'THERM1', 'THERM2', 'TOGGLE', 'BURST_LENGTH', 'BPS', 'BURST_ENABLED', 'PHASE_LEAD', 'REVERSE_BURST_PHASE', 'MIDI_UPLOAD', 'PLAY_MIDI', 'MIDI_OCTAVE', 'CHORD_SWAP_TIME']
      const teslaCoilCharPromises = teslaCoilChars.map(async (name) => {
        const uuid = CHARACTERISTIC_UUIDS[name as keyof typeof CHARACTERISTIC_UUIDS]
        try {
          const char = await this.service!.getCharacteristic(uuid)
          this.characteristics.set(name, char)
          console.log(`✓ Tesla Coil Characteristic ${name} (${uuid}) ready`)
        } catch (error) {
          console.warn(`⚠ Failed to get Tesla Coil characteristic ${name} (${uuid}):`, error)
        }
      })
      
      // Get all characteristics from Frequency Sweep service
      const frequencySweepChars = ['FREQUENCY_SWEEP_DATA', 'MIN_FREQUENCY_SWEEP', 'MAX_FREQUENCY_SWEEP', 'START_FREQUENCY_SWEEP']
      const frequencySweepCharPromises = frequencySweepChars.map(async (name) => {
        const uuid = CHARACTERISTIC_UUIDS[name as keyof typeof CHARACTERISTIC_UUIDS]
        try {
          const char = await this.frequencySweepService!.getCharacteristic(uuid)
          this.characteristics.set(name, char)
          console.log(`✓ Frequency Sweep Characteristic ${name} (${uuid}) ready`)
        } catch (error) {
          console.warn(`⚠ Failed to get Frequency Sweep characteristic ${name} (${uuid}):`, error)
        }
      })
      
      await Promise.all([...teslaCoilCharPromises, ...frequencySweepCharPromises])
    } catch (error) {
      console.error('Failed to initialize Tesla Coil Bluetooth:', error)
      throw error
    }
  }

  async readSensorData(): Promise<TeslaCoilData | undefined> {
    const data: TeslaCoilData = {
      vbus: 0,
      currentTransformer: 0,
      therm1: 0,
      therm2: 0
    }

    try {
      // Read VBus
      const vbusChar = this.characteristics.get('VBUS')
      if (vbusChar) {
        const vbusValue = await vbusChar.readValue()
        data.vbus = this.parseFloat32(vbusValue)
        //console.log('VBus:', data.vbus, 'V')
      }

      // Read Current Transformer
      const currentChar = this.characteristics.get('CURRENT_TRANSFORMER')
      if (currentChar) {
        const currentValue = await currentChar.readValue()
        data.currentTransformer = this.parseFloat32(currentValue)
        //console.log('Current Transformer:', data.currentTransformer, 'A')
      }

      // Read Therm1
      const therm1Char = this.characteristics.get('THERM1')
      if (therm1Char) {
        const therm1Value = await therm1Char.readValue()
        data.therm1 = this.parseFloat32(therm1Value)
        //console.log('Therm1:', data.therm1, '°C')
      }

      // Read Therm2
      const therm2Char = this.characteristics.get('THERM2')
      if (therm2Char) {
        const therm2Value = await therm2Char.readValue()
        data.therm2 = this.parseFloat32(therm2Value)
        //console.log('Therm2:', data.therm2, '°C')
      }

    } catch (error) {
      console.error('Error reading sensor data:', error)
      return undefined
    }

    return data
  }

  async writeControlData(control: TeslaCoilControl): Promise<void> {
    try {
      // Only write values that have changed
      // Write Toggle only if changed
      if (this.lastControlState.toggle !== control.toggle) {
        const toggleChar = this.characteristics.get('TOGGLE')
        if (toggleChar) {
          const toggleValue = new Uint8Array([control.toggle ? 1 : 0])
          await toggleChar.writeValue(toggleValue)
          console.log('Toggle:', control.toggle)
        }
      }

      // Write Burst Length only if changed
      if (this.lastControlState.burstLength !== control.burstLength) {
        const burstLengthChar = this.characteristics.get('BURST_LENGTH')
        if (burstLengthChar) {
          const burstLengthValue = new Uint16Array([control.burstLength])
          await burstLengthChar.writeValue(burstLengthValue)
          console.log('Burst Length:', control.burstLength)
        }
      }

      // Write Burst Enabled only if changed
      if (this.lastControlState.burstEnabled !== control.burstEnabled) {
        const burstEnabledChar = this.characteristics.get('BURST_ENABLED')
        if (burstEnabledChar) {
          const burstEnabledValue = new Uint8Array([control.burstEnabled ? 1 : 0])
          await burstEnabledChar.writeValue(burstEnabledValue)
          console.log('Burst Enabled:', control.burstEnabled)
        }
      }

      // Write BPS only if changed
      if (this.lastControlState.bps !== control.bps) {
        const bpsChar = this.characteristics.get('BPS')
        if (bpsChar) {
          const bpsValue = new Uint16Array([control.bps])
          await bpsChar.writeValue(bpsValue)
          console.log('BPS:', control.bps)
        }
      }

      // Write Phase Lead only if changed
      if (this.lastControlState.phaseLead !== control.phaseLead) {
        const phaseLeadChar = this.characteristics.get('PHASE_LEAD')
        if (phaseLeadChar) {
          const phaseLeadValue = new Uint16Array([control.phaseLead])
          await phaseLeadChar.writeValue(phaseLeadValue)
          console.log('Phase Lead:', control.phaseLead)
        }
      }

      // Write Reverse Burst Phase only if changed
      if (this.lastControlState.reverseBurstPhase !== control.reverseBurstPhase) {
        const reverseBurstPhaseChar = this.characteristics.get('REVERSE_BURST_PHASE')
        if (reverseBurstPhaseChar) {
          const reverseBurstPhaseValue = new Uint8Array([control.reverseBurstPhase ? 1 : 0])
          await reverseBurstPhaseChar.writeValue(reverseBurstPhaseValue)
          console.log('Reverse Burst Phase:', control.reverseBurstPhase)
        }
      }
      // Update the last known state
      this.lastControlState = { ...control }
    } catch (error) {
      console.error('Error writing control data:', error)
      throw error
    }
  }

  async writeBurstEnabled(enabled: boolean): Promise<void> {
    try {
      const burstEnabledChar = this.characteristics.get('BURST_ENABLED')
      if (burstEnabledChar) {
        const burstEnabledValue = new Uint8Array([enabled ? 1 : 0])
        await burstEnabledChar.writeValue(burstEnabledValue)
        console.log('Burst Enabled:', enabled)
      }
    } catch (error) {
      console.error('Error writing burst enabled state:', error)
      throw error
    }
  }

  async writeReverseBurstPhase(enabled: boolean): Promise<void> {
    try {
      const reverseBurstPhaseChar = this.characteristics.get('REVERSE_BURST_PHASE')
      if (reverseBurstPhaseChar) {
        const reverseBurstPhaseValue = new Uint8Array([enabled ? 1 : 0])
        await reverseBurstPhaseChar.writeValue(reverseBurstPhaseValue)
        console.log('Reverse Burst Phase:', enabled)
      }
    } catch (error) {
      console.error('Error writing reverse burst phase state:', error)
      throw error
    }
  }

  private parseFloat32(dataView: DataView): number {
    return dataView.getFloat32(0, true) // little-endian
  }

  async startNotifications(setTeslaCoilData: React.Dispatch<React.SetStateAction<TeslaCoilData | null>>): Promise<void> {
    // Set up notifications for sensor data if supported
    const notificationChars = ['VBUS', 'CURRENT_TRANSFORMER', 'THERM1', 'THERM2']
    
    // Track current sensor data
    let currentData: TeslaCoilData = {
      vbus: 0,
      currentTransformer: 0,
      therm1: 0,
      therm2: 0
    }
    
    for (const charName of notificationChars) {
      const char = this.characteristics.get(charName)
      if (char && char.properties.notify) {
        console.log("Trying to start notifications for: ", charName)
        try {
          await char.startNotifications()
          //await this._startNotifications(char)
          console.log("Setting up notifications for: ", charName)
          char.addEventListener('characteristicvaluechanged', (event) => {
            const characteristic = event.target as BluetoothRemoteGATTCharacteristic
            const value = characteristic.value
            if (value) {
              const floatValue = this.parseFloat32(value)
              console.log(`${charName} notification:`, floatValue)
              
              // Update the corresponding sensor value
              switch (charName) {
                case 'VBUS':
                  currentData.vbus = floatValue
                  break
                case 'CURRENT_TRANSFORMER':
                  currentData.currentTransformer = floatValue
                  break
                case 'THERM1':
                  currentData.therm1 = floatValue
                  break
                case 'THERM2':
                  currentData.therm2 = floatValue
                  break
              }
              
              // Update the state with the current data
              setTeslaCoilData({ ...currentData })
            }
          })
          console.log(`✓ ${charName} notifications started`)
        } catch (error) {
          console.warn(`⚠ Failed to start notifications for ${charName}:`, error)
        }
      }
    }
  }

  async stopNotifications(): Promise<void> {
    const notificationChars = ['VBUS', 'CURRENT_TRANSFORMER', 'THERM1', 'THERM2']
    
    for (const charName of notificationChars) {
      const char = this.characteristics.get(charName)
      if (char && char.properties.notify) {
        try {
          await char.stopNotifications()
          console.log(`✓ ${charName} notifications stopped`)
        } catch (error) {
          console.warn(`⚠ Failed to stop notifications for ${charName}:`, error)
        }
      }
    }
  }

  async writeFrequencySweepConfig(config: FrequencySweepConfig): Promise<void> {
    try {
      // Write min frequency
      const minFreqChar = this.characteristics.get('MIN_FREQUENCY_SWEEP')
      if (minFreqChar) {
        const minFreqValue = new Uint16Array([config.minFrequency])
        await minFreqChar.writeValue(minFreqValue)
        console.log('Min Frequency:', config.minFrequency)
      }

      // Write max frequency
      const maxFreqChar = this.characteristics.get('MAX_FREQUENCY_SWEEP')
      if (maxFreqChar) {
        const maxFreqValue = new Uint16Array([config.maxFrequency])
        await maxFreqChar.writeValue(maxFreqValue)
        console.log('Max Frequency:', config.maxFrequency)
      }
    } catch (error) {
      console.error('Error writing frequency sweep config:', error)
      throw error
    }
  }

  async startFrequencySweep(): Promise<void> {
    try {
      const startChar = this.characteristics.get('START_FREQUENCY_SWEEP')
      if (startChar) {
        const startValue = new Uint8Array([1]) // Trigger start
        await startChar.writeValue(startValue)
        console.log('Frequency sweep started')
      }
    } catch (error) {
      console.error('Error starting frequency sweep:', error)
      throw error
    }
  }

  async readFrequencySweepData(): Promise<FrequencySweepData | undefined> {
    try {
      const dataChar = this.characteristics.get('FREQUENCY_SWEEP_DATA')
      if (dataChar) {
        const value = await dataChar.readValue()
        if (value.byteLength >= 4) {
          // First 16 bits are frequency, last 16 bits are value
          const frequency = value.getUint16(0, true) // little-endian
          const dataValue = value.getUint16(2, true) // little-endian
          return { frequency, value: dataValue }
        }
      }
    } catch (error) {
      console.error('Error reading frequency sweep data:', error)
      return undefined
    }
    return undefined
  }

  async startFrequencySweepNotifications(
    onData: (data: FrequencySweepData) => void
  ): Promise<void> {
    const dataChar = this.characteristics.get('FREQUENCY_SWEEP_DATA')
    if (dataChar && dataChar.properties.notify) {
      try {
        await dataChar.startNotifications()
        dataChar.addEventListener('characteristicvaluechanged', (event) => {
          const characteristic = event.target as BluetoothRemoteGATTCharacteristic
          const value = characteristic.value
          if (value && value.byteLength >= 4) {
            const frequency = value.getUint16(2, true) // little-endian
            const dataValue = value.getUint16(0, true) // little-endian
            onData({ frequency, value: dataValue })
          }
        })
        console.log('✓ Frequency sweep data notifications started')
      } catch (error) {
        console.warn('⚠ Failed to start frequency sweep notifications:', error)
      }
    }
  }

  async stopFrequencySweepNotifications(): Promise<void> {
    const dataChar = this.characteristics.get('FREQUENCY_SWEEP_DATA')
    if (dataChar && dataChar.properties.notify) {
      try {
        await dataChar.stopNotifications()
        console.log('✓ Frequency sweep data notifications stopped')
      } catch (error) {
        console.warn('⚠ Failed to stop frequency sweep notifications:', error)
      }
    }
  }

  async disconnect(): Promise<void> {
    try {
      // Stop all notifications
      await this.stopNotifications()
      await this.stopFrequencySweepNotifications()
      
      // Disconnect from server
      if (this.server && this.server.connected) {
        await this.server.disconnect()
        console.log('✓ Disconnected from Tesla Coil device')
      }
    } catch (error) {
      console.error('Error disconnecting:', error)
    }
  }

  async uploadMidiData(data: ArrayBuffer, options?: MidiUploadOptions): Promise<void> {
    const midiChar = this.characteristics.get('MIDI_UPLOAD')
    if (!midiChar) {
      throw new MidiUploadError('MIDI_UPLOAD characteristic not available')
    }

    const chunkSize = Math.max(20, Math.min(options?.chunkSize ?? 128, 512))
    const delayMs = Math.max(0, options?.interChunkDelayMs ?? 10)

    const totalBytes = data.byteLength
    let bytesSent = 0
    const view = new Uint8Array(data)

    const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms))

    while (bytesSent < totalBytes) {
      const end = Math.min(bytesSent + chunkSize, totalBytes)
      const chunk = view.slice(bytesSent, end)
      try {
        await midiChar.writeValue(chunk)
      } catch (err) {
        console.error('MIDI upload write failed at', bytesSent, 'bytes')
        throw new MidiUploadError(
          err instanceof Error ? err.message : String(err)
        )
      }
      bytesSent = end
      options?.onProgress?.({
        totalBytes,
        bytesSent,
        percent: totalBytes ? Math.round((bytesSent / totalBytes) * 100) : 100,
      })
      if (delayMs > 0) await sleep(delayMs)
    }

    // Write empty value to signal upload completion
    try {
      const emptyValue = new Uint8Array([])
      await midiChar.writeValue(emptyValue)
      console.log('MIDI upload completed - sent empty value to signal completion')
    } catch (err) {
      console.error('Failed to write empty value after MIDI upload:', err)
      throw new MidiUploadError(
        err instanceof Error ? err.message : String(err)
      )
    }
  }

  async writePlayMidi(enabled: boolean): Promise<void> {
    try {
      const playMidiChar = this.characteristics.get('PLAY_MIDI')
      if (playMidiChar) {
        const playMidiValue = new Uint8Array([enabled ? 1 : 0])
        await playMidiChar.writeValue(playMidiValue)
        console.log('Play MIDI:', enabled)
      }
    } catch (error) {
      console.error('Error writing play MIDI state:', error)
      throw error
    }
  }

  async writeMidiOctave(octave: number): Promise<void> {
    try {
      const midiOctaveChar = this.characteristics.get('MIDI_OCTAVE')
      if (midiOctaveChar) {
        // Clamp octave to int8 range (-128 to 127), but we'll use -3 to 3 as per requirement
        const clampedOctave = Math.max(-3, Math.min(3, Math.round(octave)))
        // Convert to signed int8
        const octaveValue = new Int8Array([clampedOctave])
        await midiOctaveChar.writeValue(octaveValue)
        console.log('MIDI Octave:', clampedOctave)
      }
    } catch (error) {
      console.error('Error writing MIDI octave:', error)
      throw error
    }
  }

  async writeChordSwapTime(time: number): Promise<void> {
    try {
      const chordSwapTimeChar = this.characteristics.get('CHORD_SWAP_TIME')
      if (chordSwapTimeChar) {
        // Clamp to uint8 range (1 to 100 as per requirement)
        const clampedTime = Math.max(1, Math.min(100, Math.round(time)))
        // Convert to uint8
        const timeValue = new Uint8Array([clampedTime])
        await chordSwapTimeChar.writeValue(timeValue)
        console.log('Chord Swap Time:', clampedTime)
      }
    } catch (error) {
      console.error('Error writing chord swap time:', error)
      throw error
    }
  }
}
