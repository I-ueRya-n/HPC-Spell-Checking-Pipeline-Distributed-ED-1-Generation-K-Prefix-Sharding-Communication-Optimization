# HPC-Spell-Checking-Pipeline-Distributed-ED-1-Generation-K-Prefix-Sharding-Communication-Optimization
High-performance distributed text correction with α–β optimized communication, adaptive prefix partitioning, and deterministic global ordering.

## 🔍 Abstract

This project implements a **large-scale spell-checking engine** using  
**MPI + OpenMP hybrid parallelism**, **K-prefix partitioning**,  
**Bloom-filter-assisted distributed filtering**, and a **deterministic  
global sorting pipeline**.  

Given a dictionary and a word list, the system identifies misspelled  
words and generates all **edit-distance-1 (ED=1)** candidates  
(substitution, deletion, insertion).  
The computation is fully distributed, communication-efficient,  
and designed for HPC clusters.

The design focuses on:

- α–β **latency/bandwidth-optimized communication**  
- **Adaptive K-prefix load balancing (2/p byte cap)**  
- Global Bloom Filter OR-reduce  
- Zero-copy Alltoallv routing  
- Research-grade determinism & reproducibility  
- Strong & hybrid scaling across MPI ranks

---

## 🚀 Features & Contributions

### **🔧 Engineering**
- MPI/OpenMP hybrid parallelism (`R × P` processor mapping)  
- Zero-copy Alltoallv pack/unpack  
- Cache-friendly contiguous dictionary storage  
- LCP-based string comparison for faster sorting  
- Runtime tunables (`KWRITE`, `KMAX`, `BLOOM_BITS_PER_WORD`)

### **🔬 Research / Algorithmic**
- **Adaptive K-prefix shard search** with bucket-byte histogram  
- **Bloom-filtered ED-1 candidate funnel**  
  Reduces Phase B communication by 90–99%  
- **Deterministic global ordering**  
  Output identical across OMP/serial/MPI modes  
- **α–β Model Analysis** for message count & bytes  
- **Brent’s Work–Span Model** for scalability reasoning

---

## 🧩 System Pipeline

```


         ┌─────────────────────────────┐
         │  K-Prefix Partition (Adaptive) │
         └───────────────┬─────────────┘
                         │
                 Build Prefix Index
                         │
                Global Bloom Filter
              (All-Reduce bitwise OR)
                         │
 ┌───────────────────────┼───────────────────────┐
 │                       │                       │


Phase A                Phase B                 Phase C
Existence Filter   ED-1 Generate/Verify   Global Total Ordering
(2× Alltoallv)     (2× Alltoallv)        (1× Alltoallv)
│                       │                       │
└───────────────────────┴───────────────────────┘
Output



---

## 🛠️ Build Instructions

### **Prerequisites**
- g++ with OpenMP
- mpic++ (OpenMPI / MPICH)
- Linux HPC environment (recommended)

### **Build Baseline (serial + OMP)**
```bash
g++ -O3 -std=c++17 -fopenmp DataCollection.cc -o spell
````

### **Build MPI Version**

```bash
mpic++ -O3 -std=c++17 -fopenmp DataCollection.cc -o spell
```

### **Optional Flags**

| Flag                   | Meaning                            |
| ---------------------- | ---------------------------------- |
| `-DDISABLE_BLOOM`      | Disable Bloom filter entirely      |
| `-DVERBOSE_FROM_ENV=1` | Read verbosity from `$VERBOSE` env |
| `-DNDEBUG`             | Disable assertions                 |

---

## 📊 Experimental Methodology

Following HPC research conventions, we report:

### **1. Strong Scaling (R = 1…64)**

* Total wall time
* Speedup and parallel efficiency
* Phase decomposition (A/B/C)
* Communication breakdown by α–β model

### **2. Hybrid Mapping**

* Fix 64 CPUs
* Sweep (R, P) pairs to test MPI vs OpenMP tradeoffs

### **3. Bloom Filter Sweep**

* Measure bytes saved in Phase B
* Report FPR as function of bits/word

### **4. Load Balance**

* ghist_max_ratio vs K-prefix
* Ideal: → 1.00

### **5. Determinism Checks**

* Output MD5 must match baseline.

---

## 📄 Output Format

Each line:

```
misspelled_word: candidate1 candidate2 candidate3 ...
```

Sorted by:

1. #candidate ascending
2. misspelled word (ASCII lexicographic)

---

## 💡 Key Results (Example Summary)

* 8K→600K words strong scaling
* R=64 achieves **≈ 19.6× speedup**
* Bloom filter removes **~98% ED-1 negatives**
* K-prefix reduces skew; max bucket ratio → **1.23**
* Alltoallv bytes drop by **~3×** after Bloom
* Deterministic output across all modes (verified by md5sum)

---

## 📜 License
Apache License 2.0

---

## ⭐ Citation

```
Lin, Yu-Wei. "Parallel Sorting and Deduplication for Large-Scale Text:
An HPC Study." University of Melbourne, 2025.
```

