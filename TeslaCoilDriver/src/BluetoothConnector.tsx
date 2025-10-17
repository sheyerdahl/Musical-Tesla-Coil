import { useCallback, useEffect, useMemo, useState } from 'react'

type GattServer = BluetoothRemoteGATTServer | null

export interface BluetoothConnectorProps {
  optionalServiceUuids?: Array<BluetoothServiceUUID>
  filters?: Array<BluetoothRequestDeviceFilter>
  onConnected?: (device: BluetoothDevice, server: BluetoothRemoteGATTServer) => void
  onDisconnected?: () => void
}

export default function BluetoothConnector({
  optionalServiceUuids,
  filters,
  onConnected,
  onDisconnected,
}: BluetoothConnectorProps) {
  const supported = typeof navigator !== 'undefined' && !!navigator.bluetooth

  const [device, setDevice] = useState<BluetoothDevice | null>(null)
  const [server, setServer] = useState<GattServer>(null)
  const [isConnecting, setIsConnecting] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const isConnected = !!server?.connected

  const requestOptions = useMemo((): RequestDeviceOptions => {
    if (filters && filters.length > 0) {
      return { filters, optionalServices: optionalServiceUuids }
    }
    return { acceptAllDevices: true, optionalServices: optionalServiceUuids }
  }, [filters, optionalServiceUuids])

  const handleDisconnectEvent = useCallback(() => {
    setServer(null)
    setDevice((prev) => prev)
    onDisconnected?.()
  }, [onDisconnected])

  useEffect(() => {
    if (!device) return
    const listener = () => handleDisconnectEvent()
    device.addEventListener('gattserverdisconnected', listener)
    return () => {
      device.removeEventListener('gattserverdisconnected', listener)
    }
  }, [device, handleDisconnectEvent])

  const connect = useCallback(async () => {
    if (!supported) return
    setIsConnecting(true)
    setError(null)
    try {
      const selected = await navigator.bluetooth.requestDevice(requestOptions)
      setDevice(selected)
      const gatt = await selected.gatt?.connect()
      if (!gatt) throw new Error('Failed to open GATT server')
      setServer(gatt)
      onConnected?.(selected, gatt)
    } catch (err) {
      const message = err instanceof Error ? err.message : String(err)
      setError(message)
    } finally {
      setIsConnecting(false)
    }
  }, [onConnected, requestOptions, supported])

  const disconnect = useCallback(() => {
    try {
      setError(null)
      if (server?.connected) server.disconnect()
      setServer(null)
    } catch (err) {
      const message = err instanceof Error ? err.message : String(err)
      setError(message)
    }
  }, [server])

  if (!supported) {
    return (
      <div className="bt-card">
        <div className="bt-header">
          <h2>Bluetooth</h2>
          <span className="bt-dot bt-dot--error" />
        </div>
        <p>Your browser does not support Web Bluetooth.</p>
        <p className="bt-subtle">Use Chrome/Edge on desktop with HTTPS.</p>
      </div>
    )
  }

  return (
    <div className="bt-card">
      <div className="bt-header">
        <h2>Bluetooth</h2>
        <span className={isConnected ? 'bt-dot bt-dot--ok' : 'bt-dot'} />
      </div>

      <div className="bt-row">
        <button
          className="bt-button"
          onClick={connect}
          disabled={isConnecting || isConnected}
        >
          {isConnecting ? 'Connecting…' : isConnected ? 'Connected' : 'Connect'}
        </button>
        <button className="bt-button bt-button--ghost" onClick={disconnect} disabled={!isConnected}>
          Disconnect
        </button>
      </div>

      <div className="bt-info">
        <div>
          <span className="bt-label">Device:</span>
          <span className="bt-value">{device?.name || '—'}</span>
        </div>
        <div>
          <span className="bt-label">Connected:</span>
          <span className="bt-value">{isConnected ? 'Yes' : 'No'}</span>
        </div>
      </div>

      {error && <div className="bt-error">{error}</div>}
    </div>
  )
}


