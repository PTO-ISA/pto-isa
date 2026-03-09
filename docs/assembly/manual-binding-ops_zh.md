# 手动/资源绑定

本文档描述手动资源绑定和配置操作。

**操作总数：** 4

---

## 操作

### TASSIGN

该指令的详细介绍请见[isa/TASSIGN](../isa/TASSIGN_zh.md)

**AS Level 1 (SSA)：**
```text
pto.tassign %tile, %addr : !pto.tile<...>, dtype
```

**AS Level 2 (DPS)：**
```text
pto.tassign ins(%tile, %addr : !pto.tile_buf<...>, dtype)
```

---

### TSETFMATRIX

该指令的详细介绍请见[isa/TSETFMATRIX](../isa/TSETFMATRIX_zh.md)


**AS Level 1 (SSA)：**
```text
pto.tsetfmatrix %cfg : !pto.fmatrix_config -> ()
```

**AS Level 2 (DPS)：**
```text
pto.tsetfmatrix ins(%cfg : !pto.fmatrix_config) outs()
```

---

### TSET_IMG2COL_RPT

该指令的详细介绍请见[isa/TSET_IMG2COL_RPT](../isa/TSET_IMG2COL_RPT_zh.md)

**AS Level 1 (SSA)：**
```text
pto.tset_img2col_rpt %cfg : !pto.fmatrix_config -> ()
```

**AS Level 2 (DPS)：**
```text
pto.tset_img2col_rpt ins(%cfg : !pto.fmatrix_config) outs()
```

---

### TSET_IMG2COL_PADDING

该指令的详细介绍请见[isa/TSET_IMG2COL_PADDING](../isa/TSET_IMG2COL_PADDING_zh.md)

**AS Level 1 (SSA)：**
```text
pto.tset_img2col_padding %cfg : !pto.fmatrix_config -> ()
```

**AS Level 2 (DPS)：**
```text
pto.tset_img2col_padding ins(%cfg : !pto.fmatrix_config) outs()
```

---

