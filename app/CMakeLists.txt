add_executable(robohealth-studio
  main.cpp
  ${CMAKE_SOURCE_DIR}/presentation/DashboardViewModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/DeviceFilterModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/DevicesViewModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/AlertFilterModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/AlertsViewModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/DiagnosticsViewModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/LogFilterModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/LogsViewModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/ReplayViewModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/ReportsViewModel.cpp
  ${CMAKE_SOURCE_DIR}/presentation/RestExportHelper.cpp
  ${CMAKE_SOURCE_DIR}/presentation/SettingsViewModel.cpp
  ${CMAKE_SOURCE_DIR}/integration/TelemetryServiceBase.cpp
  ${CMAKE_SOURCE_DIR}/integration/MockTelemetryService.cpp
  ${CMAKE_SOURCE_DIR}/integration/RestTelemetryService.cpp
)

set(QML_BASE ${CMAKE_SOURCE_DIR}/ui)
set(QML_FILES
  ${QML_BASE}/Main.qml
  ${QML_BASE}/components/Sidebar.qml
  ${QML_BASE}/components/TopBar.qml
  ${QML_BASE}/components/Card.qml
  ${QML_BASE}/components/PrimaryButton.qml
  ${QML_BASE}/components/GhostButton.qml
  ${QML_BASE}/components/Badge.qml
  ${QML_BASE}/components/SectionHeader.qml
  ${QML_BASE}/components/KpiCard.qml
  ${QML_BASE}/components/TrendChart.qml
  ${QML_BASE}/components/AlertList.qml
  ${QML_BASE}/components/Timeline.qml
  ${QML_BASE}/components/DeviceStatusList.qml
  ${QML_BASE}/components/SparkLine.qml
  ${QML_BASE}/components/SkeletonTable.qml
  ${QML_BASE}/components/DataRow.qml
  ${QML_BASE}/components/HeaderLabel.qml
  ${QML_BASE}/components/LanguageToggle.qml
  ${QML_BASE}/components/Snackbar.qml
  ${QML_BASE}/components/TableHeader.qml
  ${QML_BASE}/components/EmptyState.qml
  ${QML_BASE}/components/FieldRow.qml
  ${QML_BASE}/pages/DashboardPage.qml
  ${QML_BASE}/pages/DevicesPage.qml
  ${QML_BASE}/pages/AlertsPage.qml
  ${QML_BASE}/pages/LogsPage.qml
  ${QML_BASE}/pages/ReplayPage.qml
  ${QML_BASE}/pages/DiagnosticsPage.qml
  ${QML_BASE}/pages/ReportsPage.qml
  ${QML_BASE}/pages/SettingsPage.qml
  ${QML_BASE}/state/I18n.qml
  ${QML_BASE}/state/qmldir
  ${QML_BASE}/theme/Theme.qml
  ${QML_BASE}/theme/qmldir
)

qt_add_resources(robohealth-studio qml
  PREFIX "/ui"
  BASE ${QML_BASE}
  FILES ${QML_FILES}
)

target_link_libraries(robohealth-studio
  PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Charts Qt6::Network
)

target_include_directories(robohealth-studio
  PRIVATE ${CMAKE_SOURCE_DIR}/presentation
          ${CMAKE_SOURCE_DIR}/integration
)
