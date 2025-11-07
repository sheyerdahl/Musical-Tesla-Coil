import React from 'react'

export interface SliderProps {
  min: number
  max: number
  value: number
  onChange: (value: number) => void
  label?: string
  step?: number
  disabled?: boolean
}

export function Slider({ min, max, value, onChange, label, step = 1, disabled = false }: SliderProps) {
  const handleChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    onChange(Number(event.target.value))
  }

  return (
    <div className="slider-container">
      {label && <label className="slider-label">{label}</label>}
      <div className="slider-wrapper">
        <input
          type="range"
          min={min}
          max={max}
          value={value}
          step={step}
          onChange={handleChange}
          disabled={disabled}
          className="slider"
        />
        <div className="slider-value">{value}</div>
      </div>
    </div>
  )
}

export default Slider


