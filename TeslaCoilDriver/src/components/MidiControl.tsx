import { useState, useRef, useEffect } from 'react'
import type { MidiUploadProgress } from '../utils/teslaCoilBluetooth'

export interface MidiControlProps {
  disabled?: boolean
  onUploadFile: (file: File, onProgress?: (progress: MidiUploadProgress) => void) => Promise<boolean>
  onPlayMidi?: (enabled: boolean) => Promise<void>
}

export default function MidiControl({ disabled, onUploadFile, onPlayMidi }: MidiControlProps) {
  const [file, setFile] = useState<File | null>(null)
  const [isUploading, setIsUploading] = useState(false)
  const [success, setSuccess] = useState<boolean | null>(null)
  const [error, setError] = useState<string | null>(null)
  const [playMidi, setPlayMidi] = useState(false)
  const [uploadProgress, setUploadProgress] = useState<MidiUploadProgress | null>(null)
  const progressTimeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null)

  useEffect(() => {
    return () => {
      if (progressTimeoutRef.current) {
        clearTimeout(progressTimeoutRef.current)
      }
    }
  }, [])

  const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setSuccess(null)
    setError(null)
    if (progressTimeoutRef.current) {
      clearTimeout(progressTimeoutRef.current)
      progressTimeoutRef.current = null
    }
    setUploadProgress(null)
    const f = e.target.files?.[0] ?? null
    setFile(f)
  }

  const handleUpload = async () => {
    if (!file || disabled) return
    // Clear any existing timeout
    if (progressTimeoutRef.current) {
      clearTimeout(progressTimeoutRef.current)
      progressTimeoutRef.current = null
    }
    setIsUploading(true)
    setSuccess(null)
    setError(null)
    setUploadProgress({ totalBytes: 0, bytesSent: 0, percent: 0 })
    try {
      const ok = await onUploadFile(file, (progress) => {
        setUploadProgress(progress)
      })
      setSuccess(ok)
      if (!ok) setError('Upload failed')
    } catch (e) {
      const message = e instanceof Error ? e.message : String(e)
      setError(message)
      setSuccess(false)
    } finally {
      setIsUploading(false)
      // Keep progress visible for a moment after completion, then clear it
      progressTimeoutRef.current = setTimeout(() => {
        setUploadProgress(null)
        progressTimeoutRef.current = null
      }, 2000)
    }
  }

  const handlePlayMidiToggle = async () => {
    if (disabled || !onPlayMidi) return
    const newValue = !playMidi
    setPlayMidi(newValue)
    try {
      await onPlayMidi(newValue)
    } catch (e) {
      const message = e instanceof Error ? e.message : String(e)
      setError(message)
      setPlayMidi(!newValue) // Revert on error
    }
  }

  return (
    <div className="midi-control">
      <h3>Midi Control</h3>
      <div className="midi-row">
        <input
          type="file"
          accept=".mid,.midi,audio/midi,application/x-midi"
          onChange={handleFileChange}
          disabled={disabled || isUploading}
        />
        <button
          className="midi-upload-button"
          onClick={handleUpload}
          disabled={disabled || !file || isUploading}
        >
          {isUploading ? 'Uploading…' : 'Upload'}
        </button>
        <div className="midi-status">
          {success === true && <span className="midi-indicator midi-indicator--ok">Uploaded</span>}
          {success === false && <span className="midi-indicator midi-indicator--error">Failed</span>}
        </div>
      </div>
      {error && <div className="midi-error">{error}</div>}
      {uploadProgress && uploadProgress.totalBytes > 0 && (
        <div className="midi-progress-container">
          <div className="midi-progress-bar">
            <div 
              className="midi-progress-fill" 
              style={{ width: `${uploadProgress.percent}%` }}
            />
          </div>
          <div className="midi-progress-text">
            {uploadProgress.percent}% ({uploadProgress.bytesSent.toLocaleString()} / {uploadProgress.totalBytes.toLocaleString()} bytes)
          </div>
        </div>
      )}
      {onPlayMidi && (
        <div className="midi-row">
          <button
            className={`toggle-button ${playMidi ? 'active' : ''}`}
            onClick={handlePlayMidiToggle}
            disabled={disabled}
          >
            {playMidi ? 'Play Midi ON' : 'Play Midi OFF'}
          </button>
        </div>
      )}
    </div>
  )
}


