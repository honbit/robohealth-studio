using System.Text.Json;
using Microsoft.AspNetCore.Http.Json;
using Microsoft.AspNetCore.Server.Kestrel.Core;
using RoboHealth.Simulator.Services;

var builder = WebApplication.CreateBuilder(args);

builder.WebHost.ConfigureKestrel(options =>
{
    options.ListenLocalhost(5000, listenOptions =>
    {
        listenOptions.Protocols = HttpProtocols.Http1;
    });
    options.ListenLocalhost(50051, listenOptions =>
    {
        listenOptions.Protocols = HttpProtocols.Http2;
    });
});

builder.Services.AddGrpc();
builder.Services.Configure<JsonOptions>(options =>
{
    options.SerializerOptions.PropertyNamingPolicy = JsonNamingPolicy.CamelCase;
});

var app = builder.Build();

app.MapGrpcService<TelemetryService>();
app.MapGet("/api/health", () => Results.Ok(new { status = "ok" }));
app.MapGet("/api/devices", () => Results.Ok(new { items = TelemetryState.GetDeviceViews() }));
app.MapPost("/api/devices/connectAll", () =>
{
    TelemetryState.ConnectAll();
    return Results.NoContent();
});
app.MapPost("/api/devices/disconnectAll", () =>
{
    TelemetryState.DisconnectAll();
    return Results.NoContent();
});
app.MapPost("/api/devices/{id}/disconnect", (string id) =>
{
    TelemetryState.DisconnectDevice(id);
    return Results.NoContent();
});
app.MapPost("/api/devices/{id}/simulateFault", (string id) =>
{
    TelemetryState.SimulateFault(id);
    return Results.NoContent();
});
app.MapGet("/api/metrics", () => Results.Ok(TelemetryState.GetMetricsSnapshot()));
app.MapGet("/api/alerts", (int? limit) =>
{
    return Results.Ok(new { items = TelemetryState.GetAlerts(limit ?? 20) });
});
app.MapPost("/api/alerts/{id}/ack", (string id) =>
{
    var alert = TelemetryState.AcknowledgeAlert(id);
    return alert == null ? Results.NotFound() : Results.Ok(alert);
});
app.MapPost("/api/alerts/{id}/mute", (string id) =>
{
    var alert = TelemetryState.MuteAlert(id);
    return alert == null ? Results.NotFound() : Results.Ok(alert);
});
app.MapGet("/api/logs", (int? limit) =>
{
    return Results.Ok(new { items = TelemetryState.GetLogs(limit ?? 30) });
});
app.MapGet("/api/exports/alerts", (string? level, string? device, string? timeRange, string? status) =>
{
    var stamp = DateTimeOffset.UtcNow.ToString("yyyyMMdd_HHmmss");
    return Results.Ok(new
    {
        fileName = $"alerts_{stamp}.csv",
        content = TelemetryState.ExportAlertsCsv(level, device, timeRange, status)
    });
});
app.MapGet("/api/exports/logs", (string? level, string? timeRange, string? search) =>
{
    var stamp = DateTimeOffset.UtcNow.ToString("yyyyMMdd_HHmmss");
    return Results.Ok(new
    {
        fileName = $"logs_{stamp}.csv",
        content = TelemetryState.ExportLogsCsv(level, timeRange, search)
    });
});
app.MapGet("/api/reports", (string? timeRange, string? reportType, string? device) =>
{
    return Results.Ok(new { items = TelemetryState.BuildReports(timeRange, reportType, device) });
});
app.MapGet("/api/exports/reports", (string? timeRange, string? reportType, string? device) =>
{
    var stamp = DateTimeOffset.UtcNow.ToString("yyyyMMdd_HHmmss");
    return Results.Ok(new
    {
        fileName = $"reports_{stamp}.csv",
        content = TelemetryState.ExportReportsCsv(timeRange, reportType, device)
    });
});
app.MapGet("/api/exports/reportPdf", (string? title, string? time, string? summary, string? reportType, string? timeRange, string? device) =>
{
    var stamp = DateTimeOffset.UtcNow.ToString("yyyyMMdd_HHmmss");
    return Results.Ok(new
    {
        fileName = $"report_{stamp}.pdf",
        content = TelemetryState.ExportReportPdf(title, time, summary, reportType, timeRange, device)
    });
});
app.MapGet("/api/diagnostics", (string? device, string? timeRange) =>
{
    return Results.Ok(TelemetryState.BuildDiagnostics(device, timeRange));
});
app.MapGet("/api/replay", (string? session, string? device, string? timeRange) =>
{
    return Results.Ok(TelemetryState.BuildReplay(session, device, timeRange));
});
app.MapGet("/", () => "RoboHealth gRPC simulator is running on h2c://localhost:50051");

app.Run();
