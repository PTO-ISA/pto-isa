# PTO: Document Structure

This manual is organized as a layered architecture reference: establish the programming and machine models first, then syntax and types, then memory rules, then the instruction set. The chapter roles stay fixed so readers can locate model rules before opcode detail; the content remains specific to PTO's tile-first Ascend model.

## Chapter Map

This manual is organized into 13 numbered chapters. The navigation sidebar at the left mirrors this structure.

| Chapter | Sections | What It Covers |
| --- | --- | --- |
| **1. Introduction** | What Is PTO VISA, Document Structure, Goals, Version 1.0, Scope | What PTO is, why it exists, version baseline, and specification boundaries |
| **2. Programming Model** | Tiles & Valid Regions, GlobalTensor & Data Movement, Auto vs Manual | The primary programming objects authors reason about |
| **3. Machine Model** | Execution Agents & Target Profiles, Ordering & Synchronization | Execution hierarchy, pipelines, target profiles, and sync vocabulary |
| **4. Syntax and Operands** | Assembly Spelling, Operands & Attributes, Common Conventions | Textual spelling, operand shapes, attributes, and naming conventions |
| **5. State and Types** | Type System, Layout Reference, Data Format Reference, Location Intent | Types, layouts, data formats, tile roles, and legality rules |
| **6. Memory Model** | Consistency Baseline, Producer-Consumer Ordering | Visibility and ordering rules across pipelines and cores |
| **7. Instruction Set Overview** | Instruction Surfaces, Instruction Families | High-level maps of all instruction sets and contracts |
| **8. Instruction Set Contracts** | Tile Families, Vector Families, Scalar & Control Families, Other Families | Legal types, layouts, shapes per instruction family |
| **9. Tile ISA Reference** | Sync & Config, Elementwise Tile-Tile, Tile-Scalar, Reduce & Expand, Memory & Data Movement, Matrix & Matrix-Vector, Layout & Rearrangement, Irregular & Complex | All `pto.t*` operations (~70 pages) |
| **10. Vector ISA Reference** | Vector Load/Store, Predicate & Materialization, Unary, Binary, Vec-Scalar, Conversion, Reduction, Compare & Select, Data Rearrangement, SFU & DSA | All `pto.v*` operations (~60 pages) |
| **11. Scalar and Control Reference** | Pipeline Sync, DMA Copy, Predicate Load/Store, Predicate Generation & Algebra, Shared Arithmetic, Shared SCF | All `pto.*` scalar/control operations (~50 pages) |
| **12. Other and Communication Reference** | Collective Communication, Non-ISA Supporting Ops | TBROADCAST, TGET, TPUT, TREDUCE, TWAIT, TALIAS, TAXPY, TCONCAT, etc. |
| **13. Reference Notes** | Format of Descriptions, Diagnostics, Glossary, Portability, Source of Truth | Supporting reference material |

Chapters 1–6 are the "read before opcode detail" layer. Chapters 7–8 give the high-level instruction set map. Chapters 9–12 are the per-op reference. Chapter 13 is supporting material.

## PTO-Specific Reading Notes

PTO is built around **tiles**, **valid regions**, and **explicit synchronization**. Read model chapters first, then syntax and state, then memory, then per-op pages. This keeps architecture guarantees separate from backend profile narrowing and avoids treating examples as standalone contracts.

## See Also

- [PTO ISA hub](../README.md)
- [Format of instruction descriptions](../reference/format-of-instruction-descriptions.md)
