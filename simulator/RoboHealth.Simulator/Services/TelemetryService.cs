using Grpc.Core;
using RoboHealth.Simulator;

namespace RoboHealth.Simulator.Services;

public sealed class TelemetryService : RoboHealth.Simulator.TelemetryService.TelemetryServiceBase
{
    public override Task<DeviceList> ListDevices(Empty request, ServerCallContext context)
    {
        var list = new DeviceList();
        foreach (var device in TelemetryState.GetDevices())
        {
            list.Items.Add(new Device
            {
                Id = device.Id,
                Name = device.Name,
                Type = device.Type,
                Status = device.Status,
                Health = device.Health
            });
        }
        return Task.FromResult(list);
    }

    public override async Task SubscribeMetrics(MetricsRequest request, IServerStreamWriter<MetricsSample> responseStream, ServerCallContext context)
    {
        var metric = string.IsNullOrWhiteSpace(request.Metric) ? "Temperature" : request.Metric;
        var interval = request.IntervalMs > 0 ? request.IntervalMs : 1000;

        while (!context.CancellationToken.IsCancellationRequested)
        {
            var sample = TelemetryState.CreateMetricsSample(request.DeviceId, metric);
            await responseStream.WriteAsync(sample);

            await Task.Delay(interval, context.CancellationToken);
        }
    }

    public override async Task SubscribeAlerts(AlertsRequest request, IServerStreamWriter<AlertEvent> responseStream, ServerCallContext context)
    {
        while (!context.CancellationToken.IsCancellationRequested)
        {
            var alert = TelemetryState.CreateAlertEvent(request.DeviceId);
            await responseStream.WriteAsync(alert);

            var delay = Random.Shared.Next(2500, 4500);
            await Task.Delay(delay, context.CancellationToken);
        }
    }

    public override async Task SubscribeLogs(LogsRequest request, IServerStreamWriter<LogEvent> responseStream, ServerCallContext context)
    {
        while (!context.CancellationToken.IsCancellationRequested)
        {
            var log = TelemetryState.CreateLogEvent(request.DeviceId);
            await responseStream.WriteAsync(log);

            var delay = Random.Shared.Next(800, 1500);
            await Task.Delay(delay, context.CancellationToken);
        }
    }
}
