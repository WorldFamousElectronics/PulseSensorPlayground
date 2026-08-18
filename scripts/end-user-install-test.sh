#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tmp_dir="$(mktemp -d)"
cleanup() {
  rm -rf "$tmp_dir"
}
trap cleanup EXIT

zip_path="$tmp_dir/PulseSensorPlayground.zip"
sketchbook="$tmp_dir/sketchbook"
config_file="$tmp_dir/arduino-cli.yaml"

mkdir -p "$sketchbook"

cat > "$config_file" <<EOF
directories:
  user: $sketchbook
library:
  enable_unsafe_install: true
board_manager:
  additional_urls:
    - https://espressif.github.io/arduino-esp32/package_esp32_index.json
EOF

git -C "$repo_root" archive \
  --format zip \
  --prefix PulseSensorPlayground/ \
  --output "$zip_path" \
  HEAD

arduino-cli --config-file "$config_file" lib install --zip-path "$zip_path"

installed_library="$(find "$sketchbook/libraries" -maxdepth 1 -type d -name 'PulseSensor*' | head -n 1)"
if [[ -z "$installed_library" ]]; then
  echo "Could not find installed PulseSensor library in $sketchbook/libraries" >&2
  exit 1
fi

default_matrix="XIAO ESP32-S3 BPM|esp32:esp32|esp32:esp32:XIAO_ESP32S3|examples/PulseSensor_XIAO_ESP32S3_BPM|https://espressif.github.io/arduino-esp32/package_esp32_index.json"
matrix="${PULSE_SENSOR_END_USER_MATRIX:-$default_matrix}"

while IFS='|' read -r name core fqbn sketch additional_urls; do
  [[ -z "${name:-}" ]] && continue

  if [[ "${PULSE_SENSOR_INSTALL_CORES:-0}" == "1" ]]; then
    if [[ -n "${additional_urls:-}" ]]; then
      arduino-cli --config-file "$config_file" core update-index --additional-urls "$additional_urls"
      arduino-cli --config-file "$config_file" core install "$core" --additional-urls "$additional_urls"
    else
      arduino-cli --config-file "$config_file" core update-index
      arduino-cli --config-file "$config_file" core install "$core"
    fi
  fi

  echo "Compiling installed-library example: $name"
  arduino-cli --config-file "$config_file" compile \
    --fqbn "$fqbn" \
    "$installed_library/$sketch" \
    --warnings default
done <<< "$matrix"

echo "End-user ZIP install test passed for installed library: $installed_library"
