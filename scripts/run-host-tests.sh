#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
test_output_dir="$(mktemp -d)"
trap 'rm -rf "$test_output_dir"' EXIT
cxx="${CXX:-g++}"
json_include=".pio/libdeps/esp32-s3-devkitm-1/ArduinoJson/src"
compile_and_run() {
    local name="$1"
    shift
    "$cxx" -std=c++11 -Wall -Wextra -Werror "$@" -o "$test_output_dir/$name"
    "$test_output_dir/$name"
}
compile_and_run routine-power -Itest/runtime-host -Isrc test/routine_power_test.cpp src/routines/battery-routine.cpp src/routines/routine-controller.cpp
compile_and_run local-queues -Itest/storage-host -Isrc test/local_queues_test.cpp src/storage/pending-measurement-store.cpp
compile_and_run sps30-power -Itest/sensor-host -Isrc test/sps30_power_test.cpp src/sensors/sps30.cpp
compile_and_run location-schedule test/location_schedule_test.cpp
if [[ ! -d "$json_include" ]]; then
    echo "Instale as dependências primeiro: pio pkg install" >&2
    exit 1
fi
compile_and_run location-manager -Itest/host -Isrc -I"$json_include" test/location_manager_test.cpp src/location/location-manager.cpp
compile_and_run measurement-configuration -Itest/host -Isrc -I"$json_include" test/measurement_configuration_test.cpp src/config/settings-manager.cpp
compile_and_run ble-transport -Wno-unused-parameter -Itest/ble-host -Isrc -I"$json_include" test/ble_transport_test.cpp src/bluetooth/ble-manager.cpp
