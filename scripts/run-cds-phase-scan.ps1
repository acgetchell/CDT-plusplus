param(
  [string]$BuildDir = $(if ($env:BUILD_DIR) { $env:BUILD_DIR } else { "build" }),
  [string]$ResultsDir = $(if ($env:RESULTS_DIR) { $env:RESULTS_DIR } else { "results" }),
  [string[]]$TargetVolumes = $(if ($env:TARGET_VOLUMES) { $env:TARGET_VOLUMES -split " " } else { @("8000", "16000", "32000", "64000") }),
  [string[]]$CouplingPoints = $(if ($env:COUPLING_POINTS) { $env:COUPLING_POINTS -split " " } else { @("1.0,0.2,0.1") }),
  [int]$Chains = $(if ($env:CHAINS) { [int]$env:CHAINS } else { 4 }),
  [int]$Timeslices = $(if ($env:TIMESLICES) { [int]$env:TIMESLICES } else { 32 }),
  [int]$Thermalization = $(if ($env:THERMALIZATION) { [int]$env:THERMALIZATION } else { 1000 }),
  [int]$MeasurementInterval = $(if ($env:MEASUREMENT_INTERVAL) { [int]$env:MEASUREMENT_INTERVAL } else { 10 }),
  [int]$Passes = $(if ($env:PASSES) { [int]$env:PASSES } else { 2000 }),
  [double]$VolumeEpsilon = $(if ($env:VOLUME_EPSILON) { [double]$env:VOLUME_EPSILON } else { 0.001 }),
  [int]$BaseSeed = $(if ($env:BASE_SEED) { [int]$env:BASE_SEED } else { 1000 })
)

$ErrorActionPreference = "Stop"

cmake --build $BuildDir
if ($LASTEXITCODE -ne 0) {
  throw "cmake build failed with exit code $LASTEXITCODE."
}
ctest --test-dir $BuildDir --output-on-failure
if ($LASTEXITCODE -ne 0) {
  throw "ctest failed with exit code $LASTEXITCODE."
}

$candidateExecutables = @(
  (Join-Path $BuildDir "src/cdt.exe"),
  (Join-Path $BuildDir "src/Debug/cdt.exe"),
  (Join-Path $BuildDir "src/Release/cdt.exe"),
  (Join-Path $BuildDir "src/RelWithDebInfo/cdt.exe"),
  (Join-Path $BuildDir "src/cdt")
)
$cdt = $candidateExecutables | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $cdt) {
  throw "Could not find cdt executable under $BuildDir."
}

foreach ($point in $CouplingPoints) {
  $parts = $point.Split(",")
  if ($parts.Count -lt 3) {
    throw "Coupling point '$point' must contain kappa0,kappa4,Delta."
  }
  $kappa0 = $parts[0]
  $kappa4 = $parts[1]
  $delta = $parts[2]
  foreach ($target in $TargetVolumes) {
    foreach ($chain in 1..$Chains) {
      $seedText = "$BaseSeed|$point|$target|$chain"
      $sha256 = [System.Security.Cryptography.SHA256]::Create()
      try {
        $seedBytes = $sha256.ComputeHash(
          [System.Text.Encoding]::UTF8.GetBytes($seedText))
      }
      finally {
        $sha256.Dispose()
      }
      $seed = [BitConverter]::ToUInt32($seedBytes, 0)
      $runId = "cds-k0_$kappa0-k4_$kappa4-d_$delta-n4_$target-chain_$chain"
      & $cdt `
        --spherical `
        --dimensions 4 `
        --simplices $target `
        --timeslices $Timeslices `
        --kappa0 $kappa0 `
        --kappa4 $kappa4 `
        --Delta $delta `
        --target-n4 $target `
        --volume-epsilon $VolumeEpsilon `
        --passes $Passes `
        --thermalization $Thermalization `
        --measurement-interval $MeasurementInterval `
        --seed $seed `
        --chain-id "chain-$chain" `
        --run-id $runId `
        --output-dir $ResultsDir
      if ($LASTEXITCODE -ne 0) {
        throw "cdt failed for $runId with exit code $LASTEXITCODE."
      }
    }
  }
}
