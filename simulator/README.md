# simulator/

C# .NET gRPC simulator service for data streaming and replay.

## Run

```bash
dotnet run --project simulator/RoboHealth.Simulator
```

Default endpoints:
- gRPC: `h2c://localhost:50051`
- REST: `http://localhost:5000`

REST endpoints:
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
- `GET /api/exports/alerts`
- `GET /api/exports/logs`
- `GET /api/reports`
- `GET /api/exports/reports`
- `GET /api/exports/reportPdf`
- `GET /api/diagnostics`
- `GET /api/replay`
