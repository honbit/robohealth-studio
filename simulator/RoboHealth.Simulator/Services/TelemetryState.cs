using System.Text;
using RoboHealth.Simulator;

namespace RoboHealth.Simulator.Services;

public sealed record DeviceView(string Id, string Name, string Type, string Status, int Health, string LastSeen);
public sealed record AlertView(string Id, string Level, string Message, string DeviceId, string DeviceName, string Time, string Evidence, string Action, bool Acknowledged, bool Muted);
public sealed record LogView(string Id, string Level, string Message, string DeviceId, string DeviceName, string Time);
public sealed record MetricsSnapshot(double Temperature, double Vibration, double Current, double Torque, double ErrorRate, List<double> TrendPoints);
public sealed record ReportView(string Title, string Time, string Summary);
public sealed record DiagnosticCardView(string Symptom, string Cause, string Evidence, string Action, double Confidence);
public sealed record DiagnosticsViewResponse(List<DiagnosticCardView> Cards, List<double> Evidence, List<string> Checklist);
public sealed record ReplayEventView(string Time, string Text);
public sealed record ReplayViewResponse(int DurationSeconds, List<ReplayEventView> Events, List<double> TrendPoints);

internal sealed class DeviceInfo
{
    public DeviceInfo(string id, string name, string type, string status, int health)
    {
        Id = id;
        Name = name;
        Type = type;
        Status = status;
        Health = health;
        LastSeen = DateTimeOffset.UtcNow;
    }

    public string Id { get; }
    public string Name { get; }
    public string Type { get; }
    public string Status { get; set; }
    public int Health { get; set; }
    public DateTimeOffset LastSeen { get; set; }
}

internal sealed class AlertRecord
{
    public required string Id { get; init; }
    public required string Level { get; init; }
    public required string Message { get; init; }
    public required string DeviceId { get; init; }
    public required string DeviceName { get; init; }
    public required string Evidence { get; init; }
    public required string Action { get; init; }
    public required long TsUnixMs { get; init; }
    public bool Acknowledged { get; set; }
    public bool Muted { get; set; }
}

internal sealed class LogRecord
{
    public required string Id { get; init; }
    public required string Level { get; init; }
    public required string Message { get; init; }
    public required string DeviceId { get; init; }
    public required string DeviceName { get; init; }
    public required long TsUnixMs { get; init; }
}

public static class TelemetryState
{
    private static readonly object Sync = new();
    private static readonly List<DeviceInfo> Devices = new()
    {
        new DeviceInfo("rb-01", "Robot-01", "Robot", "Online", 92),
        new DeviceInfo("rb-02", "Robot-02", "Robot", "Online", 88),
        new DeviceInfo("cam-04", "Cam-04", "Camera", "Offline", 61),
        new DeviceInfo("imu-01", "IMU-01", "IMU", "Online", 95),
        new DeviceInfo("plc-01", "PLC-01", "PLC", "Online", 90)
    };

    private static readonly List<AlertRecord> Alerts = new();
    private static readonly List<LogRecord> Logs = new();
    private static readonly Dictionary<string, double> MetricState = new();
    private static readonly List<double> TrendPoints = new();

    internal static IReadOnlyList<DeviceInfo> GetDevices()
    {
        lock (Sync)
        {
            return Devices.Select(device => new DeviceInfo(device.Id, device.Name, device.Type, device.Status, device.Health)
            {
                LastSeen = device.LastSeen
            }).ToList();
        }
    }

    public static IReadOnlyList<DeviceView> GetDeviceViews()
    {
        lock (Sync)
        {
            return Devices.Select(device => new DeviceView(
                device.Id,
                device.Name,
                device.Type,
                device.Status,
                device.Health,
                device.LastSeen.ToLocalTime().ToString("HH:mm"))).ToList();
        }
    }

    public static void ConnectAll()
    {
        lock (Sync)
        {
            var now = DateTimeOffset.UtcNow;
            foreach (var device in Devices)
            {
                device.Status = "Online";
                device.LastSeen = now;
            }
        }
    }

    public static void DisconnectAll()
    {
        lock (Sync)
        {
            var now = DateTimeOffset.UtcNow;
            foreach (var device in Devices)
            {
                device.Status = "Offline";
                device.LastSeen = now;
            }
        }
    }

    public static void DisconnectDevice(string deviceId)
    {
        lock (Sync)
        {
            var device = FindDevice(deviceId);
            if (device == null)
            {
                return;
            }
            device.Status = "Offline";
            device.LastSeen = DateTimeOffset.UtcNow;
        }
    }

    public static void SimulateFault(string deviceId)
    {
        lock (Sync)
        {
            var device = FindDevice(deviceId) ?? Devices[Random.Shared.Next(Devices.Count)];
            device.Status = "Offline";
            device.Health = Math.Max(10, device.Health - Random.Shared.Next(8, 20));
            device.LastSeen = DateTimeOffset.UtcNow;

            AddAlert(device, "Critical", "Vibration above threshold", "Vibration > 0.4g", "Stop & inspect");
            AddLog(device, "Error", "Motor stall");
        }
    }

    public static MetricsSnapshot GetMetricsSnapshot()
    {
        lock (Sync)
        {
            var temperature = NextMetricValue("Temperature", 30.0, 60.0, 0.4);
            var vibration = NextMetricValue("Vibration", 0.1, 1.2, 0.05);
            var current = NextMetricValue("Current", 5.0, 20.0, 0.6);
            var torque = NextMetricValue("Torque", 10.0, 30.0, 0.9);
            var errorRate = NextMetricValue("ErrorRate", 0.0, 5.0, 0.2);

            UpdateTrend(vibration);
            UpdateDeviceHealth();

            return new MetricsSnapshot(temperature, vibration, current, torque, errorRate, TrendPoints.ToList());
        }
    }

    public static IReadOnlyList<AlertView> GetAlerts(int limit)
    {
        lock (Sync)
        {
            MaybeAppendAlert();
            return Alerts
                .OrderByDescending(alert => alert.TsUnixMs)
                .Take(Math.Max(1, limit))
                .Select(ToAlertView)
                .ToList();
        }
    }

    public static AlertView? AcknowledgeAlert(string id)
    {
        lock (Sync)
        {
            var alert = Alerts.FirstOrDefault(item => item.Id == id);
            if (alert == null)
            {
                return null;
            }
            alert.Acknowledged = true;
            return ToAlertView(alert);
        }
    }

    public static AlertView? MuteAlert(string id)
    {
        lock (Sync)
        {
            var alert = Alerts.FirstOrDefault(item => item.Id == id);
            if (alert == null)
            {
                return null;
            }
            alert.Muted = true;
            return ToAlertView(alert);
        }
    }

    public static IReadOnlyList<LogView> GetLogs(int limit)
    {
        lock (Sync)
        {
            MaybeAppendLog();
            return Logs
                .OrderByDescending(log => log.TsUnixMs)
                .Take(Math.Max(1, limit))
                .Select(ToLogView)
                .ToList();
        }
    }

    public static IReadOnlyList<ReportView> BuildReports(string? timeRange, string? reportType, string? device)
    {
        var normalizedType = string.IsNullOrWhiteSpace(reportType) ? "Daily" : reportType;
        var normalizedRange = timeRange is "1h" or "24h" or "7d" ? timeRange : "24h";
        var today = DateTimeOffset.UtcNow;
        var timeLabel = normalizedType == "Monthly"
            ? today.ToString("yyyy-MM")
            : normalizedRange == "7d"
                ? $"{today.AddDays(-6):yyyy-MM-dd} ~ {today:yyyy-MM-dd}"
                : today.ToString("yyyy-MM-dd");

        if (!string.IsNullOrWhiteSpace(device))
        {
            timeLabel = $"{timeLabel} · {device}";
        }

        var reports = new List<ReportView>();
        if (normalizedType == "Monthly")
        {
            reports.Add(new ReportView("Monthly Health Summary", timeLabel, "Monthly health scores and uptime summary."));
            reports.Add(new ReportView("Daily Alerts", timeLabel, "Daily alert counts and critical incidents."));
        }
        else if (normalizedType == "Weekly")
        {
            reports.Add(new ReportView("Weekly Health Summary", timeLabel, "Weekly health scores and uptime summary."));
            reports.Add(new ReportView("Daily Alerts", timeLabel, "Daily alert counts and critical incidents."));
        }
        else if (normalizedRange == "1h")
        {
            reports.Add(new ReportView("Daily Alerts", timeLabel, "Daily alert counts and critical incidents."));
            reports.Add(new ReportView("Hourly Snapshot", timeLabel, "Hourly anomalies and key metrics."));
        }
        else
        {
            reports.Add(new ReportView("Daily Alerts", timeLabel, "Daily alert counts and critical incidents."));
            reports.Add(new ReportView("Daily Health Summary", timeLabel, "Daily health scores and uptime summary."));
        }

        return reports;
    }

    public static string ExportAlertsCsv(string? level, string? device, string? timeRange, string? status)
    {
        lock (Sync)
        {
            MaybeAppendAlert();
            var sb = new StringBuilder();
            sb.AppendLine("Level,Message,Device,Time,Evidence,Action,Acknowledged,Muted");
            foreach (var alert in Alerts.OrderByDescending(item => item.TsUnixMs))
            {
                if (!MatchesTimeRange(alert.TsUnixMs, timeRange))
                {
                    continue;
                }
                if (!string.IsNullOrWhiteSpace(level) && !alert.Level.Equals(level, StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }
                if (!string.IsNullOrWhiteSpace(device) && !alert.DeviceName.Equals(device, StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }
                if (!MatchesStatus(alert, status))
                {
                    continue;
                }

                var view = ToAlertView(alert);
                sb.Append(CsvEscape(view.Level)).Append(',')
                    .Append(CsvEscape(view.Message)).Append(',')
                    .Append(CsvEscape(view.DeviceName)).Append(',')
                    .Append(CsvEscape(view.Time)).Append(',')
                    .Append(CsvEscape(view.Evidence)).Append(',')
                    .Append(CsvEscape(view.Action)).Append(',')
                    .Append(view.Acknowledged ? "true" : "false").Append(',')
                    .Append(view.Muted ? "true" : "false")
                    .AppendLine();
            }
            return sb.ToString();
        }
    }

    public static string ExportLogsCsv(string? level, string? timeRange, string? search)
    {
        lock (Sync)
        {
            MaybeAppendLog();
            var sb = new StringBuilder();
            sb.AppendLine("Level,Message,Device,Time");
            foreach (var log in Logs.OrderByDescending(item => item.TsUnixMs))
            {
                if (!MatchesTimeRange(log.TsUnixMs, timeRange))
                {
                    continue;
                }
                if (!string.IsNullOrWhiteSpace(level) && !log.Level.Equals(level, StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }
                if (!string.IsNullOrWhiteSpace(search) && !MatchesSearch(log, search))
                {
                    continue;
                }

                var view = ToLogView(log);
                sb.Append(CsvEscape(view.Level)).Append(',')
                    .Append(CsvEscape(view.Message)).Append(',')
                    .Append(CsvEscape(view.DeviceName)).Append(',')
                    .Append(CsvEscape(view.Time))
                    .AppendLine();
            }
            return sb.ToString();
        }
    }

    public static string ExportReportsCsv(string? timeRange, string? reportType, string? device)
    {
        var reports = BuildReports(timeRange, reportType, device);
        var sb = new StringBuilder();
        sb.AppendLine("Title,Time,Summary,ReportType,TimeRange,Device");
        foreach (var report in reports)
        {
            sb.Append(CsvEscape(report.Title)).Append(',')
                .Append(CsvEscape(report.Time)).Append(',')
                .Append(CsvEscape(report.Summary)).Append(',')
                .Append(CsvEscape(reportType ?? "Daily")).Append(',')
                .Append(CsvEscape(timeRange ?? "24h")).Append(',')
                .Append(CsvEscape(string.IsNullOrWhiteSpace(device) ? "All" : device))
                .AppendLine();
        }
        return sb.ToString();
    }

    public static string ExportReportPdf(string? title, string? time, string? summary, string? reportType, string? timeRange, string? device)
    {
        var sb = new StringBuilder();
        sb.AppendLine("RoboHealth Report (mock PDF)");
        sb.AppendLine($"Title: {title ?? "Untitled"}");
        sb.AppendLine($"Time: {time ?? "-"}");
        sb.AppendLine($"Summary: {summary ?? "-"}");
        sb.AppendLine($"ReportType: {reportType ?? "Daily"}");
        sb.AppendLine($"TimeRange: {timeRange ?? "24h"}");
        sb.AppendLine($"Device: {(string.IsNullOrWhiteSpace(device) ? "All" : device)}");
        return sb.ToString();
    }

    public static DiagnosticsViewResponse BuildDiagnostics(string? device, string? timeRange)
    {
        var normalizedRange = timeRange is "1h" or "24h" or "7d" ? timeRange : "24h";
        var checklist = new List<string> { "Stop robot", "Inspect joint", "Apply lubricant", "Re-run calibration" };

        var templates = new[]
        {
            new { Symptom = "Vibration spike", Cause = "Loose bearing", Evidence = "Vibration > 0.4g", Action = "Stop and inspect" },
            new { Symptom = "Torque drift", Cause = "Calibration offset", Evidence = "Torque > 20N*m", Action = "Re-calibrate" },
            new { Symptom = "Current surge", Cause = "Cable wear", Evidence = "Current > 15A", Action = "Check wiring" },
            new { Symptom = "Temperature rise", Cause = "Cooling reduced", Evidence = "Temp > 55C", Action = "Inspect fan" }
        };

        var rng = new Random(Hash(device ?? string.Empty, normalizedRange));
        var first = rng.Next(0, 2);
        var second = rng.Next(2, 4);
        var confidenceBoost = normalizedRange == "1h" ? 0.05 : normalizedRange == "7d" ? -0.05 : 0.0;

        var cards = new List<DiagnosticCardView>();
        foreach (var index in new[] { first, second })
        {
            var entry = templates[index];
            var confidence = Math.Clamp(0.55 + rng.NextDouble() * 0.4 + confidenceBoost, 0.4, 0.95);
            cards.Add(new DiagnosticCardView(entry.Symptom, entry.Cause, entry.Evidence, entry.Action, confidence));
        }

        var evidence = new List<double>();
        var baseValue = 0.25 + rng.Next(0, 5) * 0.05;
        var variance = normalizedRange == "1h" ? 0.7 : normalizedRange == "7d" ? 0.45 : 0.55;
        for (var i = 0; i < 16; i++)
        {
            var jitter = (rng.NextDouble() - 0.5) * variance;
            var value = Math.Clamp(baseValue + jitter, 0.05, 0.95);
            evidence.Add(value);
        }

        return new DiagnosticsViewResponse(cards, evidence, checklist);
    }

    public static ReplayViewResponse BuildReplay(string? session, string? device, string? timeRange)
    {
        var normalizedRange = timeRange is "1h" or "24h" or "7d" ? timeRange : "24h";
        var seed = Hash(session ?? string.Empty, device ?? string.Empty, normalizedRange);
        var rng = new Random(seed);

        var durationSeconds = normalizedRange switch
        {
            "1h" => 300,
            "7d" => 1800,
            _ => 900
        };

        var events = normalizedRange switch
        {
            "1h" => new List<ReplayEventView>
            {
                new("00:10", "Replay started"),
                new("01:22", "Warning spike"),
                new("03:45", "Recovered")
            },
            "7d" => new List<ReplayEventView>
            {
                new("01:30", "Daily load"),
                new("08:10", "Warning spike"),
                new("12:20", "Critical stop"),
                new("18:05", "Recovered")
            },
            _ => new List<ReplayEventView>
            {
                new("00:30", "Replay started"),
                new("04:10", "Warning spike"),
                new("08:45", "Recovered")
            }
        };

        var offset = rng.Next(0, 45);
        var adjustedEvents = new List<ReplayEventView>();
        foreach (var evt in events)
        {
            var parts = evt.Time.Split(':');
            if (parts.Length == 2 && int.TryParse(parts[0], out var minutes) && int.TryParse(parts[1], out var seconds))
            {
                var total = (minutes * 60 + seconds + offset) % Math.Max(1, durationSeconds);
                var adjMin = total / 60;
                var adjSec = total % 60;
                adjustedEvents.Add(new ReplayEventView($"{adjMin:00}:{adjSec:00}", evt.Text));
            }
            else
            {
                adjustedEvents.Add(evt);
            }
        }

        var maxPoints = normalizedRange switch
        {
            "1h" => 20,
            "7d" => 40,
            _ => 30
        };
        var trendPoints = new List<double>(maxPoints);
        var baseValue = 0.15 + rng.NextDouble() * 0.2;
        var step = maxPoints > 1 ? 0.6 / (maxPoints - 1) : 0.0;
        for (var i = 0; i < maxPoints; i++)
        {
            trendPoints.Add(Math.Clamp(baseValue + step * i, 0.05, 0.95));
        }

        return new ReplayViewResponse(durationSeconds, adjustedEvents, trendPoints);
    }

    public static MetricsSample CreateMetricsSample(string deviceId, string metric)
    {
        lock (Sync)
        {
            var device = ResolveDevice(deviceId);
            var value = NextMetricValue($"{device.Id}:{metric}", metric);
            UpdateTrend(metric == "Vibration" ? value : (NextMetricValue("Vibration", 0.1, 1.2, 0.05)));
            return new MetricsSample
            {
                DeviceId = device.Id,
                Metric = metric,
                Value = value,
                TsUnixMs = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds()
            };
        }
    }

    public static AlertEvent CreateAlertEvent(string deviceId)
    {
        lock (Sync)
        {
            var device = ResolveDevice(deviceId);
            var alert = AddAlert(device, null, null, null, null);
            return new AlertEvent
            {
                Id = alert.Id,
                Level = alert.Level,
                Message = alert.Message,
                DeviceId = alert.DeviceId,
                DeviceName = alert.DeviceName,
                Evidence = alert.Evidence,
                Action = alert.Action,
                TsUnixMs = alert.TsUnixMs
            };
        }
    }

    public static LogEvent CreateLogEvent(string deviceId)
    {
        lock (Sync)
        {
            var device = ResolveDevice(deviceId);
            var log = AddLog(device, null, null);
            return new LogEvent
            {
                Id = log.Id,
                Level = log.Level,
                Message = log.Message,
                DeviceId = log.DeviceId,
                DeviceName = log.DeviceName,
                TsUnixMs = log.TsUnixMs
            };
        }
    }

    private static DeviceInfo? FindDevice(string deviceId)
    {
        return Devices.FirstOrDefault(device => device.Id == deviceId);
    }

    private static DeviceInfo ResolveDevice(string deviceId)
    {
        if (!string.IsNullOrWhiteSpace(deviceId))
        {
            var found = FindDevice(deviceId);
            if (found != null)
            {
                return found;
            }
        }

        return Devices[Random.Shared.Next(Devices.Count)];
    }

    private static double NextMetricValue(string metric, double min, double max, double step)
    {
        if (!MetricState.TryGetValue(metric, out var current))
        {
            current = (min + max) / 2.0;
        }
        var delta = (Random.Shared.NextDouble() * 2.0 - 1.0) * step;
        var next = Math.Clamp(current + delta, min, max);
        MetricState[metric] = next;
        return next;
    }

    private static double NextMetricValue(string key, string metric)
    {
        return metric switch
        {
            "Temperature" => NextMetricValue(key, 30.0, 60.0, 0.4),
            "Vibration" => NextMetricValue(key, 0.1, 1.2, 0.05),
            "Current" => NextMetricValue(key, 5.0, 20.0, 0.6),
            "Torque" => NextMetricValue(key, 10.0, 30.0, 0.9),
            "ErrorRate" => NextMetricValue(key, 0.0, 5.0, 0.2),
            _ => NextMetricValue(key, 0.0, 1.0, 0.1)
        };
    }

    private static void UpdateTrend(double vibration)
    {
        var normalized = Math.Clamp(vibration / 1.2, 0.0, 1.0);
        if (TrendPoints.Count >= 20)
        {
            TrendPoints.RemoveAt(0);
        }
        TrendPoints.Add(normalized);
    }

    private static void UpdateDeviceHealth()
    {
        if (Devices.Count == 0)
        {
            return;
        }

        var device = Devices[Random.Shared.Next(Devices.Count)];
        var delta = Random.Shared.Next(-3, 4);
        device.Health = Math.Clamp(device.Health + delta, 50, 100);
        device.LastSeen = DateTimeOffset.UtcNow;

        if (Random.Shared.Next(100) < 5)
        {
            device.Status = device.Status == "Online" ? "Offline" : "Online";
        }
    }

    private static void MaybeAppendAlert()
    {
        if (Alerts.Count == 0 || Random.Shared.Next(100) < 20)
        {
            var device = Devices[Random.Shared.Next(Devices.Count)];
            AddAlert(device, null, null, null, null);
        }
    }

    private static void MaybeAppendLog()
    {
        if (Logs.Count == 0 || Random.Shared.Next(100) < 35)
        {
            var device = Devices[Random.Shared.Next(Devices.Count)];
            AddLog(device, null, null);
        }
    }

    private static AlertRecord AddAlert(DeviceInfo device, string? level, string? message, string? evidence, string? action)
    {
        var templates = new[]
        {
            new { Level = "Critical", Message = "Joint torque spike", Evidence = "Torque > 20N*m", Action = "Stop & inspect" },
            new { Level = "Warn", Message = "Vibration above threshold", Evidence = "Vibration > 0.4g", Action = "Check bearing" },
            new { Level = "Info", Message = "Camera reconnect", Evidence = "Link restored", Action = "None" }
        };
        var choice = templates[Random.Shared.Next(templates.Length)];

        var alert = new AlertRecord
        {
            Id = Guid.NewGuid().ToString("N"),
            Level = level ?? choice.Level,
            Message = message ?? choice.Message,
            DeviceId = device.Id,
            DeviceName = device.Name,
            Evidence = evidence ?? choice.Evidence,
            Action = action ?? choice.Action,
            TsUnixMs = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds(),
            Acknowledged = false,
            Muted = false
        };

        Alerts.Add(alert);
        if (Alerts.Count > 20)
        {
            Alerts.RemoveAt(0);
        }
        return alert;
    }

    private static LogRecord AddLog(DeviceInfo device, string? level, string? message)
    {
        var templates = new[]
        {
            new { Level = "Error", Message = "Motor stall" },
            new { Level = "Warn", Message = "Vibration high" },
            new { Level = "Info", Message = "Camera reconnect" }
        };
        var choice = templates[Random.Shared.Next(templates.Length)];

        var log = new LogRecord
        {
            Id = Guid.NewGuid().ToString("N"),
            Level = level ?? choice.Level,
            Message = message ?? choice.Message,
            DeviceId = device.Id,
            DeviceName = device.Name,
            TsUnixMs = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds()
        };

        Logs.Add(log);
        if (Logs.Count > 30)
        {
            Logs.RemoveAt(0);
        }
        return log;
    }

    private static AlertView ToAlertView(AlertRecord alert)
    {
        var time = DateTimeOffset.FromUnixTimeMilliseconds(alert.TsUnixMs)
            .ToLocalTime()
            .ToString("HH:mm");
        return new AlertView(
            alert.Id,
            alert.Level,
            alert.Message,
            alert.DeviceId,
            alert.DeviceName,
            time,
            alert.Evidence,
            alert.Action,
            alert.Acknowledged,
            alert.Muted);
    }

    private static LogView ToLogView(LogRecord log)
    {
        var time = DateTimeOffset.FromUnixTimeMilliseconds(log.TsUnixMs)
            .ToLocalTime()
            .ToString("HH:mm");
        return new LogView(
            log.Id,
            log.Level,
            log.Message,
            log.DeviceId,
            log.DeviceName,
            time);
    }

    private static int Hash(params string[] parts)
    {
        unchecked
        {
            var hash = 17;
            foreach (var part in parts)
            {
                hash = hash * 31 + part.GetHashCode(StringComparison.Ordinal);
            }
            return hash;
        }
    }

    private static bool MatchesTimeRange(long tsUnixMs, string? timeRange)
    {
        if (string.IsNullOrWhiteSpace(timeRange))
        {
            return true;
        }

        var now = DateTimeOffset.UtcNow;
        var range = timeRange switch
        {
            "1h" => TimeSpan.FromHours(1),
            "24h" => TimeSpan.FromHours(24),
            "7d" => TimeSpan.FromDays(7),
            _ => TimeSpan.Zero
        };
        if (range == TimeSpan.Zero)
        {
            return true;
        }
        var threshold = now.Add(-range).ToUnixTimeMilliseconds();
        return tsUnixMs >= threshold;
    }

    private static bool MatchesStatus(AlertRecord alert, string? status)
    {
        if (string.IsNullOrWhiteSpace(status))
        {
            return true;
        }

        return status switch
        {
            "active" => !alert.Acknowledged && !alert.Muted,
            "acknowledged" => alert.Acknowledged,
            "muted" => alert.Muted,
            _ => true
        };
    }

    private static bool MatchesSearch(LogRecord log, string search)
    {
        if (string.IsNullOrWhiteSpace(search))
        {
            return true;
        }
        return (log.Message?.Contains(search, StringComparison.OrdinalIgnoreCase) ?? false)
               || (log.DeviceName?.Contains(search, StringComparison.OrdinalIgnoreCase) ?? false)
               || (log.Level?.Contains(search, StringComparison.OrdinalIgnoreCase) ?? false);
    }

    private static string CsvEscape(string? value)
    {
        var safe = value ?? string.Empty;
        if (safe.Contains('"'))
        {
            safe = safe.Replace("\"", "\"\"");
        }
        if (safe.Contains(',') || safe.Contains('\n') || safe.Contains('\r'))
        {
            safe = $"\"{safe}\"";
        }
        return safe;
    }
}
