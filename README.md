# RoboHealth Studio

RoboHealth Studio is a Qt 6 desktop app for monitoring industrial robots and edge devices. It combines a rich QML UI with a clean C++ MVVM layer and a .NET simulator backend (REST + gRPC) so the full experience can run without real hardware.

## Why it stands out (interview focus)
- End-to-end delivery: UI + business logic + simulated backend + exports
- Clear architecture boundaries (QML UI / C++ ViewModels / service adapters)
- Realistic data flow (REST polling, ack/mute actions, reports & diagnostics)
- Production-minded UX: filters, detail panels, empty states, exports

## Features
- Live dashboard with health KPIs and trend charts
- Device management (connect/disconnect/simulate faults)
- Alerts center with acknowledge/mute actions
- Logs explorer with filters and contextual detail
- Diagnostics and replay views with mock analytics
- Reports with CSV + mock PDF export
- REST + gRPC simulator to generate realistic data

## Screenshot

![Dashboard](docs/screenshots/dashboard.png)

## Architecture

```
QML UI  ->  C++ ViewModels  ->  Telemetry Service (Mock or REST)
                                      |
                                      v
                            .NET Simulator (REST + gRPC)
```

## Tech Stack
- Qt 6 (QML, Widgets, Charts, Network)
- C++17 (MVVM-style ViewModels)
- .NET 8 (Simulator service)
- CMake

## Quick Start

### Prerequisites
- Qt 6.10+
- CMake 3.20+
- .NET 8 SDK

### Build

```bash
cmake -S . -B build
cmake --build build
```

### Run (Mock backend)

```bash
./build/app/robohealth-studio
```

### Run (Simulator REST backend)

```bash
dotnet run --project simulator/RoboHealth.Simulator
ROBOHEALTH_BACKEND=rest ./build/app/robohealth-studio
```

Override API base URL if needed:

```bash
ROBOHEALTH_API_URL=http://127.0.0.1:5000 ROBOHEALTH_BACKEND=rest ./build/app/robohealth-studio
```

## Simulator REST Endpoints
- `GET /api/health`
- `GET /api/devices`
- `POST /api/devices/connectAll`
- `POST /api/devices/disconnectAll`
- `POST /api/devices/{id}/disconnect`
- `POST /api/devices/{id}/simulateFault`
- `GET /api/metrics`
- `GET /api/alerts`
- `POST /api/alerts/{id}/ack`
- `POST /api/alerts/{id}/mute`
- `GET /api/logs`
- `GET /api/reports`
- `GET /api/diagnostics`
- `GET /api/replay`
- `GET /api/exports/alerts`
- `GET /api/exports/logs`
- `GET /api/exports/reports`
- `GET /api/exports/reportPdf`

## Repository Layout
- `app/`            C++ app entry point
- `ui/`             QML pages and UI components
- `presentation/`   ViewModels and UI state
- `domain/`         Business rules and interfaces
- `data/`           Repository implementations
- `integration/`    Device adapters (Mock/REST)
- `infrastructure/` Logging, config, i18n
- `simulator/`      .NET simulator service
- `docs/`           Screenshots and notes

## Notes
This project is built to demonstrate an end-to-end desktop workflow, with a realistic data pipeline and UX features that resemble production monitoring tools.
