# Manual / Resource Binding

This document describes manual resource binding and configuration operations.

**Total Operations:** 4

---

## Operations

### TASSIGN

For detailed instruction documentation, see [isa/TASSIGN](../isa/TASSIGN.md)

**AS Level 1 (SSA):**
```text
pto.tassign %tile, %addr : !pto.tile<...>, dtype
```

**AS Level 2 (DPS):**
```text
pto.tassign ins(%tile, %addr : !pto.tile_buf<...>, dtype)
```

---

### TSETFMATRIX

For detailed instruction documentation, see [isa/TSETFMATRIX](../isa/TSETFMATRIX.md)


**AS Level 1 (SSA):**
```text
pto.tsetfmatrix %cfg : !pto.fmatrix_config -> ()
```

**AS Level 2 (DPS):**
```text
pto.tsetfmatrix ins(%cfg : !pto.fmatrix_config) outs()
```

---

### TSET_IMG2COL_RPT

For detailed instruction documentation, see [isa/TSET_IMG2COL_RPT](../isa/TSET_IMG2COL_RPT.md)

**AS Level 1 (SSA):**
```text
pto.tset_img2col_rpt %cfg : !pto.fmatrix_config -> ()
```

**AS Level 2 (DPS):**
```text
pto.tset_img2col_rpt ins(%cfg : !pto.fmatrix_config) outs()
```

---

### TSET_IMG2COL_PADDING

For detailed instruction documentation, see [isa/TSET_IMG2COL_PADDING](../isa/TSET_IMG2COL_PADDING.md)

**AS Level 1 (SSA):**
```text
pto.tset_img2col_padding %cfg : !pto.fmatrix_config -> ()
```

**AS Level 2 (DPS):**
```text
pto.tset_img2col_padding ins(%cfg : !pto.fmatrix_config) outs()
```

---

