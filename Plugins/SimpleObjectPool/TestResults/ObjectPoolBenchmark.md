# SimpleObjectPool Benchmark Results - 2026-07-08

## Objective

This benchmark compares the runtime cost of spawning/destroying projectile actors against reusing projectile actors through `SimpleObjectPool`.

The test uses the Aura FireBolt actor as the benchmark projectile and measures both acquire cost and release cost across identical workloads.

## Test Configuration

| Setting | Value |
| --- | --- |
| Projectile actor | FireBolt |
| Test modes | `SpawnDestroy`, `ObjectPool` |
| Waves per mode | 5 |
| Actors per wave | 500 |
| Total actor operations per mode | 2500 acquire operations + 2500 release operations |
| Active duration per wave | `2.000s` |
| Wave interval | `1.000s` |

## Test Artifacts

| Artifact | Path |
| --- | --- |
| Benchmark output | `C:/Unreal5/Aura/Saved/Logs/Aura.log` |
| Spawn/destroy trace | `C:/Unreal5/Aura/Saved/Profiling/20260708_143748.utrace` |
| Object pool trace | `C:/Unreal5/Aura/Saved/Profiling/20260708_143817.utrace` |
| Spawn/destroy Insights export | `C:/Unreal5/Aura/Plugins/SimpleObjectPool/TestResults/spawn_events.csv` |
| Object pool Insights export | `C:/Unreal5/Aura/Plugins/SimpleObjectPool/TestResults/pool_events.csv` |

Unreal trace timestamps use a UTC-style timestamp. Vancouver local time on the test date was PDT, UTC-7.

| Mode | Completion Time in Trace Output | Vancouver Local Time |
| --- | --- | --- |
| `SpawnDestroy` | `2026.07.08-21.38.03` | `2026-07-08 2:38:03 PM` |
| `ObjectPool` | `2026.07.08-21.38.32` | `2026-07-08 2:38:32 PM` |

## Performance Summary

| Metric | Description | SpawnDestroy | ObjectPool | Result |
| --- | --- | ---: | ---: | --- |
| Cold initialization | One-time cost to pre-spawn the initial pool before runtime reuse begins. `SpawnDestroy` has no equivalent setup step. | 0.000ms | 106.852ms | One-time setup cost for the pooled path |
| Acquire total | Total time to get 2500 actors into play. For SpawnDestroy this is spawning; for ObjectPool this is borrowing/reusing. | 872.717ms | 62.426ms | Pool is 92.8% lower, about 14.0x faster |
| Acquire average per actor | Average time to acquire one actor. Useful for estimating cost per projectile. | 0.349087ms | 0.024971ms | Pool is 92.8% lower |
| Max acquire wave | Slowest single wave of acquiring 500 actors. This approximates the largest frame-time spike during projectile creation. | 196.782ms | 14.458ms | Pool is 92.7% lower, about 13.6x faster |
| Release total | Total time to release 2500 actors. For SpawnDestroy this is destroying; for ObjectPool this is returning/resetting into the pool. | 138.005ms | 22.615ms | Pool is 83.6% lower, about 6.1x faster |
| Release average per actor | Average time to release one actor. Useful for estimating cleanup/reset cost per projectile. | 0.055202ms | 0.009046ms | Pool is 83.6% lower |
| Max release wave | Slowest single wave of releasing 500 actors. This approximates the largest frame-time spike during cleanup/reset. | 28.231ms | 5.073ms | Pool is 82.0% lower, about 5.6x faster |
| Runtime total | Acquire total + release total during active gameplay, excluding pool setup. This measures the runtime reuse benefit. | 1010.722ms | 85.041ms | Pool is 91.6% lower, about 11.9x faster |
| Total including cold setup | Runtime total plus one-time pool initialization. This shows whether pooling still wins after paying setup cost. | 1010.722ms | 191.893ms | Pool is 81.0% lower, about 5.3x faster |

## Wave-Level Results

| Mode | Wave | Acquire | Release |
| --- | ---: | ---: | ---: |
| SpawnDestroy | 1 | 111.676ms | 28.022ms |
| SpawnDestroy | 2 | 182.450ms | 26.984ms |
| SpawnDestroy | 3 | 190.877ms | 26.902ms |
| SpawnDestroy | 4 | 196.782ms | 27.866ms |
| SpawnDestroy | 5 | 190.932ms | 28.231ms |
| ObjectPool | 1 | 10.844ms | 3.958ms |
| ObjectPool | 2 | 12.458ms | 4.753ms |
| ObjectPool | 3 | 14.458ms | 4.116ms |
| ObjectPool | 4 | 12.266ms | 4.716ms |
| ObjectPool | 5 | 12.401ms | 5.073ms |

## Unreal Insights Analysis

Unreal Insights successfully analyzed both trace files and exported the benchmark CPU markers with `TimingInsights.ExportTimingEvents`, filtered to `SimpleObjectPoolBenchmark_*`.

Trace analysis metadata:

| Trace | Session Duration | Threads | Timers | CPU Scopes | Exported Benchmark Events |
| --- | ---: | ---: | ---: | ---: | ---: |
| `20260708_143748.utrace` | 3m 46.5s | 54 | 7065 | 17111464 | 13 |
| `20260708_143817.utrace` | 4m 19.3s | 50 | 7066 | 21175310 | 15 |

The trace files include a few adjacent benchmark scopes from manual testing. For this comparison, the `SpawnDestroy` analysis uses the five `SpawnActorWave` and five `DestroyActorWave` events. The `ObjectPool` analysis starts at `SimpleObjectPoolBenchmark_ColdInitializePool` and uses the five borrow/return waves after that point.

| Mode | Insights Marker | Description | Count | Total | Average Per Wave | Max Wave |
| --- | --- | --- | ---: | ---: | ---: | ---: |
| SpawnDestroy | `SimpleObjectPoolBenchmark_SpawnActorWave` | CPU scope around each 500-actor spawn wave. | 5 | 872.715ms | 174.543ms | 196.781ms |
| SpawnDestroy | `SimpleObjectPoolBenchmark_DestroyActorWave` | CPU scope around each 500-actor destroy wave. | 5 | 138.002ms | 27.600ms | 28.231ms |
| ObjectPool | `SimpleObjectPoolBenchmark_ColdInitializePool` | One-time pre-spawn cost for the pool. | 1 | 106.851ms | 106.851ms | 106.851ms |
| ObjectPool | `SimpleObjectPoolBenchmark_BorrowFromPoolWave` | CPU scope around each 500-actor pooled borrow/reuse wave. | 5 | 62.424ms | 12.485ms | 14.457ms |
| ObjectPool | `SimpleObjectPoolBenchmark_ReturnToPoolWave` | CPU scope around each 500-actor deactivate/reset/return wave. | 5 | 22.611ms | 4.522ms | 5.072ms |

The Unreal Insights export matches the benchmark output within a few thousandths of a millisecond, confirming that the measured results are consistent across both instrumentation paths.

## Conclusion

The object pool provides a substantial performance improvement for repeated projectile creation and cleanup.

Borrowing 2500 pooled FireBolt actors took `62.426ms`, compared with `872.717ms` for spawning the same number of actors. This is a `92.8%` reduction in acquire time, or roughly `14x` faster.

Returning 2500 actors to the pool took `22.615ms`, compared with `138.005ms` for destroying them. This is an `83.6%` reduction in release time, or roughly `6.1x` faster.

The runtime total, acquire plus release, decreased from `1010.722ms` to `85.041ms`, a `91.6%` reduction. Including the one-time pool initialization cost of `106.852ms`, the pooled path still measured `81.0%` lower overall for this workload.
