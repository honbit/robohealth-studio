# RoboHealth Studio — UI Wireframe Notes

## Global Layout
- **Left Sidebar**: Dashboard / Devices / Alerts / Logs / Replay / Diagnostics / Reports / Settings
- **Top Bar**: Project name, environment badge (Simulated), language toggle (中/EN), time range selector
- **Main Area**: Page content
- **Style**: Light, clean, orange accent, subtle shadows

## Dashboard
- **Row 1 (KPI Cards)**: Temperature / Vibration / Current / Torque / Error Rate
- **Row 2 (Charts + Status)**
  - Left: TrendChart (select device + metric)
  - Right: DeviceStatusList (online/offline + alerts count)
- **Row 3 (Alerts + Timeline)**
  - Left: Recent Alerts list
  - Right: Event Timeline

## Devices
- **Top Filter**: Search, type filter, status filter
- **Main Table**: Device list (name/type/status/last seen)
- **Detail Panel**: Selected device detail (health score, live metrics, mini-chart)
- **Actions**: Connect/Disconnect, Simulate Fault

## Alerts
- **Filter Bar**: Level, device, time range
- **Alert List**: Table with severity badges
- **Detail Drawer**: Cause, evidence, recommended action

## Logs
- **Search Bar**: keyword + time range
- **Log Stream**: scrollable list (color by severity)
- **Detail Panel**: full log context

## Replay
- **Timeline Control**: play/pause/seek/speed
- **Main Chart**: metric playback
- **Snapshot Panel**: alert markers + event notes

## Diagnostics
- **Diagnosis Cards**: symptom, cause, evidence, action, confidence
- **Evidence Chart**: shows anomaly spike
- **Action Checklist**: suggested steps

## Reports
- **Report List**: daily/weekly health summary
- **Preview Panel**: snapshot
- **Export**: PDF / Markdown

## Settings
- **Language**: 中/EN
- **Theme**: light (future: dark)
- **Simulator Config**: device count, fault rate, refresh rate
- **Thresholds**: alert rules

## Components to Build First
- Sidebar
- KPI Card
- TrendChart Placeholder
- AlertList Placeholder
- Timeline Placeholder
- DeviceStatusList Placeholder
