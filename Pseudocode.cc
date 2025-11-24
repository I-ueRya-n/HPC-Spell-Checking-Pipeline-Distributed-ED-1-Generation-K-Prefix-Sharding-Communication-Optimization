

## Note on release

Due to course academic-integrity policy, the full implementation is not open-sourced.
This repo provides **pseudocode, system design, and experimental results** only.

---

## Pseudocode

### Algorithm 0 — Preprocess, Partition, and Build Index/Bloom

```text
Input:
  D : dictionary tokens (one per line)
  W : word list tokens (one per line)
  p : number of MPI ranks
  KMAX, KWRITE : K-prefix parameters
  bpw : Bloom bits-per-word (optional)

Output:
  prefix_index_local on each rank
  bloom_global on each rank (optional)
  bucket_to_rank mapping

0. Normalize all tokens in D and W:
     - lowercase
     - keep only [a-z0-9]
     - keep line order stable

1. Adaptive K-prefix partitioning:
     - Use first K letters/digits of each dictionary token as bucket id.
     - Increase K and split heavy buckets until
         bytes(bucket) ≤ (2/p) * bytes(D_global)
       or K reaches KMAX.
     - Produce bucket_to_rank so each bucket has a unique owner rank.

2. Redistribute dictionary by bucket_to_rank (Alltoallv).

3. On each rank r:
     prefix_index_local ← build exact-match index over D_local.

4. If Bloom enabled:
     bloom_local ← empty Bloom(m = |D_global| * bpw)
     for each word d in D_local:
         bloom_local.insert(d)
     bloom_global ← Allreduce_OR(bloom_local)
   else:
     bloom_global ← null
```

(對應你的 prefix 分桶、2/p cap、local prefix index、global Bloom OR-reduce。)

---

### Algorithm 1 — Phase A: Existence Filter (route words, keep misspelled)

```text
Input:
  W, bucket_to_rank, prefix_index_local
Output:
  M : misspelled set

1. For each word w in W:
     dst ← owner(bucket_id(w, KWRITE))
     send w to dst

2. Alltoallv exchange routed words.

3. On each rank r, for each received w:
     flag ← prefix_index_local.contains(w)  // exact lookup
     return flag to source of w

4. Alltoallv return flags.

5. Each source rank keeps only words with flag = false:
     M ← { w in W | not present in D }
6. Return M
```

(對應 Phase A 的兩次 Alltoallv + exact lookup。)

---

### Algorithm 2 — Phase B: ED1 Generate → Bloom Filter → Verify

```text
Input:
  M, bloom_global(optional), prefix_index_local, bucket_to_rank
Output:
  Pairs_B : set of (miss, candidate_hit)

1. For each misspelled word x in M (parallel on rank):
     C ← generate_ED1(x)  // substitute/delete/insert, dedup & sort

     For each candidate c in C:
         if Bloom enabled AND bloom_global.maybe(c) = false:
              drop c   // negative pruned early
         else:
              dst ← owner(bucket_id(c, KWRITE))
              send (x, c) to dst

2. Alltoallv exchange candidate requests.

3. On each owner rank r, for each received (x, c):
     if prefix_index_local.contains(c) = true:
          keep (x, c) as a hit

4. Alltoallv return hits to the source ranks.

5. Each source rank collects returned hits:
     Pairs_B ← {(x, c) that are verified hits}
6. Return Pairs_B
```

(對應 ED1 生成、Bloom 先過濾、再用 Alltoallv 去 owner 驗證、回傳 hits。)

---

### Algorithm 3 — Phase C: Deterministic Global Output

```text
Input:
  Pairs_B, bucket_to_rank
Output:
  Final output lines (global total order)

1. Local aggregation on each rank:
     For each (x, c) in Pairs_B:
         append c into Cand[x]
     For each x:
         Cand[x] ← sort_unique(Cand[x])

2. Determine output owner for each misspelled word x:
     out_dst ← owner(bucket_id(x, KWRITE))
     send record (x, Cand[x]) to out_dst

3. Alltoallv exchange aggregated records.

4. On each output-owner rank:
     merge candidates for same x
     Cand[x] ← sort_unique(Cand[x])

5. Deterministic ordering:
     - sort groups by:
         (a) |Cand[x]| ascending
         (b) x ASCII lexicographic ascending
     - within each group, candidates are ASCII sorted already

6. Write ordered lines:
     "x: c1 c2 c3 ..."
```

(對應 Phase C 的 group/dedup/sort + global ordering key。)

---

### Helper — ED=1 Candidate Generation

```text
generate_ED1(x):
  C ← empty list

  // substitution
  for i in [0 .. len(x)-1]:
      for ch in ALNUM:
          if ch != x[i]:
              C.add( x with x[i] replaced by ch )

  // deletion
  for i in [0 .. len(x)-1]:
      C.add( x with x[i] removed )

  // insertion
  for i in [0 .. len(x)]:
      for ch in ALNUM:
          C.add( x with ch inserted at position i )

  C ← sort_unique(C)  // remove duplicates
  return C
```

