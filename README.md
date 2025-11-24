# HPC-NLP-Lexical-Matching-Pipeline-Distributed-ED-1-Generation-K-Prefix-Sharding-Communication-Optimization
High-performance distributed text correction with α–β optimized communication, adaptive prefix partitioning, and deterministic global ordering.


---


## 🔍 Abstract

This project implements a **large-scale lexical spell-checking / ED-1 matching engine** with a focus on HPC scaling.
It uses **MPI + OpenMP hybrid parallelism**, **adaptive K-prefix partitioning**, **Bloom-filter assisted distributed pruning**, and a **deterministic global ordering stage**.

Given a dictionary and a word list, the system finds misspelled tokens and generates all **edit-distance-1 (ED=1)** candidates (substitution, deletion, insertion).
The computation is fully distributed and communication-efficient, and it is designed for an HPC cluster.

The design focuses on:

* α–β **latency / bandwidth modeling** for communication decisions
* **Adaptive K-prefix load balancing** with a 2/p cap for heavy buckets
* **Global Bloom filter** built by OR-reduce for early pruning
* Alltoallv-based routing with low copy overhead
* Research-grade **determinism and reproducibility**
* Strong scaling and hybrid scaling across MPI ranks and threads

---

## 🚀 Features & Contributions

### 🔧 Engineering

* MPI + OpenMP hybrid mapping (**R × P**) on Spartan HPC
* Communication-aware routing built around Alltoallv patterns
* Cache-friendly, contiguous dictionary layout
* Optimized string comparison and sorting in the gather stage
* Runtime tunables for K-prefix and Bloom settings (kept private here)

### 🔬 Research / System insights

* **Adaptive K-prefix shard search** to control lexical skew
* **Bloom-filtered ED-1 candidate funnel** that removes most negatives before routing
* **Deterministic global ordering** so outputs match across serial, OMP, MPI, and hybrid runs
* **α–β analysis** showing message latency dominates bytes
* **Brent-style work/span reasoning** to explain the scaling knee
* **Systematic evaluation over 672 configurations** (strong scaling, hybrid sweeps, and multi-node placement)

---

## 🧩 System Pipeline (high level)

```
         ┌───────────────────────────────┐
         │   Adaptive K-Prefix Partition  │
         └───────────────┬───────────────┘
                         │
                  Build Prefix Index
                         │
               Global Bloom Filter
            (All-Reduce bitwise OR)
                         │
 ┌───────────────────────┼───────────────────────┐
 │                       │                       │

Phase A                Phase B                 Phase C
Existence Filter   ED-1 Generate / Verify   Global Total Ordering
(Alltoallv)         (Prune → Route → Check)    (Alltoallv Gather)

 │                       │                       │
 └───────────────────────┴───────────────────────┘
                        Output
```

---

## 📊 Experimental Methodology (summary)

I followed standard HPC evaluation practice:

1. **Strong scaling (R = 1…64)**

   * Total wall time and phase breakdown
   * Speedup and parallel efficiency
   * α–β communication fitting for hot phases

2. **Hybrid mapping sweep**

   * Fixed 64 CPUs
   * Tested multiple (R, P) pairs to study MPI vs OpenMP tradeoffs

3. **Bloom filter sweep**

   * Measured pruning impact and FPR vs bits/word
   * Chose a knee point setting for best cost/benefit

4. **Load balance checks**

   * Tracked heavy-bucket ratios under different K values

5. **Determinism checks**

   * Outputs must match the serial baseline (MD5-verified)

---

## 💡 Key Results (fixed workload ≈659k tokens)

**Runtime**

* Single-node OpenMP baseline: **8.2 s**
* Pure MPI (64×1): **0.428 s**
* Hybrid MPI+OpenMP (16×4): **0.280 s**

This is about **29× faster than single node**, and about **35% faster than pure MPI** even with fewer ranks.
At the sweet spot, throughput reaches **~2.35M tokens/s** (vs ~1.65M tokens/s for pure MPI and ~82k tokens/s for single node).

---

## 🔍 Bottleneck Insights (measured)

**Message count dominates bytes.**
From α–β modeling in Phase B:
**α = 0.031 ms/message**, **β = 0.00087 ms/byte**.
So latency is driven mainly by **how many messages**, not how big they are. Bloom pruning targets this directly.

**📊 Scaling reality on real hardware.**
Efficiency stayed near 100% up to 16 ranks, then dropped:

* **R = 32 → ~49% efficiency**
* **R = 64 → ~31% efficiency**

The strong-scaling knee appears around **P* ≈ 54**, which matches Brent-style work/span reasoning.

**Multi-node scaling collapses.**
64 ranks across 4 nodes took **1232 ms vs 428 ms** on one node (**+187.9% slowdown**).
Cross-node latency dominates this fine-grain routing workload once the lexicon fits in memory.

**💡 The sweet spot.**
After testing several hybrid settings, **16 MPI ranks × 4 threads** is best:

* **0.28 s total time**
* **~35% faster than 64×1 pure MPI**
* **~99.998% fewer messages** than high-rank designs

Fewer ranks mean far fewer messages, so latency stops being the limit.

---

## 📁 What’s in this public repo

* `figures/01_Architecture_Diagram.png`
* `figures/02_Strong_Scaling_Curve.png`
* `figures/03_Hybrid_Comparison.png`
* `report/` (PDF summary and plots)
* This README

Implementation and scripts are private due to course policy.

---

## 📜 License (documentation only)
Apache License 2.0
This public summary is shared for portfolio use.
If you reuse figures or text, please cite the project.
(The implementation is not open-sourced.)

---

## ⭐ Citation

Lin, Yu-Wei. *Distributed ED-1 Lexical Matching on Spartan: A Hybrid MPI+OpenMP Bottleneck Study.* University of Melbourne, 2025.




